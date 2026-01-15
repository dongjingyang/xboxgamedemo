//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include <future>
#include <ResourceUploadBatch.h>
#include <GraphicsMemory.h>
#include <CommonStates.h>
#include <DescriptorHeap.h>
#include <Audio.h>
#include <BufferHelpers.h>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

// Don't use "using namespace GameInput::v3" to avoid ambiguity with XGameStreaming::IGameInputReading
// Use full namespace qualification instead

Game::Game() noexcept(false)
    : m_state(GameState::Title)
    , m_playerPos(400.0f, 300.0f)  // Center of default 800x600 window (kept for compatibility)
    , m_score(0)
    , m_time(0.0f)
    , m_gameInput(nullptr)
    , m_lastButtonState(0)
    , m_direction(Direction::Right)
    , m_nextDirection(Direction::Right)
    , m_moveAccumulator(0.0f)
    , m_rumbleTimeLeft(0.0f)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    //   Add DX::DeviceResources::c_ReverseDepth to optimize depth buffer clears for 0 instead of 1.
    m_deviceResources->RegisterDeviceNotify(this);
    
    // Initialize GameInput
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
    GameInput::v3::IGameInput* gameInput = nullptr;
    if (SUCCEEDED(GameInput::v3::GameInputCreate(&gameInput)))
    {
        m_gameInput = gameInput;
    }
    else
    {
        m_gameInput = nullptr;
    }
#else
    m_gameInput = nullptr;
#endif
}

