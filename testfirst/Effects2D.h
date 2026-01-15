//
// Effects2D.h
// Effects layer: particles and screen shake
//

#pragma once

#include <vector>
#include <DirectXMath.h>

// Forward declarations
namespace DirectX
{
    namespace DX12
    {
        class SpriteBatch;
    }
}

// Particle structure
struct Particle
{
    DirectX::XMFLOAT2 pos;
    DirectX::XMFLOAT2 velocity;
    DirectX::XMFLOAT4 color;
    float lifetime;  // Remaining lifetime in seconds
    float maxLifetime;  // Total lifetime in seconds
    float size;
};

// Effects system for 2D rendering (particles + screen shake)
class Effects2D
{
public:
    Effects2D();
    ~Effects2D() = default;

    // Trigger effects
    void OnEatFood(const DirectX::XMFLOAT2& foodPos);

    // Update effects
    void Update(float elapsedTime);

    // Draw particles (requires SpriteBatch and placeholder texture SRV)
    void Draw(
        DirectX::DX12::SpriteBatch* spriteBatch,
        D3D12_GPU_DESCRIPTOR_HANDLE placeholderSRV,
        const DirectX::XMFLOAT2& cameraOffset) const;

    // Get current camera offset from screen shake
    DirectX::XMFLOAT2 GetCameraOffset() const { return m_cameraOffset; }

private:
    void SpawnEatParticles(const DirectX::XMFLOAT2& pos);
    void StartScreenShake(float intensity, float duration);

    // Particles
    std::vector<Particle> m_particles;

    // Screen shake
    DirectX::XMFLOAT2 m_cameraOffset;
    float m_shakeIntensity;
    float m_shakeTimeLeft;
    float m_shakeDuration;

    // Constants
    static constexpr int c_particlesPerEat = 12;  // Increased from 8 for more visible effect
    static constexpr float c_particleLifetime = 0.6f;  // Increased from 0.5f
    static constexpr float c_particleSpeed = 150.0f;  // Increased from 100.0f
    static constexpr float c_shakeIntensity = 8.0f;  // Increased from 5.0f for more visible shake
    static constexpr float c_shakeDuration = 0.2f;  // Increased from 0.15f
};
