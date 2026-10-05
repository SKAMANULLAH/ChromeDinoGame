#include "ParticleSystem.h"
#include <random>

void ParticleSystem::update(float dt) {
    for (size_t i = 0; i < m_particles.size(); ) {
        auto& p = m_particles[i];
        p.life -= dt;
        if (p.life <= 0.0f) {
            m_particles.erase(m_particles.begin() + i);
            continue;
        }

        p.x += p.vx * dt;
        p.y += p.vy * dt;

        // Apply slight gravity or friction depending on type
        if (p.type == ParticleType::Dust) {
            p.vx *= (1.0f - 1.5f * dt); // Ground friction
            p.vy += 80.0f * dt;         // Gravity
        } else if (p.type == ParticleType::Spark) {
            p.vy += 300.0f * dt;        // Heavy gravity
        }
        ++i;
    }
}

void ParticleSystem::render(SoftwareRasterizer& rasterizer) {
    for (const auto& p : m_particles) {
        float alphaFactor = std::clamp(p.life / p.maxLife, 0.0f, 1.0f);
        uint8_t a, r, g, b;
        Framebuffer::unpackARGB(p.color, a, r, g, b);
        uint8_t finalA = static_cast<uint8_t>(a * alphaFactor);
        uint32_t fadedColor = Framebuffer::packARGB(finalA, r, g, b);

        int px = static_cast<int>(p.x);
        int py = static_cast<int>(p.y);

        if (p.type == ParticleType::Dust) {
            // Drawn using Midpoint Circle algorithm
            int curR = std::max(1, static_cast<int>(p.radius * alphaFactor));
            rasterizer.fillCircleMidpoint(px, py, curR, fadedColor);
        } else if (p.type == ParticleType::Spark) {
            // Drawn using Bresenham Line algorithm showing velocity trail
            int tailX = px - static_cast<int>(p.vx * 0.03f);
            int tailY = py - static_cast<int>(p.vy * 0.03f);
            rasterizer.drawLineBresenham(px, py, tailX, tailY, fadedColor);
        } else if (p.type == ParticleType::StarTwinkle) {
            // Drawn using Bresenham cross sparkle
            int len = 2;
            rasterizer.drawLineBresenham(px - len, py, px + len, py, fadedColor);
            rasterizer.drawLineBresenham(px, py - len, px, py + len, fadedColor);
        }
    }
}

void ParticleSystem::clear() {
    m_particles.clear();
}

void ParticleSystem::emitDustPuff(float x, float y, int count, uint32_t color) {
    static std::mt19937 rng(42);
    std::uniform_real_distribution<float> distVx(-40.0f, -10.0f);
    std::uniform_real_distribution<float> distVy(-30.0f, -5.0f);
    std::uniform_real_distribution<float> distLife(0.3f, 0.6f);
    std::uniform_int_distribution<int> distR(1, 3);

    for (int i = 0; i < count; ++i) {
        Particle p;
        p.x = x;
        p.y = y;
        p.vx = distVx(rng);
        p.vy = distVy(rng);
        p.life = distLife(rng);
        p.maxLife = p.life;
        p.radius = distR(rng);
        p.color = color;
        p.type = ParticleType::Dust;
        m_particles.push_back(p);
    }
}

void ParticleSystem::emitCollisionSparks(float x, float y, int count, uint32_t color) {
    static std::mt19937 rng(1337);
    std::uniform_real_distribution<float> distAngle(0.0f, 6.28318f);
    std::uniform_real_distribution<float> distSpeed(60.0f, 180.0f);
    std::uniform_real_distribution<float> distLife(0.4f, 0.8f);

    for (int i = 0; i < count; ++i) {
        float angle = distAngle(rng);
        float speed = distSpeed(rng);
        Particle p;
        p.x = x;
        p.y = y;
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed - 50.0f; // Initial upward burst
        p.life = distLife(rng);
        p.maxLife = p.life;
        p.radius = 2;
        p.color = color;
        p.type = ParticleType::Spark;
        m_particles.push_back(p);
    }
}

void ParticleSystem::emitStarTwinkle(float x, float y, uint32_t color) {
    Particle p;
    p.x = x;
    p.y = y;
    p.vx = 0.0f;
    p.vy = 0.0f;
    p.life = 0.5f;
    p.maxLife = 0.5f;
    p.radius = 1;
    p.color = color;
    p.type = ParticleType::StarTwinkle;
    m_particles.push_back(p);
}
