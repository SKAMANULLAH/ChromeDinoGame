#include "Obstacle.h"
#include "Terrain.h"
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
    } else if (m_type == ObstacleType::Bat) {
        // 3 heights: 0 = low (must duck), 1 = mid (jump or duck), 2 = high (overhead)
        if (pteroFlyHeightLevel == 0) {
            m_y = groundY - 42; // Low: Standing Dino (43h) hits; Ducking Dino (26h) ducks cleanly
        } else if (pteroFlyHeightLevel == 1) {
            m_y = groundY - 65; // Mid: Jumpable or duckable
        } else {
            m_y = groundY - 95; // High: Overhead
        }
        m_baseY = m_y;
    }
}

void Obstacle::update(float dt, float speed, const Terrain* terrain) {
    m_x -= speed * dt;

    if (m_type == ObstacleType::Pterodactyl) {
        m_animTimer += dt;
        if (m_animTimer >= 0.14f) {
            m_animTimer = 0.0f;
            m_animFrame = (m_animFrame + 1) % 2;
        }
    } else if (m_type == ObstacleType::Bat) {
        m_animTimer += dt;
        if (m_animTimer >= 0.09f) {
            m_animTimer = 0.0f;
            m_animFrame = (m_animFrame + 1) % 2;
        }
        m_flutterTimer += dt;
        m_y = m_baseY + std::sin(m_flutterTimer * 8.0f) * 2.0f;
    } else if (terrain) {
        // Cacti ride along the undulating terrain surface
        float curGround = terrain->getGroundHeightAt(m_x + currentSprite().width() * 0.5f);
        m_y = curGround - currentSprite().height();
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
        case ObstacleType::Bat:
            return (m_animFrame == 0) ? SpriteData::batWingUp() : SpriteData::batWingDown();
    }
    return SpriteData::cactusSmall1();
}

AABB Obstacle::boundingBox() const {
    const Sprite& sp = currentSprite();
    int insetX = (m_type == ObstacleType::Bat) ? 3 : 4;
    int insetY = (m_type == ObstacleType::Bat) ? 2 : 4;
    return {
        static_cast<int>(m_x) + insetX,
        static_cast<int>(m_y) + insetY,
        sp.width() - (insetX * 2),
        sp.height() - (insetY * 2)
    };
}

void Obstacle::render(SoftwareRasterizer& rasterizer, bool nightMode, float cameraY, bool /*inCave*/) {
    const Sprite& base = currentSprite();
    int rx = static_cast<int>(m_x);
    int ry = static_cast<int>(m_y - cameraY);

    if (nightMode) {
        Sprite inv = base.invertedColors();
        rasterizer.blitSprite(inv, rx, ry);
    } else {
        rasterizer.blitSprite(base, rx, ry);
    }

    if (m_type == ObstacleType::Bat) {
        // Glowing ruby red / crimson bat eyes
        uint32_t eyeCol = 0xFFFF1744;
        int eyeY = (m_animFrame == 0) ? (ry + 6) : (ry + 4);
        rasterizer.setPixel(rx + 9, eyeY, eyeCol);
        rasterizer.setPixel(rx + 14, eyeY, eyeCol);
    }
}
