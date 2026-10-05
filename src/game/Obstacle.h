#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "Entity.h"
#include "../graphics/SoftwareRasterizer.h"

enum class ObstacleType {
    CactusSmall1,
    CactusSmall2,
    CactusSmall3,
    CactusBig1,
    CactusBig2,
    Pterodactyl
};

class Obstacle {
public:
    Obstacle(ObstacleType type, float x, int groundY, int pteroFlyHeightLevel = 0);

    void update(float dt, float speed);
    void render(SoftwareRasterizer& rasterizer, bool nightMode);

    bool isOffScreen() const { return m_x < -100.0f; }
    AABB boundingBox() const;
    const Sprite& currentSprite() const;
    int x() const { return static_cast<int>(m_x); }
    int y() const { return static_cast<int>(m_y); }
    ObstacleType type() const { return m_type; }

private:
    ObstacleType m_type;
    float m_x{0.0f};
    float m_y{0.0f};
    int m_groundY;
    
    // Pterodactyl animation
    float m_animTimer{0.0f};
    int m_animFrame{0};
};

#endif // OBSTACLE_H
