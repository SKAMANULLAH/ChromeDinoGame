#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../graphics/SoftwareRasterizer.h"
#include <vector>

struct Cloud {
    float x;
    float y;
    float speed;
};

struct GroundBump {
    float x;
    int type; // 0 = single pebble, 1 = double pebble, 2 = bumpy line
};

struct Star {
    int x;
    int y;
    int size;
    float twinklePhase;
};

struct Meteor {
    float x;
    float y;
    float vx;
    float vy;
    float life;
    float maxLife;
};

class Environment {
public:
    Environment(int width = 800, int height = 300, int groundY = 220);

    void reset();
    void update(float dt, float gameSpeed, int currentScore);
    void renderBackground(SoftwareRasterizer& rasterizer);
    void renderForeground(SoftwareRasterizer& rasterizer);

    bool isNightMode() const { return m_isNight; }
    uint32_t backgroundColor() const;

private:
    int m_width;
    int m_height;
    int m_groundY;

    // Scrolling ground
    float m_groundScroll{0.0f};
    std::vector<GroundBump> m_bumps;

    // Parallax Mountains
    float m_mountainScrollFar{0.0f};
    float m_mountainScrollMid{0.0f};

    // Clouds
    std::vector<Cloud> m_clouds;

    // Night cycle & Celestial bodies
    bool m_isNight{false};
    float m_nightTransition{0.0f}; // 0.0 (day) to 1.0 (night)
    float m_moonX{700.0f};
    float m_moonY{50.0f};
    std::vector<Star> m_stars;
    std::vector<Meteor> m_meteors;
    float m_meteorTimer{0.0f};

    void initClouds();
    void initGroundBumps();
    void initStars();
};

#endif // ENVIRONMENT_H
