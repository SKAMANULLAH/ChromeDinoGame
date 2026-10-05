#include "Environment.h"
#include "../graphics/SpriteData.h"
#include <random>
#include <cmath>

Environment::Environment(int width, int height, int groundY)
    : m_width(width), m_height(height), m_groundY(groundY)
{
    reset();
}

void Environment::reset() {
    m_groundScroll = 0.0f;
    m_isNight = false;
    m_nightTransition = 0.0f;
    m_moonX = m_width - 80.0f;
    m_moonY = 45.0f;

    initClouds();
    initGroundBumps();
    initStars();
}

void Environment::initClouds() {
    m_clouds.clear();
    m_clouds.push_back({150.0f, 40.0f, 25.0f});
    m_clouds.push_back({450.0f, 65.0f, 30.0f});
    m_clouds.push_back({720.0f, 30.0f, 20.0f});
}

void Environment::initGroundBumps() {
    m_bumps.clear();
    for (int i = 0; i < 20; ++i) {
        m_bumps.push_back({i * 45.0f + (rand() % 15), rand() % 3});
    }
}

void Environment::initStars() {
    m_stars.clear();
    static std::mt19937 rng(777);
    std::uniform_int_distribution<int> distX(20, m_width - 20);
    std::uniform_int_distribution<int> distY(15, m_groundY - 60);
    std::uniform_int_distribution<int> distS(1, 2);
    std::uniform_real_distribution<float> distP(0.0f, 6.28f);

    for (int i = 0; i < 24; ++i) {
        m_stars.push_back({distX(rng), distY(rng), distS(rng), distP(rng)});
    }
}

uint32_t Environment::backgroundColor() const {
    if (m_nightTransition <= 0.001f) {
        return 0xFFF7F7F7; // Chrome light gray/white
    } else if (m_nightTransition >= 0.999f) {
        return 0xFF202124; // Chrome dark mode charcoal
    }

    // Linear RGB interpolation during sunset/sunrise
    uint8_t dayR = 247, dayG = 247, dayB = 247;
    uint8_t nightR = 32, nightG = 33, nightB = 36;

    float t = m_nightTransition;
    uint8_t r = static_cast<uint8_t>(dayR * (1.0f - t) + nightR * t);
    uint8_t g = static_cast<uint8_t>(dayG * (1.0f - t) + nightG * t);
    uint8_t b = static_cast<uint8_t>(dayB * (1.0f - t) + nightB * t);
    return Framebuffer::packARGB(255, r, g, b);
}

void Environment::update(float dt, float gameSpeed, int currentScore) {
    // Parallax clouds
    for (auto& cloud : m_clouds) {
        cloud.x -= cloud.speed * dt;
        if (cloud.x < -60.0f) {
            cloud.x = m_width + 30.0f;
            cloud.y = 25.0f + static_cast<float>(rand() % 50);
        }
    }

    // Ground bumps scrolling
    m_groundScroll += gameSpeed * dt;
    for (auto& bump : m_bumps) {
        bump.x -= gameSpeed * dt;
        if (bump.x < -20.0f) {
            bump.x += m_width + (rand() % 40);
            bump.type = rand() % 3;
        }
    }

    // Day/Night cycle logic: night triggers every 700 points for 250 points
    int cycle = currentScore % 700;
    m_isNight = (cycle >= 500 && cycle <= 700);

    float targetTransition = m_isNight ? 1.0f : 0.0f;
    float transitionSpeed = 1.5f;
    if (m_nightTransition < targetTransition) {
        m_nightTransition = std::min(targetTransition, m_nightTransition + transitionSpeed * dt);
    } else if (m_nightTransition > targetTransition) {
        m_nightTransition = std::max(targetTransition, m_nightTransition - transitionSpeed * dt);
    }

    // Parallax mountain scrolling
    m_mountainScrollFar += gameSpeed * 0.06f * dt;
    m_mountainScrollMid += gameSpeed * 0.18f * dt;

    // Shooting stars / Meteors in night mode
    if (m_isNight) {
        m_meteorTimer += dt;
        if (m_meteorTimer >= 3.5f) {
            m_meteorTimer = 0.0f;
            if (rand() % 100 < 65) {
                Meteor m;
                m.x = static_cast<float>(m_width - 80 - (rand() % 200));
                m.y = static_cast<float>(15 + (rand() % 35));
                m.vx = -380.0f - (rand() % 120);
                m.vy = 180.0f + (rand() % 80);
                m.life = 0.55f;
                m.maxLife = m.life;
                m_meteors.push_back(m);
            }
        }
    }

    for (size_t i = 0; i < m_meteors.size(); ) {
        m_meteors[i].life -= dt;
        m_meteors[i].x += m_meteors[i].vx * dt;
        m_meteors[i].y += m_meteors[i].vy * dt;
        if (m_meteors[i].life <= 0.0f) {
            m_meteors.erase(m_meteors.begin() + i);
        } else {
            ++i;
        }
    }

    // Twinkle stars
    for (auto& st : m_stars) {
        st.twinklePhase += dt * 3.0f;
    }
}

