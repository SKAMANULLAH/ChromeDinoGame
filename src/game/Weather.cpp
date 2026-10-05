#include "Weather.h"
#include <random>
#include <cmath>
#include <algorithm>

Weather::Weather(int width, int height, int groundY)
    : m_width(width), m_height(height), m_groundY(groundY)
{
    reset();
}

void Weather::reset() {
    m_type = WeatherType::Clear;
    m_weatherAutoTimer = 0.0f;
    m_lightningFlashTimer = 0.0f;
    m_nextLightningTimer = 4.0f;
    m_raindrops.clear();
    m_splashes.clear();
    m_sandParticles.clear();
}

void Weather::cycleWeather() {
    int cur = static_cast<int>(m_type);
    cur = (cur + 1) % 4;
    setWeather(static_cast<WeatherType>(cur));
}

void Weather::setWeather(WeatherType type) {
    m_type = type;
    m_weatherAutoTimer = 0.0f;
    m_raindrops.clear();
    m_splashes.clear();
    m_sandParticles.clear();
    m_lightningFlashTimer = 0.0f;

    if (m_type == WeatherType::Rain || m_type == WeatherType::Thunderstorm) {
        initRain();
    } else if (m_type == WeatherType::Sandstorm) {
        initSand();
    }
}

const char* Weather::weatherName() const {
    switch (m_type) {
        case WeatherType::Clear: return "Clear Skies";
        case WeatherType::Rain: return "Desert Rain";
        case WeatherType::Thunderstorm: return "Thunderstorm & Lightning";
        case WeatherType::Sandstorm: return "Dust Sandstorm";
    }
    return "Clear";
}

void Weather::initRain() {
    int count = (m_type == WeatherType::Thunderstorm) ? 90 : 50;
    m_raindrops.reserve(count);
    for (int i = 0; i < count; ++i) {
        m_raindrops.push_back({
            static_cast<float>(rand() % (m_width + 40)),
            static_cast<float>(rand() % m_groundY),
            400.0f + static_cast<float>(rand() % 250)
        });
    }
}

void Weather::initSand() {
    m_sandParticles.reserve(70);
    static const uint32_t SAND_COLORS[3] = { 0xCCDEB887, 0xCCD2B48C, 0xCCF4A460 };
    for (int i = 0; i < 70; ++i) {
        m_sandParticles.push_back({
            static_cast<float>(rand() % m_width),
            static_cast<float>(rand() % m_groundY),
            -450.0f - static_cast<float>(rand() % 300),
            -20.0f + static_cast<float>(rand() % 40),
            SAND_COLORS[rand() % 3]
        });
    }
}

void Weather::update(float dt, float gameSpeed) {
    // Automatic weather progression: cycles every 35 seconds
    m_weatherAutoTimer += dt;
    if (m_weatherAutoTimer >= 35.0f) {
        m_weatherAutoTimer = 0.0f;
        cycleWeather();
    }

    if (m_type == WeatherType::Rain || m_type == WeatherType::Thunderstorm) {
        for (auto& drop : m_raindrops) {
            drop.y += drop.speed * dt;
            drop.x -= (gameSpeed * 0.2f + 50.0f) * dt; // Angled fall

            if (drop.y >= static_cast<float>(m_groundY)) {
                // Splash ripple
                if (m_splashes.size() < 40) {
                    m_splashes.push_back({
                        static_cast<int>(drop.x),
                        m_groundY,
                        0.18f,
                        0.18f,
                        1 + (rand() % 3)
                    });
                }
                drop.y = -10.0f;
                drop.x = static_cast<float>(rand() % (m_width + 60));
            }
        }

        // Update splash ripples
        for (size_t i = 0; i < m_splashes.size(); ) {
            m_splashes[i].life -= dt;
            if (m_splashes[i].life <= 0.0f) {
                m_splashes.erase(m_splashes.begin() + i);
            } else {
                ++i;
            }
        }

        // Thunderstorm lightning flashes
        if (m_type == WeatherType::Thunderstorm) {
            if (m_lightningFlashTimer > 0.0f) {
                m_lightningFlashTimer -= dt;
            } else {
                m_nextLightningTimer -= dt;
                if (m_nextLightningTimer <= 0.0f) {
                    m_lightningFlashTimer = 0.09f; // 90ms flash
                    m_nextLightningTimer = 3.5f + static_cast<float>(rand() % 5);
                }
            }
        }
    } else if (m_type == WeatherType::Sandstorm) {
        for (auto& s : m_sandParticles) {
            s.x += (s.vx - gameSpeed * 0.5f) * dt;
            s.y += s.vy * dt;
            if (s.x < -10.0f) {
                s.x = static_cast<float>(m_width + (rand() % 30));
                s.y = static_cast<float>(rand() % m_groundY);
            }
        }
    }
}

void Weather::render(SoftwareRasterizer& rasterizer) {
    if (m_type == WeatherType::Clear) return;

    if (m_type == WeatherType::Rain || m_type == WeatherType::Thunderstorm) {
        uint32_t rainCol = (m_type == WeatherType::Thunderstorm) ? 0xDD88B0D0 : 0xAA99BBDD;

        // Falling raindrops (Angled lines: Bresenham / Xiaolin Wu)
        for (const auto& drop : m_raindrops) {
            int x = static_cast<int>(drop.x);
            int y = static_cast<int>(drop.y);
            rasterizer.drawLineAuto(x, y, x - 1, y + 6, rainCol);
        }

        // Ground splash ripples (Midpoint horizontal spans)
        for (const auto& sp : m_splashes) {
            float progress = 1.0f - (sp.life / sp.maxLife);
            int curR = std::max(1, static_cast<int>(sp.radius * (1.0f + progress)));
            uint8_t a = static_cast<uint8_t>(std::clamp(180.0f * (1.0f - progress), 0.0f, 255.0f));
            uint32_t rippleCol = Framebuffer::packARGB(a, 150, 190, 225);
            rasterizer.drawLineAuto(sp.x - curR, sp.y, sp.x + curR, sp.y, rippleCol);
        }

        // Lightning flash overlay
        if (m_type == WeatherType::Thunderstorm && m_lightningFlashTimer > 0.0f) {
            int w = rasterizer.framebuffer().width();
            int h = rasterizer.framebuffer().height();
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    rasterizer.framebuffer().setPixelBlend(x, y, 0xBBEDF7FA);
                }
            }
        }
    } else if (m_type == WeatherType::Sandstorm) {
        // Sandstorm particles with proper alpha blending
        for (const auto& s : m_sandParticles) {
            int px = static_cast<int>(s.x);
            int py = static_cast<int>(s.y);
            rasterizer.framebuffer().setPixelBlend(px, py, s.color);
            rasterizer.framebuffer().setPixelBlend(px + 1, py, s.color);
        }
    }
}
