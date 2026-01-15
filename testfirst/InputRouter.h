//
// InputRouter.h
// Input layer: GameInput + Keyboard -> simplified input events
//

#pragma once

#include <optional>
#include "SnakeGame.h"  // For Direction enum

// Forward declarations
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
namespace GameInput
{
    namespace v3
    {
        struct IGameInput;
        struct IGameInputDevice;
    }
}
#endif

// Simplified input state
struct InputState
{
    bool startPressed;  // A/Space/Enter pressed
    bool pausePressed;  // Menu/Esc pressed
    std::optional<Direction> dir;  // Direction from left thumbstick or keys (only on change)
};

// Input router that processes GameInput and keyboard, outputs simplified events
class InputRouter
{
public:
    InputRouter();
    ~InputRouter();

    // Poll input and return simplified state
    // Also returns the active gamepad device for rumble (if available)
    InputState Poll(
        void* gameInput,  // IGameInput* when GameInput is available
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
        GameInput::v3::IGameInputDevice** outActiveGamepadDevice = nullptr
#else
        void** outActiveGamepadDevice = nullptr
#endif
    );

private:
    uint64_t m_lastButtonState;  // Track button state to detect presses
    uint64_t m_lastKeyState;  // Track keyboard state to detect presses
    Direction m_lastDirection;  // Track last direction to detect changes
};
