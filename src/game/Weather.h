#ifndef WEATHER_H
#define WEATHER_H

#include "../graphics/SoftwareRasterizer.h"
#include <vector>

enum class WeatherType {
    Clear,
    Rain,
    Thunderstorm,
    Sandstorm
};

struct Raindrop {
    float x;
    float y;
    float speed;
};

struct SplashRipple {
    int x;
    int y;
    float life;
    float maxLife;
    int radius;
};

struct SandParticle {
    float x;
    float y;
    float vx;
    float vy;
    uint32_t color;
};

class Weather {
public:
    Weather(int width = 800, int height = 300, int groundY = 225);

    void reset();
    void update(float dt, float gameSpeed);
    void render(SoftwareRasterizer& rasterizer);

    void cycleWeather();
    void setWeather(WeatherType type);
    WeatherType type() const { return m_type; }
    const char* weatherName() const;

    bool isLightningActive() const { return m_lightningFlashTimer > 0.0f; }

private:
    int m_width;
    int m_height;
    int m_groundY;

    WeatherType m_type{WeatherType::Clear};
    float m_weatherAutoTimer{0.0f};

    // Rain & Ripples
    std::vector<Raindrop> m_raindrops;
    std::vector<SplashRipple> m_splashes;

    // Thunderstorm
    float m_lightningFlashTimer{0.0f};
    float m_nextLightningTimer{5.0f};

    // Sandstorm
    std::vector<SandParticle> m_sandParticles;

    void initRain();
    void initSand();
};

#endif // WEATHER_H
