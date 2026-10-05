#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <vector>
#include <cstdint>
#include "../graphics/SoftwareRasterizer.h"

/**
 * @file ParticleSystem.h
 * @brief Discrete 2D Particle Engine rendered via Midpoint Circles and Bresenham lines.
 */
class ParticleSystem {
public:
    enum class ParticleType {
        Dust,       // Midpoint circle puff
        Spark,      // Bresenham velocity streak
        StarTwinkle // Bresenham cross sparkle
    };

    struct Particle {
        float x{0.0f};
        float y{0.0f};
        float vx{0.0f};
        float vy{0.0f};
        float life{1.0f};
        float maxLife{1.0f};
        int radius{2};
        uint32_t color{0xFF535353};
        ParticleType type{ParticleType::Dust};
    };

    ParticleSystem() = default;

    void update(float dt);
    void render(SoftwareRasterizer& rasterizer);
    void clear();

    // Spawners
    void emitDustPuff(float x, float y, int count = 3, uint32_t color = 0xFF888888);
    void emitCollisionSparks(float x, float y, int count = 12, uint32_t color = 0xFFFF5555);
    void emitStarTwinkle(float x, float y, uint32_t color = 0xFFFFFFFF);

private:
    std::vector<Particle> m_particles;
};

#endif // PARTICLESYSTEM_H
