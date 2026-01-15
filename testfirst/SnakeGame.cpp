//
// SnakeGame.cpp
// Pure gameplay logic layer
//

#include "pch.h"
#include "SnakeGame.h"
#include <DirectXMath.h>

using namespace DirectX;

SnakeGame::SnakeGame()
    : m_direction(Direction::Right)
    , m_nextDirection(Direction::Right)
    , m_moveAccumulator(0.0f)
    , m_score(0)
    , m_gameOver(false)
    , m_screenWidth(800)
    , m_screenHeight(600)
{
    m_food.alive = false;
    srand(static_cast<unsigned int>(time(nullptr)));
}

void SnakeGame::Reset(int screenWidth, int screenHeight)
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_score = 0;
    m_moveAccumulator = 0.0f;
    m_direction = Direction::Right;
    m_nextDirection = Direction::Right;
    m_gameOver = false;

    // Clear snake and initialize with initial length
    m_snake.clear();

    // Start snake in center, facing right
    float startX = static_cast<float>(screenWidth) * 0.5f;
    float startY = static_cast<float>(screenHeight) * 0.5f;

    // Align to grid
    startX = floorf(startX / c_cellSize) * c_cellSize + c_cellSize * 0.5f;
    startY = floorf(startY / c_cellSize) * c_cellSize + c_cellSize * 0.5f;

    // Create initial snake (head + 2 segments, all in a row)
    for (int i = 0; i < c_initialSnakeLength; ++i)
    {
        XMFLOAT2 segment;
        segment.x = startX - static_cast<float>(i) * c_cellSize;
        segment.y = startY;
        m_snake.push_back(segment);
    }

    // Spawn initial food
    m_food.alive = true;
    SpawnFoodNotOnSnake();
}

void SnakeGame::QueueDirection(Direction dir)
{
    // Prevent 180-degree turn
    if ((m_direction == Direction::Up && dir != Direction::Down) ||
        (m_direction == Direction::Down && dir != Direction::Up) ||
        (m_direction == Direction::Left && dir != Direction::Right) ||
        (m_direction == Direction::Right && dir != Direction::Left))
    {
        m_nextDirection = dir;
    }
}

SnakeGameEvents SnakeGame::Update(float elapsedTime)
{
    SnakeGameEvents events = {};
    events.ateFood = false;
    events.gameOver = false;

    if (m_gameOver || m_snake.empty())
        return events;

    // Update direction from queued direction
    m_direction = m_nextDirection;

    // Accumulate time for discrete movement
    m_moveAccumulator += elapsedTime;

    // Move snake when accumulator reaches move interval
    while (m_moveAccumulator >= c_moveInterval)
    {
        // Store food position before move (in case we eat it)
        XMFLOAT2 foodPosBeforeMove = m_food.pos;

        // Move snake and check if food was eaten
        bool ateFood = MoveSnakeOneStep();

        // Check if we ate food
        if (ateFood)
        {
            events.ateFood = true;
            events.foodPos = foodPosBeforeMove;
            // Spawn new food after detecting the event
            SpawnFoodNotOnSnake();
        }

        // Check if game over
        if (m_gameOver)
        {
            events.gameOver = true;
            break;
        }

        m_moveAccumulator -= c_moveInterval;
    }

    return events;
}

bool SnakeGame::MoveSnakeOneStep()
{
    if (m_snake.empty() || m_gameOver)
        return false;

    // Calculate next head position based on direction
    XMFLOAT2 head = m_snake.front();
    XMFLOAT2 nextHead = head;

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
    default:
        // Should never reach here, but ensure function returns
        return false;
    }

    // Check boundary collision
    if (nextHead.x < 0.0f || nextHead.x >= static_cast<float>(m_screenWidth) ||
        nextHead.y < 0.0f || nextHead.y >= static_cast<float>(m_screenHeight))
    {
        // Hit boundary - game over
        m_gameOver = true;
        return false;
    }

    // Check self collision (compare with all body segments except head)
    for (size_t i = 1; i < m_snake.size(); ++i)
    {
        const XMFLOAT2& segment = m_snake[i];
        if (nextHead.x == segment.x && nextHead.y == segment.y)
        {
            // Hit self - game over
            m_gameOver = true;
            return false;
        }
    }

    // Move snake: add new head
    m_snake.push_front(nextHead);

    // Check if food is eaten (head position matches food position)
    bool ateFood = false;
    if (m_food.alive && nextHead.x == m_food.pos.x && nextHead.y == m_food.pos.y)
    {
        // Food eaten - grow snake (don't pop tail), increase score
        m_score++;
        m_food.alive = false; // Mark as eaten (will be respawned by Update)
        ateFood = true;
        // Don't spawn new food here - let Update() handle it after detecting the event
    }
    else
    {
        // Normal movement - remove tail
        m_snake.pop_back();
    }

    return ateFood;
}

void SnakeGame::SpawnFoodNotOnSnake()
{
    const float margin = 50.0f; // Margin from screen edges
    const int minCellX = static_cast<int>(ceilf(margin / c_cellSize));
    const int maxCellX = static_cast<int>(floorf((static_cast<float>(m_screenWidth) - margin) / c_cellSize));
    const int minCellY = static_cast<int>(ceilf(margin / c_cellSize));
    const int maxCellY = static_cast<int>(floorf((static_cast<float>(m_screenHeight) - margin) / c_cellSize));

    // Try to find a valid position (not on snake)
    const int maxAttempts = 100;
    for (int attempt = 0; attempt < maxAttempts; ++attempt)
    {
        int cellX = minCellX + (rand() % (maxCellX - minCellX + 1));
        int cellY = minCellY + (rand() % (maxCellY - minCellY + 1));

        // Convert to pixel position (center of cell)
        XMFLOAT2 foodPos;
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
    m_food.pos.x = static_cast<float>(m_screenWidth) * 0.5f;
    m_food.pos.y = static_cast<float>(m_screenHeight) * 0.5f;
    m_food.alive = true;
}
