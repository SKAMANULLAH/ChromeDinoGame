#include "PowerUp.h"
#include "../graphics/SpriteData.h"
#include <cmath>

PowerUp::PowerUp(PowerUpType type, float x, int groundY)
    : m_type(type), m_x(x)
{
    // Floats in air at jump height so player has to jump to collect it!
    m_baseY = groundY - 55.0f;
    m_y = m_baseY;
}

void PowerUp::update(float dt, float speed) {
    m_x -= speed * dt;
    m_bobTimer += dt * 4.0f;
    // Gentle sine wave floating bob
    m_y = m_baseY + std::sin(m_bobTimer) * 6.0f;
}

AABB PowerUp::boundingBox() const {
    return {
        static_cast<int>(m_x),
        static_cast<int>(m_y),
        14,
        14
    };
}

void PowerUp::render(SoftwareRasterizer& rasterizer, bool nightMode) {
    if (m_collected) return;

    int rx = static_cast<int>(m_x);
    int ry = static_cast<int>(m_y);

    // Subtle pulsating glow circle around powerup
    uint32_t glowCol = 0x5500E5FF;
    if (m_type == PowerUpType::BulletTime) glowCol = 0x55FFD700;
    else if (m_type == PowerUpType::DoubleJump) glowCol = 0x55FF007F;

    rasterizer.fillCircleMidpoint(rx + 6, ry + 6, 9, glowCol);

    const Sprite* sp = &SpriteData::powerupShield();
    if (m_type == PowerUpType::BulletTime) sp = &SpriteData::powerupClock();
    else if (m_type == PowerUpType::DoubleJump) sp = &SpriteData::powerupFeather();

    if (nightMode) {
        Sprite inv = sp->invertedColors();
        rasterizer.blitSprite(inv, rx, ry);
    } else {
        rasterizer.blitSprite(*sp, rx, ry);
    }
}
