#ifndef POWERUP_H
#define POWERUP_H

#include "../graphics/SoftwareRasterizer.h"
#include "Entity.h"

enum class PowerUpType {
    Shield,
    BulletTime,
    DoubleJump
};

class PowerUp {
public:
    PowerUp(PowerUpType type, float x, int groundY);

    void update(float dt, float speed);
    void render(SoftwareRasterizer& rasterizer, bool nightMode);

    bool isOffScreen() const { return m_x < -40.0f; }
    bool isCollected() const { return m_collected; }
    void collect() { m_collected = true; }

    PowerUpType type() const { return m_type; }
    AABB boundingBox() const;

    int x() const { return static_cast<int>(m_x); }
    int y() const { return static_cast<int>(m_y); }

private:
    PowerUpType m_type;
    float m_x{0.0f};
    float m_baseY{0.0f};
    float m_y{0.0f};
    float m_bobTimer{0.0f};
    bool m_collected{false};
};

#endif // POWERUP_H