Game::~Game()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
    
    // Release GameInput
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
    if (m_gameInput)
    {
        static_cast<GameInput::v3::IGameInput*>(m_gameInput)->Release();
        m_gameInput = nullptr;
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());
    m_time += elapsedTime;

    // Update audio engine
    if (m_audioEngine)
    {
        m_audioEngine->Update();
    }
    
    // Update rumble timer
    UpdateRumble(elapsedTime);

    // Process GameInput (Gamepad and Keyboard)
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
    if (m_gameInput)
    {
        GameInput::v3::IGameInput* gameInput = static_cast<GameInput::v3::IGameInput*>(m_gameInput);
        
        // Try gamepad first
        GameInput::v3::IGameInputReading* reading = nullptr;
        if (SUCCEEDED(gameInput->GetCurrentReading(GameInput::v3::GameInputKindGamepad, nullptr, &reading)) && reading)
        {
            // Get and cache the gamepad device for rumble (update every frame to ensure we have the latest device)
            GameInput::v3::IGameInputDevice* device = nullptr;
            reading->GetDevice(&device);
            if (device)
            {
                // ComPtr assignment will automatically AddRef
                m_activeGamepadDevice = device;
                device->Release(); // Release the extra reference from GetDevice
                
#ifdef _DEBUG
                // Log device acquisition and rumble support (only once to avoid spam)
                static bool loggedDevice = false;
                if (!loggedDevice && m_activeGamepadDevice)
                {
                    const GameInput::v3::GameInputDeviceInfo* info = nullptr;
                    m_activeGamepadDevice->GetDeviceInfo(&info);
                    if (info)
                    {
                        char debugMsg[256];
                        sprintf_s(debugMsg, "Gamepad device acquired: %p, Rumble motors: 0x%X\n", 
                            m_activeGamepadDevice.Get(), info->supportedRumbleMotors);
                        AddLog(debugMsg);
                        
                        // Check if rumble is supported
                        if (info->supportedRumbleMotors & (GameInput::v3::GameInputRumbleLowFrequency | GameInput::v3::GameInputRumbleHighFrequency))
                        {
                            AddLog("Device supports rumble motors\n");
                        }
                        else
                        {
                            AddLog("WARNING: Device does not support rumble motors!\n");
                        }
                    }
                    loggedDevice = true;
                }
#endif
            }
            
            GameInput::v3::GameInputGamepadState gamepadState = {};
            if (SUCCEEDED(reading->GetGamepadState(&gamepadState)))
            {
                // Detect button presses (buttons that are pressed now but weren't before)
                uint64_t buttonChanges = gamepadState.buttons & ~m_lastButtonState;
                
                // Handle A button press (start game from title, resume from paused, or restart from win/gameover)
                if (buttonChanges & GameInput::v3::GameInputGamepadA)
                {
                    if (m_state == GameState::Title)
                    {
                        m_state = GameState::Playing;
                        ResetGame();
                    }
                    else if (m_state == GameState::Paused)
                    {
                        m_state = GameState::Playing;
                    }
                    else if (m_state == GameState::Win || m_state == GameState::GameOver)
                    {
                        // Restart game
                        m_state = GameState::Playing;
                        ResetGame();
                    }
                }
                
                // Handle Menu button (Start button) press - pause/unpause
                if (buttonChanges & GameInput::v3::GameInputGamepadMenu)
                {
                    if (m_state == GameState::Playing)
                    {
                        m_state = GameState::Paused;
                        StopRumble(); // Stop rumble when pausing
                    }
                    else if (m_state == GameState::Paused)
                    {
                        m_state = GameState::Playing;
                    }
                }
                
                // Update last button state
                m_lastButtonState = gamepadState.buttons;
                
                // Handle left thumbstick for direction input (only in Playing state)
                if (m_state == GameState::Playing)
                {
                    const float threshold = 0.5f; // Threshold for direction detection
                    
                    float leftX = gamepadState.leftThumbstickX;
                    float leftY = -gamepadState.leftThumbstickY; // Invert Y for screen coordinates
                    
                    // Determine primary direction (horizontal takes priority if |x| > |y|)
                    Direction newDir = m_nextDirection; // Keep current if no input
                    
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
                        
                        // Prevent 180-degree turn
                        if ((m_direction == Direction::Up && newDir != Direction::Down) ||
                            (m_direction == Direction::Down && newDir != Direction::Up) ||
                            (m_direction == Direction::Left && newDir != Direction::Right) ||
                            (m_direction == Direction::Right && newDir != Direction::Left))
                        {
                            m_nextDirection = newDir;
                        }
                    }
                }
            }
            
            reading->Release();
        }
        
        // Also check keyboard input as fallback
        reading = nullptr;
        if (SUCCEEDED(gameInput->GetCurrentReading(GameInput::v3::GameInputKindKeyboard, nullptr, &reading)) && reading)
        {
            static uint64_t lastKeyState = 0;
            uint32_t keyCount = reading->GetKeyCount();
            if (keyCount > 0)
            {
                constexpr uint32_t maxKeys = 16;
                GameInput::v3::GameInputKeyState keyState[maxKeys];
                uint32_t actualCount = reading->GetKeyState(maxKeys, keyState);
                
                // GetKeyState returns currently pressed keys - if a key is in the array, it's pressed
                uint64_t currentKeyState = 0;
                for (uint32_t i = 0; i < actualCount; ++i)
                {
                    // Key is in the array, so it's pressed (no need to check isActive - it doesn't exist)
                    currentKeyState |= (1ULL << (keyState[i].virtualKey % 64));
                }
                
                uint64_t keyChanges = currentKeyState & ~lastKeyState;
                
                // Space or Enter = A button (start/resume/restart)
                if (keyChanges & ((1ULL << (VK_SPACE % 64)) | (1ULL << (VK_RETURN % 64))))
                {
                    if (m_state == GameState::Title)
                    {
                        m_state = GameState::Playing;
                        ResetGame();
                    }
                    else if (m_state == GameState::Paused)
                    {
                        m_state = GameState::Playing;
                    }
                    else if (m_state == GameState::Win || m_state == GameState::GameOver)
                    {
                        // Restart game
                        m_state = GameState::Playing;
                        ResetGame();
                    }
                }
                
                // Escape = Menu button (pause)
                if (keyChanges & (1ULL << (VK_ESCAPE % 64)))
                {
                    if (m_state == GameState::Playing)
                    {
                        m_state = GameState::Paused;
                    }
                    else if (m_state == GameState::Paused)
                    {
                        m_state = GameState::Playing;
                    }
                }
                
                // Arrow keys or WASD for direction input (only in Playing state)
                if (m_state == GameState::Playing)
                {
                    Direction newDir = m_nextDirection; // Keep current if no input
                    
                    // Check arrow keys and WASD for direction
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
                    
                    // Prevent 180-degree turn
                    if ((m_direction == Direction::Up && newDir != Direction::Down) ||
                        (m_direction == Direction::Down && newDir != Direction::Up) ||
                        (m_direction == Direction::Left && newDir != Direction::Right) ||
                        (m_direction == Direction::Right && newDir != Direction::Left))
                    {
                        m_nextDirection = newDir;
                    }
                }
                
                // Handle Space/Enter for restart from Win/GameOver state
                if (keyChanges & ((1ULL << (VK_SPACE % 64)) | (1ULL << (VK_RETURN % 64))))
                {
                    if (m_state == GameState::Win || m_state == GameState::GameOver)
                    {
                        // Restart game
                        m_state = GameState::Playing;
                        ResetGame();
                    }
                }
                
                lastKeyState = currentKeyState;
            }
            
            reading->Release();
        }
    }
