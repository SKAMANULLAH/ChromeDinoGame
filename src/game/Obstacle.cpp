#include "Obstacle.h"
#include "../graphics/SpriteData.h"

Obstacle::Obstacle(ObstacleType type, float x, int groundY, int pteroFlyHeightLevel)
    : m_type(type), m_x(x), m_groundY(groundY)
{
    if (m_type == ObstacleType::CactusSmall1) {
        m_y = groundY - SpriteData::cactusSmall1().height();
    } else if (m_type == ObstacleType::CactusSmall2) {
        m_y = groundY - SpriteData::cactusSmall2().height();
    } else if (m_type == ObstacleType::CactusSmall3) {
        m_y = groundY - SpriteData::cactusSmall3().height();
    } else if (m_type == ObstacleType::CactusBig1) {
        m_y = groundY - SpriteData::cactusBig1().height();
    } else if (m_type == ObstacleType::CactusBig2) {
        m_y = groundY - SpriteData::cactusBig2().height();
    } else if (m_type == ObstacleType::Pterodactyl) {
        // 3 heights: 0 = low (must duck), 1 = mid (jump or duck), 2 = high (overhead)
        if (pteroFlyHeightLevel == 0) {
            m_y = groundY - 32; // Low: Dino head height (duck needed)
        } else if (pteroFlyHeightLevel == 1) {
            m_y = groundY - 62; // Mid: Jumpable
        } else {
            m_y = groundY - 95; // High: Overhead
        }
    }
}

void Obstacle::update(float dt, float speed) {
    m_x -= speed * dt;

    if (m_type == ObstacleType::Pterodactyl) {
        m_animTimer += dt;
        if (m_animTimer >= 0.14f) {
            m_animTimer = 0.0f;
            m_animFrame = (m_animFrame + 1) % 2;
        }
    }
}

const Sprite& Obstacle::currentSprite() const {
    switch (m_type) {
        case ObstacleType::CactusSmall1:
            return SpriteData::cactusSmall1();
        case ObstacleType::CactusSmall2:
            return SpriteData::cactusSmall2();
        case ObstacleType::CactusSmall3:
            return SpriteData::cactusSmall3();
        case ObstacleType::CactusBig1:
            return SpriteData::cactusBig1();
        case ObstacleType::CactusBig2:
            return SpriteData::cactusBig2();
        case ObstacleType::Pterodactyl:
            return (m_animFrame == 0) ? SpriteData::pteroWingUp() : SpriteData::pteroWingDown();
    }
    return SpriteData::cactusSmall1();
}

AABB Obstacle::boundingBox() const {
    const Sprite& sp = currentSprite();
    int insetX = 4;
    int insetY = 4;
    return {
        static_cast<int>(m_x) + insetX,
        static_cast<int>(m_y) + insetY,
        sp.width() - (insetX * 2),
        sp.height() - (insetY * 2)
    };
}

void Obstacle::render(SoftwareRasterizer& rasterizer, bool nightMode) {
    const Sprite& base = currentSprite();
    int rx = static_cast<int>(m_x);
    int ry = static_cast<int>(m_y);

    if (nightMode) {
        Sprite inv = base.invertedColors();
        rasterizer.blitSprite(inv, rx, ry);
    } else {
        rasterizer.blitSprite(base, rx, ry);
    }
}