void Environment::renderBackground(SoftwareRasterizer& rasterizer) {
    // 1. Far Mountain Silhouette (Parallax Layer 1 - Scanline Polygon Fill)
    uint32_t farMountainCol = m_isNight ? 0xFF191A20 : 0xFFE8E8E8;
    int farOffset = static_cast<int>(m_mountainScrollFar) % 240;
    for (int bx = -farOffset - 240; bx < m_width + 240; bx += 240) {
        std::vector<QPoint> farPoly = {
            QPoint(bx, m_groundY),
            QPoint(bx + 70, m_groundY - 55),
            QPoint(bx + 130, m_groundY - 70),
            QPoint(bx + 180, m_groundY - 45),
            QPoint(bx + 240, m_groundY)
        };
        rasterizer.fillPolygonScanline(farPoly, farMountainCol);
    }

    // 2. Mid Desert Dunes (Parallax Layer 2 - Scanline Polygon Fill)
    uint32_t midDuneCol = m_isNight ? 0xFF1D2028 : 0xFFDDDDDD;
    int midOffset = static_cast<int>(m_mountainScrollMid) % 180;
    for (int bx = -midOffset - 180; bx < m_width + 180; bx += 180) {
        std::vector<QPoint> midPoly = {
            QPoint(bx, m_groundY),
            QPoint(bx + 50, m_groundY - 32),
            QPoint(bx + 110, m_groundY - 38),
            QPoint(bx + 180, m_groundY)
        };
        rasterizer.fillPolygonScanline(midPoly, midDuneCol);
    }

    // 3. Night sky features (Moon, Stars, Meteors)
    if (m_nightTransition > 0.05f) {
        // Stars rendered using Midpoint Circle and Bresenham points
        for (const auto& st : m_stars) {
            float brightness = 0.5f + 0.5f * std::sin(st.twinklePhase);
            uint8_t starAlpha = static_cast<uint8_t>(255 * m_nightTransition * brightness);
            if (starAlpha > 20) {
                uint32_t starCol = Framebuffer::packARGB(starAlpha, 230, 230, 240);
                if (st.size == 1) {
                    rasterizer.setPixel(st.x, st.y, starCol);
                } else {
                    rasterizer.fillCircleMidpoint(st.x, st.y, 1, starCol);
                    // Star sparkle cross using Bresenham lines
                    if (brightness > 0.85f) {
                        rasterizer.drawLineBresenham(st.x - 2, st.y, st.x + 2, st.y, starCol);
                        rasterizer.drawLineBresenham(st.x, st.y - 2, st.x, st.y + 2, starCol);
                    }
                }
            }
        }

        // Shooting Stars (Bresenham line with fading alpha trail)
        for (const auto& m : m_meteors) {
            float alphaF = std::clamp(m.life / m.maxLife, 0.0f, 1.0f);
            uint8_t ma = static_cast<uint8_t>(255 * alphaF);
            uint32_t headCol = Framebuffer::packARGB(ma, 255, 255, 255);
            int headX = static_cast<int>(m.x);
            int headY = static_cast<int>(m.y);
            int tailX = headX - static_cast<int>(m.vx * 0.05f);
            int tailY = headY - static_cast<int>(m.vy * 0.05f);
            rasterizer.drawLineBresenham(headX, headY, tailX, tailY, headCol);
            rasterizer.fillCircleMidpoint(headX, headY, 2, headCol);
        }

        // Moon rendering using Midpoint Circle Algorithm & Constructive Solid Geometry (CSG)
        int moonX = static_cast<int>(m_moonX);
        int moonY = static_cast<int>(m_moonY);
        int moonR = 14;

        uint8_t moonA = static_cast<uint8_t>(255 * m_nightTransition);
        uint32_t moonBodyCol = Framebuffer::packARGB(moonA, 240, 240, 245);
        rasterizer.fillCircleMidpoint(moonX, moonY, moonR, moonBodyCol);

        // Crescent cutout using sky color
        uint32_t skyBg = backgroundColor();
        rasterizer.fillCircleMidpoint(moonX + 6, moonY - 3, moonR - 2, skyBg);
    }

    // Clouds (Floating across sky)
    uint32_t cloudTint = m_isNight ? 0xFF505055 : 0xFFC4C4C4;
    for (const auto& cloud : m_clouds) {
        int cx = static_cast<int>(cloud.x);
        int cy = static_cast<int>(cloud.y);
        // Render cloud as combination of Midpoint Ellipses and Circles (pure raster!)
        rasterizer.fillEllipseMidpoint(cx + 20, cy + 8, 18, 6, cloudTint);
        rasterizer.fillCircleMidpoint(cx + 14, cy + 5, 6, cloudTint);
        rasterizer.fillCircleMidpoint(cx + 24, cy + 4, 8, cloudTint);
        rasterizer.fillCircleMidpoint(cx + 31, cy + 6, 5, cloudTint);
    }
}

void Environment::renderForeground(SoftwareRasterizer& rasterizer) {
    uint32_t lineColor = m_isNight ? 0xFFACACAC : 0xFF535353;

    // 1. Main Horizon Line (Bresenham / Xiaolin Wu if AA enabled)
    rasterizer.drawLineAuto(0, m_groundY, m_width - 1, m_groundY, lineColor);

    // 2. Procedural Ground details (Pebbles, Bumps) using Bresenham Lines and Points
    for (const auto& bump : m_bumps) {
        int bx = static_cast<int>(bump.x);
        int by = m_groundY + 3;

        if (bx < 0 || bx >= m_width - 10) continue;

        if (bump.type == 0) {
            // Tiny 2-pixel pebble
            rasterizer.setPixel(bx, by, lineColor);
            rasterizer.setPixel(bx + 1, by, lineColor);
        } else if (bump.type == 1) {
            // Stepped pebble segment
            rasterizer.drawLineBresenham(bx, by + 1, bx + 3, by + 1, lineColor);
            rasterizer.setPixel(bx + 1, by, lineColor);
        } else if (bump.type == 2) {
            // Small terrain streak
            rasterizer.drawLineBresenham(bx, by + 4, bx + 5, by + 4, lineColor);
            rasterizer.drawLineBresenham(bx + 8, by + 2, bx + 11, by + 2, lineColor);
        }
    }
}
