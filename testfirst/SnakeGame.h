//
// SnakeGame.h
// Pure gameplay logic layer (no D3D12, no DirectXTK dependencies)
//

#pragma once

#include <deque>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

// Forward declarations
namespace DirectX
{
    struct XMFLOAT2;
}

// Direction enumeration for snake movement
enum class Direction
{
    Up,
    Down,
    Left,
    Right
};

// Food structure
struct Food
{
    DirectX::XMFLOAT2 pos;  // Position (grid-aligned)
    bool alive;             // Always true for food
};

// Game events returned from Update
struct SnakeGameEvents
{
    bool ateFood;           // True if food was eaten this frame
    DirectX::XMFLOAT2 foodPos; // Position where food was eaten (if ateFood is true)
    bool gameOver;          // True if game over occurred this frame
};

// Pure gameplay logic for snake game
class SnakeGame
{
public:
    SnakeGame();
    ~SnakeGame() = default;

    // Reset game state
    void Reset(int screenWidth, int screenHeight);

    // Queue a direction change (prevents 180-degree turns)
    void QueueDirection(Direction dir);

    // Update game logic (returns events)
    SnakeGameEvents Update(float elapsedTime);

    // Getters
    const std::deque<DirectX::XMFLOAT2>& GetSnakeSegments() const { return m_snake; }
    const Food& GetFood() const { return m_food; }
    int GetScore() const { return m_score; }
    size_t GetLength() const { return m_snake.size(); }
    bool IsGameOver() const { return m_gameOver; }

private:
    bool MoveSnakeOneStep();  // Returns true if food was eaten
    void SpawnFoodNotOnSnake();

    // Game constants
    static constexpr float c_cellSize = 20.0f;  // Grid cell size in pixels
    static constexpr float c_moveInterval = 0.10f;  // Time between moves (10 cells/second)
    static constexpr int c_initialSnakeLength = 3;  // Initial snake length (head + 2 segments)

    // Game state
    std::deque<DirectX::XMFLOAT2> m_snake;  // Snake body segments (grid-aligned positions)
    Direction m_direction;  // Current movement direction
    Direction m_nextDirection;  // Queued direction (prevents 180-degree turns)
    float m_moveAccumulator;  // Accumulated time for discrete movement
    Food m_food;  // Single food item
    int m_score;
    bool m_gameOver;

    // Screen bounds
    int m_screenWidth;
    int m_screenHeight;
};
