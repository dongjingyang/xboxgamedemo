//
// InputRouter.cpp
// Input layer implementation
//

#include "pch.h"
#include "InputRouter.h"

#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
#include <gameinput.h>
#endif

#include <cmath>

InputRouter::InputRouter()
    : m_lastButtonState(0)
    , m_lastKeyState(0)
    , m_lastDirection(Direction::Right)
{
}

InputRouter::~InputRouter()
{
}

InputState InputRouter::Poll(
    void* gameInput,
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
    GameInput::v3::IGameInputDevice** outActiveGamepadDevice
#else
    void** outActiveGamepadDevice
#endif
)
{
    InputState state = {};
    state.startPressed = false;
    state.pausePressed = false;
    state.dir = std::nullopt;

#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
    if (outActiveGamepadDevice)
        *outActiveGamepadDevice = nullptr;

    if (!gameInput)
        return state;

    GameInput::v3::IGameInput* gameInputPtr = static_cast<GameInput::v3::IGameInput*>(gameInput);

    // Try gamepad first
    GameInput::v3::IGameInputReading* reading = nullptr;
    if (SUCCEEDED(gameInputPtr->GetCurrentReading(GameInput::v3::GameInputKindGamepad, nullptr, &reading)) && reading)
    {
        // Get and cache the gamepad device for rumble
        GameInput::v3::IGameInputDevice* device = nullptr;
        reading->GetDevice(&device);
        if (device && outActiveGamepadDevice)
        {
            *outActiveGamepadDevice = device;
            // Note: Caller should AddRef if they want to keep it
        }

        GameInput::v3::GameInputGamepadState gamepadState = {};
        if (SUCCEEDED(reading->GetGamepadState(&gamepadState)))
        {
            // Detect button presses
            uint64_t buttonChanges = gamepadState.buttons & ~m_lastButtonState;

            // A button = start
            if (buttonChanges & GameInput::v3::GameInputGamepadA)
            {
                state.startPressed = true;
            }

            // Menu button = pause
            if (buttonChanges & GameInput::v3::GameInputGamepadMenu)
            {
                state.pausePressed = true;
            }

            m_lastButtonState = gamepadState.buttons;

            // Handle left thumbstick for direction input
            const float threshold = 0.5f;
            float leftX = gamepadState.leftThumbstickX;
            float leftY = -gamepadState.leftThumbstickY; // Invert Y for screen coordinates

            Direction newDir = m_lastDirection;

            if (fabsf(leftX) > threshold || fabsf(leftY) > threshold)
            {
                if (fabsf(leftX) > fabsf(leftY))
                {
                    // Horizontal movement
                    if (leftX > threshold)
                        newDir = Direction::Right;
                    else if (leftX < -threshold)
                        newDir = Direction::Left;
                }
                else
                {
                    // Vertical movement
                    if (leftY > threshold)
                        newDir = Direction::Down;
                    else if (leftY < -threshold)
                        newDir = Direction::Up;
                }

                // Only report direction if it changed
                if (newDir != m_lastDirection)
                {
                    state.dir = newDir;
                    m_lastDirection = newDir;
                }
            }
        }

        reading->Release();
    }

    // Also check keyboard input as fallback
    reading = nullptr;
    if (SUCCEEDED(gameInputPtr->GetCurrentReading(GameInput::v3::GameInputKindKeyboard, nullptr, &reading)) && reading)
    {
        uint32_t keyCount = reading->GetKeyCount();
        if (keyCount > 0)
        {
            constexpr uint32_t maxKeys = 16;
            GameInput::v3::GameInputKeyState keyState[maxKeys];
            uint32_t actualCount = reading->GetKeyState(maxKeys, keyState);

            // GetKeyState returns currently pressed keys
            uint64_t currentKeyState = 0;
            for (uint32_t i = 0; i < actualCount; ++i)
            {
                currentKeyState |= (1ULL << (keyState[i].virtualKey % 64));
            }

            uint64_t keyChanges = currentKeyState & ~m_lastKeyState;

            // Space or Enter = start
            if (keyChanges & ((1ULL << (VK_SPACE % 64)) | (1ULL << (VK_RETURN % 64))))
            {
                state.startPressed = true;
            }

            // Escape = pause
            if (keyChanges & (1ULL << (VK_ESCAPE % 64)))
            {
                state.pausePressed = true;
            }

            // Arrow keys or WASD for direction input
            Direction newDir = m_lastDirection;

            if (keyChanges & (1ULL << (VK_UP % 64)) || keyChanges & (1ULL << ('W' % 64)))
            {
                newDir = Direction::Up;
            }
            else if (keyChanges & (1ULL << (VK_DOWN % 64)) || keyChanges & (1ULL << ('S' % 64)))
            {
                newDir = Direction::Down;
            }
            else if (keyChanges & (1ULL << (VK_LEFT % 64)) || keyChanges & (1ULL << ('A' % 64)))
            {
                newDir = Direction::Left;
            }
            else if (keyChanges & (1ULL << (VK_RIGHT % 64)) || keyChanges & (1ULL << ('D' % 64)))
            {
                newDir = Direction::Right;
            }

            // Only report direction if it changed
            if (newDir != m_lastDirection)
            {
                state.dir = newDir;
                m_lastDirection = newDir;
            }

            m_lastKeyState = currentKeyState;
        }

        reading->Release();
    }
#endif

    return state;
}