#endif

    // Snake movement (only in Playing state, not paused)
    if (m_state == GameState::Playing)
    {
        // Update direction from queued direction
        m_direction = m_nextDirection;
        
        // Accumulate time for discrete movement
        m_moveAccumulator += elapsedTime;
        
        // Move snake when accumulator reaches move interval
        while (m_moveAccumulator >= c_moveInterval)
        {
            MoveSnakeOneStep();
            m_moveAccumulator -= c_moveInterval;
        }
    }

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    // TODO: Add your rendering code here.

    // 2D Rendering with SpriteBatch
    // Set descriptor heap for textures
    if (m_srvDescriptorHeap && m_srvDescriptorHeap->Heap())
    {
        ID3D12DescriptorHeap* heaps[] = { m_srvDescriptorHeap->Heap() };
        commandList->SetDescriptorHeaps(1, heaps);
    }
    
    if (m_spriteBatch)
    {
        m_spriteBatch->Begin(commandList);
    
    // Draw based on game state
    switch (m_state)
    {
    case GameState::Title:
        // Draw title screen text
        if (m_spriteFont)
        {
            const wchar_t* titleText = L"Press A to Start";
            DirectX::XMVECTOR textSize = m_spriteFont->MeasureString(titleText);
            float textWidth = DirectX::XMVectorGetX(textSize);
            float textHeight = DirectX::XMVectorGetY(textSize);
            
            int width, height;
            GetDefaultSize(width, height);
            float x = (static_cast<float>(width) - textWidth) * 0.5f;
            float y = (static_cast<float>(height) - textHeight) * 0.5f;
            
            m_spriteFont->DrawString(m_spriteBatch.get(), titleText, DirectX::XMFLOAT2(x, y), DirectX::Colors::White);
        }
        else
        {
            // If no font, draw multiple squares to show something and indicate it's working
            if (m_placeholderTexture && m_placeholderTextureSRV.ptr != 0)
            {
                int width, height;
                GetDefaultSize(width, height);
                float centerX = static_cast<float>(width) * 0.5f;
                float centerY = static_cast<float>(height) * 0.5f;
                float squareSize = 80.0f;
                
                // Draw a larger square in the center (title indicator)
                m_spriteBatch->Draw(
                    m_placeholderTextureSRV,
                    DirectX::XMUINT2(1, 1),
                    DirectX::XMFLOAT2(centerX - squareSize * 0.5f, centerY - squareSize * 0.5f),
                    nullptr,
                    DirectX::Colors::White,
                    0.0f,
                    DirectX::XMFLOAT2(0.0f, 0.0f),
                    squareSize);
                
                // Draw smaller squares around it to indicate "Press Space/Enter to Start"
                float smallSize = 20.0f;
                float offset = squareSize * 0.5f + 30.0f;
                
                // Top square (up arrow indicator)
                m_spriteBatch->Draw(
                    m_placeholderTextureSRV,
                    DirectX::XMUINT2(1, 1),
                    DirectX::XMFLOAT2(centerX - smallSize * 0.5f, centerY - offset - smallSize * 0.5f),
                    nullptr,
                    DirectX::Colors::Yellow,
                    0.0f,
                    DirectX::XMFLOAT2(0.0f, 0.0f),
                    smallSize);
                
                // Bottom square (down arrow indicator)
                m_spriteBatch->Draw(
                    m_placeholderTextureSRV,
                    DirectX::XMUINT2(1, 1),
                    DirectX::XMFLOAT2(centerX - smallSize * 0.5f, centerY + offset - smallSize * 0.5f),
                    nullptr,
                    DirectX::Colors::Yellow,
                    0.0f,
                    DirectX::XMFLOAT2(0.0f, 0.0f),
                    smallSize);
            }
#ifdef _DEBUG
            else
            {
                AddLog("WARNING: Cannot draw placeholder - texture or SRV invalid\n");
            }
#endif
        }
        break;
        
    case GameState::Playing:
    case GameState::Paused:
    case GameState::GameOver:
    case GameState::Win:
        // Draw snake (only if not empty)
        if (m_placeholderTexture && m_placeholderTextureSRV.ptr != 0 && !m_snake.empty())
        {
            const float segmentSize = c_cellSize * 0.9f; // Slightly smaller than cell for visual gap
            
            // Draw snake body (all segments except head)
            for (size_t i = 1; i < m_snake.size(); ++i)
            {
                const DirectX::XMFLOAT2& segment = m_snake[i];
                m_spriteBatch->Draw(
                    m_placeholderTextureSRV,
                    DirectX::XMUINT2(1, 1),
                    DirectX::XMFLOAT2(segment.x - segmentSize * 0.5f, segment.y - segmentSize * 0.5f),
                    nullptr,
                    DirectX::Colors::LightGreen, // Body color
                    0.0f,
                    DirectX::XMFLOAT2(0.0f, 0.0f),
                    segmentSize);
            }
            
            // Draw snake head (first segment)
            const DirectX::XMFLOAT2& head = m_snake.front();
            m_spriteBatch->Draw(
                m_placeholderTextureSRV,
                DirectX::XMUINT2(1, 1),
                DirectX::XMFLOAT2(head.x - segmentSize * 0.5f, head.y - segmentSize * 0.5f),
                nullptr,
                DirectX::Colors::Cyan, // Head color
                0.0f,
                DirectX::XMFLOAT2(0.0f, 0.0f),
                segmentSize);
        }
        
        // Draw food
        if (m_placeholderTexture && m_placeholderTextureSRV.ptr != 0 && m_food.alive)
        {
            const float foodSize = c_cellSize * 0.8f;
            m_spriteBatch->Draw(
                m_placeholderTextureSRV,
                DirectX::XMUINT2(1, 1),
                DirectX::XMFLOAT2(m_food.pos.x - foodSize * 0.5f, m_food.pos.y - foodSize * 0.5f),
                nullptr,
                DirectX::Colors::Gold, // Food color
                0.0f,
                DirectX::XMFLOAT2(0.0f, 0.0f),
                foodSize);
        }
        
        // Draw HUD (FPS, Score, Length)
        if (m_spriteFont)
        {
            float yPos = 10.0f;
            const float lineHeight = 30.0f; // Approximate line height
            
            // FPS
            wchar_t fpsText[64];
            float fps = static_cast<float>(m_timer.GetFramesPerSecond());
            swprintf_s(fpsText, L"FPS: %.1f", fps);
            m_spriteFont->DrawString(m_spriteBatch.get(), fpsText, DirectX::XMFLOAT2(10.0f, yPos), DirectX::Colors::Yellow);
            yPos += lineHeight;
            
            // Score
            wchar_t scoreText[64];
            swprintf_s(scoreText, L"Score: %d", m_score);
            m_spriteFont->DrawString(m_spriteBatch.get(), scoreText, DirectX::XMFLOAT2(10.0f, yPos), DirectX::Colors::Yellow);
            yPos += lineHeight;
            
            // Length
            wchar_t lengthText[64];
            swprintf_s(lengthText, L"Length: %zu", m_snake.size());
            m_spriteFont->DrawString(m_spriteBatch.get(), lengthText, DirectX::XMFLOAT2(10.0f, yPos), DirectX::Colors::Yellow);
        }
        
        // Draw state-specific text
        if (m_spriteFont)
        {
            int width, height;
            GetDefaultSize(width, height);
            
            if (m_state == GameState::Paused)
            {
                const wchar_t* pausedText = L"Paused - Press Start";
                DirectX::XMVECTOR textSize = m_spriteFont->MeasureString(pausedText);
                float textWidth = DirectX::XMVectorGetX(textSize);
                float textHeight = DirectX::XMVectorGetY(textSize);
                float x = (static_cast<float>(width) - textWidth) * 0.5f;
                float y = (static_cast<float>(height) - textHeight) * 0.5f;
                m_spriteFont->DrawString(m_spriteBatch.get(), pausedText, DirectX::XMFLOAT2(x, y), DirectX::Colors::White);
            }
            else if (m_state == GameState::GameOver)
            {
                const wchar_t* gameOverText = L"Game Over - Press A to Restart";
                DirectX::XMVECTOR textSize = m_spriteFont->MeasureString(gameOverText);
                float textWidth = DirectX::XMVectorGetX(textSize);
                float textHeight = DirectX::XMVectorGetY(textSize);
                float x = (static_cast<float>(width) - textWidth) * 0.5f;
                float y = (static_cast<float>(height) - textHeight) * 0.5f;
                m_spriteFont->DrawString(m_spriteBatch.get(), gameOverText, DirectX::XMFLOAT2(x, y), DirectX::Colors::Red);
            }
            else if (m_state == GameState::Win)
            {
                const wchar_t* winText = L"You Win - Press A to Restart";
                DirectX::XMVECTOR textSize = m_spriteFont->MeasureString(winText);
                float textWidth = DirectX::XMVectorGetX(textSize);
                float textHeight = DirectX::XMVectorGetY(textSize);
                float x = (static_cast<float>(width) - textWidth) * 0.5f;
                float y = (static_cast<float>(height) - textHeight) * 0.5f;
                m_spriteFont->DrawString(m_spriteBatch.get(), winText, DirectX::XMFLOAT2(x, y), DirectX::Colors::Lime);
            }
        }
        break;
    }
    
    // Draw log output in the bottom half of the screen
    if (m_spriteFont)
    {
        int width, height;
        GetDefaultSize(width, height);
        float logAreaStartY = static_cast<float>(height) * 0.5f; // Start from middle of screen
        float logX = 10.0f;
        float logY = logAreaStartY + 10.0f;
        const float lineHeight = 14.0f; // Smaller font for logs (reduced further)
        
        // Get a copy of log buffer (thread-safe)
        std::deque<std::string> logCopy;
        {
            std::lock_guard<std::mutex> lock(m_logMutex);
            logCopy = m_logBuffer;
        }
        
        // Draw the most recent log lines (up to c_maxLogLines)
        size_t startIdx = 0;
        if (logCopy.size() > c_maxLogLines)
        {
            startIdx = logCopy.size() - c_maxLogLines;
        }
        
        float currentY = logY;
        for (size_t i = startIdx; i < logCopy.size() && currentY < static_cast<float>(height) - 10.0f; ++i)
        {
            // Convert std::string to wchar_t*
            const std::string& logLine = logCopy[i];
            size_t len = logLine.length();
            std::vector<wchar_t> wstr(len + 1);
            size_t converted = 0;
            mbstowcs_s(&converted, wstr.data(), len + 1, logLine.c_str(), len);
            
            // Draw log line
            m_spriteFont->DrawString(
                m_spriteBatch.get(),
                wstr.data(),
                DirectX::XMFLOAT2(logX, currentY),
                DirectX::Colors::LightGray,
                0.0f,
                DirectX::XMFLOAT2(0.0f, 0.0f),
                0.45f); // Scale down more for logs (reduced further to fit more text)
            
            currentY += lineHeight;
        }
    }
    
        m_spriteBatch->End();
    }

    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();

    // Commit graphics memory
    m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

    PIXEndEvent(m_deviceResources->GetCommandQueue());
}

