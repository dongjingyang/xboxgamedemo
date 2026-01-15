//
// Effects2D.cpp
// Effects layer implementation
//

#include "pch.h"
#include "Effects2D.h"
#include <SpriteBatch.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <cstdlib>
#include <cmath>

using namespace DirectX;

Effects2D::Effects2D()
    : m_shakeIntensity(0.0f)
    , m_shakeTimeLeft(0.0f)
    , m_shakeDuration(0.0f)
{
    m_cameraOffset = XMFLOAT2(0.0f, 0.0f);
}

void Effects2D::OnEatFood(const DirectX::XMFLOAT2& foodPos)
{
    SpawnEatParticles(foodPos);
    StartScreenShake(c_shakeIntensity, c_shakeDuration);
#ifdef _DEBUG
    // Log particle count for debugging
    char debugMsg[128];
    sprintf_s(debugMsg, "Effects2D: Spawned %d particles, shake intensity=%.1f\n", 
        static_cast<int>(m_particles.size()), c_shakeIntensity);
    OutputDebugStringA(debugMsg);
#endif
}

void Effects2D::Update(float elapsedTime)
{
    // Update particles
    for (auto it = m_particles.begin(); it != m_particles.end();)
    {
        Particle& p = *it;
        p.lifetime -= elapsedTime;

        if (p.lifetime <= 0.0f)
        {
            // Particle expired, remove it
            it = m_particles.erase(it);
        }
        else
        {
            // Update position
            p.pos.x += p.velocity.x * elapsedTime;
            p.pos.y += p.velocity.y * elapsedTime;

            // Fade out color based on lifetime
            float t = p.lifetime / p.maxLifetime;
            p.color.w = t; // Alpha fade

            ++it;
        }
    }

    // Update screen shake
    if (m_shakeTimeLeft > 0.0f)
    {
        m_shakeTimeLeft -= elapsedTime;

        if (m_shakeTimeLeft > 0.0f)
        {
            // Generate random offset (decay over time)
            float t = m_shakeTimeLeft / m_shakeDuration;
            float currentIntensity = m_shakeIntensity * t;

            // Random offset in range [-currentIntensity, currentIntensity]
            float angle = static_cast<float>(rand()) / RAND_MAX * XM_2PI;
            float distance = static_cast<float>(rand()) / RAND_MAX * currentIntensity;

            m_cameraOffset.x = cosf(angle) * distance;
            m_cameraOffset.y = sinf(angle) * distance;
        }
        else
        {
            // Shake finished
            m_cameraOffset = XMFLOAT2(0.0f, 0.0f);
            m_shakeTimeLeft = 0.0f;
        }
    }
}

void Effects2D::Draw(
    DirectX::DX12::SpriteBatch* spriteBatch,
    D3D12_GPU_DESCRIPTOR_HANDLE placeholderSRV,
    const DirectX::XMFLOAT2& cameraOffset) const
{
    if (!spriteBatch || placeholderSRV.ptr == 0)
        return;

    // Draw all particles
    for (const auto& particle : m_particles)
    {
        // Apply camera offset to particle position
        XMFLOAT2 drawPos;
        drawPos.x = particle.pos.x + cameraOffset.x - particle.size * 0.5f;
        drawPos.y = particle.pos.y + cameraOffset.y - particle.size * 0.5f;

        // Convert color to XMVECTOR
        XMVECTOR colorVec = XMVectorSet(
            particle.color.x,
            particle.color.y,
            particle.color.z,
            particle.color.w);

        spriteBatch->Draw(
            placeholderSRV,
            XMUINT2(1, 1),
            drawPos,
            nullptr,
            colorVec,
            0.0f,
            XMFLOAT2(0.0f, 0.0f),
            particle.size);
    }
}

void Effects2D::SpawnEatParticles(const DirectX::XMFLOAT2& pos)
{
    for (int i = 0; i < c_particlesPerEat; ++i)
    {
        Particle p;
        p.pos = pos;
        p.maxLifetime = c_particleLifetime;
        p.lifetime = c_particleLifetime;
        p.size = 12.0f + static_cast<float>(rand() % 12); // Random size 12-24 (increased for visibility)

        // Random velocity in all directions
        float angle = static_cast<float>(rand()) / RAND_MAX * XM_2PI;
        float speed = c_particleSpeed * (0.5f + static_cast<float>(rand()) / RAND_MAX * 0.5f); // 50-100% speed
        p.velocity.x = cosf(angle) * speed;
        p.velocity.y = sinf(angle) * speed;

        // Gold color with full alpha
        p.color = XMFLOAT4(1.0f, 0.84f, 0.0f, 1.0f); // Gold

        m_particles.push_back(p);
    }
}

void Effects2D::StartScreenShake(float intensity, float duration)
{
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
    m_shakeTimeLeft = duration;
}
