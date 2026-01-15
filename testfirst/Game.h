//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"

#include <memory>
#include <deque>
#include <string>
#include <mutex>
#include <vector>

// Include GameInput header if available
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
// Temporarily modify WINAPI_FAMILY to include WINAPI_PARTITION_APP for gameinput.h
// gameinput.h requires WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM | WINAPI_PARTITION_GAMES
#pragma push_macro("WINAPI_FAMILY")
#undef WINAPI_FAMILY
#define WINAPI_FAMILY WINAPI_FAMILY_APP
#include <gameinput.h>
#pragma pop_macro("WINAPI_FAMILY")
#endif

// DirectX Tool Kit forward declarations
// Note: AudioEngine needs full definition for std::unique_ptr, so include Audio.h
#include <Audio.h>

namespace DirectX
{
    class DescriptorHeap;
    
    namespace DX12
    {
        class GraphicsMemory;
        class SpriteBatch;
        class SpriteFont;
        class CommonStates;
    }
}

// Game state enumeration
enum class GameState
{
    Title,      // Title screen - "Press A to Start"
    Playing,    // Game is running
    Paused,     // Game is paused - "Paused - Press Start"
    Win,        // Game won - "You Win - Press A to Restart" (kept for compatibility)
    GameOver    // Game over - "Game Over - Press A to Restart"
};

// Direction enumeration for snake movement
enum class Direction
{
    Up,
    Down,
    Left,
    Right
};

// Food structure (reusing Coin name for compatibility)
struct Coin
{
    DirectX::XMFLOAT2 pos;  // Position (grid-aligned)
    bool alive;             // Always true for food
};

// A basic game implementation that creates a D3D12 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    
    // Logging helper function
    void AddLog(const char* message);
    
    // Snake game system
    void ResetGame();  // Reset game state (snake, score, food, direction)
    void MoveSnakeOneStep();  // Move snake one cell in current direction
    void SpawnFoodNotOnSnake();  // Spawn food at random position not on snake
    
    // Rumble system
    void StartRumble(float lowFrequency, float highFrequency, float leftTrigger, float rightTrigger, float durationSeconds);
    void UpdateRumble(float elapsedTime);
    void StopRumble();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>        m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                               m_timer;

    // DirectX Tool Kit for DX12
    std::unique_ptr<DirectX::DX12::GraphicsMemory> m_graphicsMemory;
    std::unique_ptr<DirectX::DX12::SpriteBatch>  m_spriteBatch;
    std::unique_ptr<DirectX::DX12::SpriteFont>   m_spriteFont;
    std::unique_ptr<DirectX::DX12::CommonStates> m_commonStates;
    std::unique_ptr<DirectX::AudioEngine>        m_audioEngine;
    
    // Descriptor heap for textures (SpriteFont + placeholder texture)
    std::unique_ptr<DirectX::DescriptorHeap>    m_srvDescriptorHeap;
    
    // Game state
    GameState                                   m_state;
    DirectX::XMFLOAT2                            m_playerPos;  // Kept for compatibility, not used in snake game
    int                                         m_score;
    float                                       m_time;
    
    // Snake game
    std::deque<DirectX::XMFLOAT2>               m_snake;  // Snake body segments (grid-aligned positions)
    Direction                                    m_direction;  // Current movement direction
    Direction                                    m_nextDirection;  // Queued direction (prevents 180-degree turns)
    float                                       m_moveAccumulator;  // Accumulated time for discrete movement
    Coin                                        m_food;  // Single food item (always alive=true)
    
    // Snake game constants
    static constexpr float                      c_cellSize = 20.0f;  // Grid cell size in pixels
    static constexpr float                      c_moveInterval = 0.10f;  // Time between moves (10 cells/second)
    static constexpr int                        c_initialSnakeLength = 3;  // Initial snake length (head + 2 segments)
    
    // Placeholder texture for player sprite (1x1 white texture)
    Microsoft::WRL::ComPtr<ID3D12Resource>      m_placeholderTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE                 m_placeholderTextureSRV;
    
    // GameInput (forward declared to avoid header dependencies)
    void*                                        m_gameInput; // IGameInput* when GameInput is available
    uint64_t                                     m_lastButtonState; // Track button state to detect presses
    
    // Gamepad device for rumble (only when GameInput is available)
#if defined(USING_GAMEINPUT) || defined(_GAMING_DESKTOP) || defined(_GAMING_XBOX)
    Microsoft::WRL::ComPtr<GameInput::v3::IGameInputDevice> m_activeGamepadDevice;
#endif
    float                                        m_rumbleTimeLeft; // Remaining rumble time in seconds
    
    // Log buffer for on-screen display
    std::deque<std::string>                      m_logBuffer;
    static constexpr size_t                      c_maxLogLines = 20; // Maximum number of log lines to display
    mutable std::mutex                           m_logMutex; // Thread-safe log access
};