// Reset game state for snake game
void Game::ResetGame()
{
    m_score = 0;
    m_time = 0.0f;
    m_moveAccumulator = 0.0f;
    m_direction = Direction::Right;
    m_nextDirection = Direction::Right;
    
    // Initialize random number generator
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // Clear snake and initialize with initial length
    m_snake.clear();
    
    int width, height;
    GetDefaultSize(width, height);
    
    // Start snake in center, facing right
    float startX = static_cast<float>(width) * 0.5f;
    float startY = static_cast<float>(height) * 0.5f;
    
    // Align to grid
    startX = floorf(startX / c_cellSize) * c_cellSize + c_cellSize * 0.5f;
    startY = floorf(startY / c_cellSize) * c_cellSize + c_cellSize * 0.5f;
    
    // Create initial snake (head + 2 segments, all in a row)
    for (int i = 0; i < c_initialSnakeLength; ++i)
    {
        DirectX::XMFLOAT2 segment;
        segment.x = startX - static_cast<float>(i) * c_cellSize;
        segment.y = startY;
        m_snake.push_back(segment);
    }
    
    // Spawn initial food
    m_food.alive = true;
    SpawnFoodNotOnSnake();
    
    StopRumble(); // Ensure rumble is stopped when starting
}

// Move snake one step in current direction
void Game::MoveSnakeOneStep()
{
    if (m_snake.empty())
        return;
    
    // Calculate next head position based on direction
    DirectX::XMFLOAT2 head = m_snake.front();
    DirectX::XMFLOAT2 nextHead = head;
    
    switch (m_direction)
    {
    case Direction::Up:
        nextHead.y -= c_cellSize;
        break;
    case Direction::Down:
        nextHead.y += c_cellSize;
        break;
    case Direction::Left:
        nextHead.x -= c_cellSize;
        break;
    case Direction::Right:
        nextHead.x += c_cellSize;
        break;
    }
    
    // Check boundary collision
    int width, height;
    GetDefaultSize(width, height);
    
    if (nextHead.x < 0.0f || nextHead.x >= static_cast<float>(width) ||
        nextHead.y < 0.0f || nextHead.y >= static_cast<float>(height))
    {
        // Hit boundary - game over
        m_state = GameState::GameOver;
        StopRumble();
        return;
    }
    
    // Check self collision (compare with all body segments except head)
    for (size_t i = 1; i < m_snake.size(); ++i)
    {
        const DirectX::XMFLOAT2& segment = m_snake[i];
        if (nextHead.x == segment.x && nextHead.y == segment.y)
        {
            // Hit self - game over
            m_state = GameState::GameOver;
            StopRumble();
            return;
        }
    }
    
    // Move snake: add new head
    m_snake.push_front(nextHead);
    
    // Check if food is eaten (head position matches food position)
    if (nextHead.x == m_food.pos.x && nextHead.y == m_food.pos.y)
    {
        // Food eaten - grow snake (don't pop tail), increase score, spawn new food
        m_score++;
        StartRumble(0.6f, 0.7f, 0.0f, 0.0f, 0.08f); // Light rumble when eating
        SpawnFoodNotOnSnake();
    }
    else
    {
        // Normal movement - remove tail
        m_snake.pop_back();
    }
}

// Spawn food at random position not on snake
void Game::SpawnFoodNotOnSnake()
{
    int width, height;
    GetDefaultSize(width, height);
    
    const float margin = 50.0f; // Margin from screen edges
    const int minCellX = static_cast<int>(ceilf(margin / c_cellSize));
    const int maxCellX = static_cast<int>(floorf((static_cast<float>(width) - margin) / c_cellSize));
    const int minCellY = static_cast<int>(ceilf(margin / c_cellSize));
    const int maxCellY = static_cast<int>(floorf((static_cast<float>(height) - margin) / c_cellSize));
    
    // Try to find a valid position (not on snake)
    const int maxAttempts = 100;
    for (int attempt = 0; attempt < maxAttempts; ++attempt)
    {
        int cellX = minCellX + (rand() % (maxCellX - minCellX + 1));
        int cellY = minCellY + (rand() % (maxCellY - minCellY + 1));
        
        // Convert to pixel position (center of cell)
        DirectX::XMFLOAT2 foodPos;
        foodPos.x = static_cast<float>(cellX) * c_cellSize + c_cellSize * 0.5f;
        foodPos.y = static_cast<float>(cellY) * c_cellSize + c_cellSize * 0.5f;
        
        // Check if position is on snake
        bool onSnake = false;
        for (const auto& segment : m_snake)
        {
            if (foodPos.x == segment.x && foodPos.y == segment.y)
            {
                onSnake = true;
                break;
            }
        }
        
        if (!onSnake)
        {
            m_food.pos = foodPos;
            m_food.alive = true;
            return;
        }
    }
    
    // Fallback: if we can't find a position after maxAttempts, place at a default location
    m_food.pos.x = static_cast<float>(width) * 0.5f;
    m_food.pos.y = static_cast<float>(height) * 0.5f;
    m_food.alive = true;
}

// Start rumble feedback
void Game::StartRumble(float lowFrequency, float highFrequency, float leftTrigger, float rightTrigger, float durationSeconds)
{
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
    if (!m_activeGamepadDevice)
    {
#ifdef _DEBUG
        AddLog("StartRumble: No active gamepad device\n");
#endif
        return;
    }
    
    // Clamp values to valid range [0.0, 1.0]
    lowFrequency = std::clamp(lowFrequency, 0.0f, 1.0f);
    highFrequency = std::clamp(highFrequency, 0.0f, 1.0f);
    leftTrigger = std::clamp(leftTrigger, 0.0f, 1.0f);
    rightTrigger = std::clamp(rightTrigger, 0.0f, 1.0f);
    durationSeconds = std::max(0.0f, durationSeconds);
    
    // Set rumble state
    GameInput::v3::GameInputRumbleParams rumbleParams = {};
    rumbleParams.lowFrequency = lowFrequency;
    rumbleParams.highFrequency = highFrequency;
    rumbleParams.leftTrigger = leftTrigger;
    rumbleParams.rightTrigger = rightTrigger;
    
    m_activeGamepadDevice->SetRumbleState(&rumbleParams);
    
    // Set timer
    m_rumbleTimeLeft = durationSeconds;
    
#ifdef _DEBUG
    char debugMsg[256];
    sprintf_s(debugMsg, "StartRumble: low=%.2f, high=%.2f, duration=%.2f, device=%p\n", 
        lowFrequency, highFrequency, durationSeconds, m_activeGamepadDevice.Get());
    AddLog(debugMsg);
#endif
#endif
}

// Update rumble timer and stop when expired
void Game::UpdateRumble(float elapsedTime)
{
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
    if (m_rumbleTimeLeft > 0.0f)
    {
        m_rumbleTimeLeft -= elapsedTime;
        
        // Keep rumble active while timer is running (some devices may need continuous updates)
        if (m_rumbleTimeLeft > 0.0f && m_activeGamepadDevice)
        {
            // Re-apply rumble state each frame to ensure it stays active
            GameInput::v3::GameInputRumbleParams rumbleParams = {};
            // Calculate current intensity (could fade out, but for now keep constant)
            // Use higher values for Xbox controllers
            rumbleParams.lowFrequency = 0.6f;  // Increased for better feel
            rumbleParams.highFrequency = 0.7f; // Increased for better feel
            rumbleParams.leftTrigger = 0.0f;
            rumbleParams.rightTrigger = 0.0f;
            m_activeGamepadDevice->SetRumbleState(&rumbleParams);
        }
        
        // Stop rumble when timer expires
        if (m_rumbleTimeLeft <= 0.0f)
        {
            StopRumble();
        }
    }
#endif
}

// Stop rumble feedback
void Game::StopRumble()
{
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
    if (!m_activeGamepadDevice)
        return;
    
    // Set rumble to zero
    GameInput::v3::GameInputRumbleParams rumbleParams = {};
    rumbleParams.lowFrequency = 0.0f;
    rumbleParams.highFrequency = 0.0f;
    rumbleParams.leftTrigger = 0.0f;
    rumbleParams.rightTrigger = 0.0f;
    
    m_activeGamepadDevice->SetRumbleState(&rumbleParams);
    
    // Reset timer
    m_rumbleTimeLeft = 0.0f;
#endif
}

// Logging helper function - adds message to log buffer and also outputs to debug console
void Game::AddLog(const char* message)
{
    if (!message)
        return;
    
    // Output to debug console
#ifdef _DEBUG
    OutputDebugStringA(message);
#endif
    
    // Add to log buffer (thread-safe)
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        
        // Remove newline if present (we'll add it when displaying)
        std::string logLine = message;
        if (!logLine.empty() && logLine.back() == '\n')
        {
            logLine.pop_back();
        }
        
        m_logBuffer.push_back(logLine);
        
        // Limit buffer size
        if (m_logBuffer.size() > c_maxLogLines * 2) // Keep more in buffer than displayed
        {
            m_logBuffer.pop_front();
        }
    }
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    const auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    const auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    const auto viewport = m_deviceResources->GetScreenViewport();
    const auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    const auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Check Shader Model 6 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
    {
#ifdef _DEBUG
        AddLog("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    // Initialize DirectX Tool Kit for DX12
    m_graphicsMemory = std::make_unique<DirectX::DX12::GraphicsMemory>(device);
    m_commonStates = std::make_unique<DirectX::DX12::CommonStates>(device);
    
    // Create SRV descriptor heap for textures (font + placeholder)
    m_srvDescriptorHeap = std::make_unique<DirectX::DescriptorHeap>(
        device,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        2); // Two descriptors: one for font texture, one for placeholder texture
    
    // Create ResourceUploadBatch for SpriteBatch initialization
    DirectX::ResourceUploadBatch upload(device);
    upload.Begin();
    
    // Create RenderTargetState for SpriteBatch
    DirectX::RenderTargetState rtState(
        m_deviceResources->GetBackBufferFormat(),
        m_deviceResources->GetDepthBufferFormat());
    
    // Create SpriteBatch for 2D rendering
    DirectX::DX12::SpriteBatchPipelineStateDescription psoDesc(
        rtState,
        &DirectX::DX12::CommonStates::NonPremultiplied);
    
    // Get viewport from DeviceResources
    const D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
    
    m_spriteBatch = std::make_unique<DirectX::DX12::SpriteBatch>(
        device, upload, psoDesc, &viewport);
    
    // Create placeholder texture (1x1 white texture) for player sprite
    {
        // Create 1x1 white texture data
        const uint32_t whitePixel = 0xFFFFFFFF; // RGBA: White
        D3D12_SUBRESOURCE_DATA initData = {};
        initData.pData = &whitePixel;
        initData.RowPitch = sizeof(uint32_t);
        initData.SlicePitch = sizeof(uint32_t);
        
        // Create texture
        DX::ThrowIfFailed(CreateTextureFromMemory(
            device,
            upload,
            1, 1, // 1x1 texture
            DXGI_FORMAT_R8G8B8A8_UNORM,
            initData,
            m_placeholderTexture.ReleaseAndGetAddressOf()));
        
        // Create SRV for placeholder texture (descriptor index 1)
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_srvDescriptorHeap->GetCpuHandle(1);
        m_placeholderTextureSRV = m_srvDescriptorHeap->GetGpuHandle(1);
        
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;
        device->CreateShaderResourceView(m_placeholderTexture.Get(), &srvDesc, cpuHandle);
        
#ifdef _DEBUG
        char debugMsg[256];
        sprintf_s(debugMsg, "Placeholder texture created: texture=%p, SRV.ptr=%llu\n", 
            m_placeholderTexture.Get(), m_placeholderTextureSRV.ptr);
        AddLog(debugMsg);
#endif
    }
    
    // Create SpriteFont for text rendering
    // Note: SpriteFont requires a .spritefont file generated by MakeSpriteFont tool
    // For now, we'll try to load it, but handle the case where it doesn't exist
    // You can generate a spritefont file using: MakeSpriteFont.exe "Arial" arial.spritefont
    try
    {
        // Get descriptor handles from the heap (descriptor index 0 for font)
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_srvDescriptorHeap->GetCpuHandle(0);
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_srvDescriptorHeap->GetGpuHandle(0);
        
        m_spriteFont = std::make_unique<DirectX::DX12::SpriteFont>(
            device, upload, L"Assets/arial.spritefont", cpuHandle, gpuHandle);
    }
    catch (const std::exception& e)
    {
        // Font file not found - SpriteFont will be nullptr, text rendering will be skipped
        AddLog("WARNING: Could not load arial.spritefont. Text rendering will be disabled.\n");
        AddLog("To generate the font file, use: MakeSpriteFont.exe \"Arial\" Assets/arial.spritefont\n");
        AddLog(e.what());
        AddLog("\n");
        m_spriteFont.reset();
    }
    catch (...)
    {
        // Generic exception - SpriteFont will be nullptr
        AddLog("WARNING: Could not load arial.spritefont (unknown error). Text rendering will be disabled.\n");
        AddLog("To generate the font file, use: MakeSpriteFont.exe \"Arial\" Assets/arial.spritefont\n");
        m_spriteFont.reset();
    }
    
    // Finish upload
    auto uploadFinished = upload.End(m_deviceResources->GetCommandQueue());
    uploadFinished.wait();
    
    // Initialize AudioEngine
    m_audioEngine = std::make_unique<DirectX::AudioEngine>();
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // Update SpriteBatch viewport when window size changes
    if (m_spriteBatch)
    {
        const D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
        m_spriteBatch->SetViewport(viewport);
    }
}

void Game::OnDeviceLost()
{
    // Cleanup DirectX Tool Kit resources
    m_spriteFont.reset();
    m_spriteBatch.reset();
    m_commonStates.reset();
    m_graphicsMemory.reset();
    m_audioEngine.reset();
    m_srvDescriptorHeap.reset();
    m_placeholderTexture.Reset();
    m_placeholderTextureSRV.ptr = 0;
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
