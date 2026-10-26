#include "Terrain.h"
#include <cmath>
#include <algorithm>

Terrain::Terrain(int width, int height, float defaultGroundY)
    : m_width(width), m_height(height), m_defaultGroundY(defaultGroundY), m_rng(98765)
{
    reset();
}

void Terrain::reset() {
    m_segments.clear();
    m_lastSpecialBiome = TerrainType::Flat;
    m_biomeCooldown = 1;
    generateInitialSegments();
}

void Terrain::generateInitialSegments() {
    // Start with a runway so player can get ready
    m_segments.push_back(createFlatSegment(-60.0f, 520.0f, m_defaultGroundY));
    // Follow with a welcoming gentle dune
    m_segments.push_back(createDuneSegment(460.0f, 440.0f, m_defaultGroundY, 26.0f, 1));
    // Follow with early floating platform archipelago
    m_segments.push_back(createPlatformSegment(900.0f, 480.0f, m_defaultGroundY));
    // Flat buffer runway
    m_segments.push_back(createFlatSegment(1380.0f, 350.0f, m_defaultGroundY));
}

TerrainSegment Terrain::createFlatSegment(float startX, float length, float groundY) {
    TerrainSegment seg;
    seg.type = TerrainType::Flat;
    seg.startX = startX;
    seg.endX = startX + length;
    seg.startGroundY = groundY;
    seg.endGroundY = groundY;
    return seg;
}

TerrainSegment Terrain::createDuneSegment(float startX, float length, float groundY, float amplitude, int waves) {
    TerrainSegment seg;
    seg.type = TerrainType::Dunes;
    seg.startX = startX;
    seg.endX = startX + length;
    seg.startGroundY = groundY;
    seg.endGroundY = groundY;
    seg.duneAmplitude = amplitude;
    seg.duneWaves = std::max(1, waves);
    return seg;
}

TerrainSegment Terrain::createPlatformSegment(float startX, float length, float groundY) {
    TerrainSegment seg;
    seg.type = TerrainType::FloatingPlatforms;
    seg.startX = startX;
    seg.endX = startX + length;
    seg.startGroundY = groundY;
    seg.endGroundY = groundY;

    // Platform 1: Lower stepping stone
    FloatingPlatform p1;
    p1.x = startX + 60.0f;
    p1.y = groundY - 55.0f; // y ~ 170
    p1.width = 130.0f;
    p1.height = 14.0f;
    seg.platforms.push_back(p1);

    // Platform 2: Higher vantage perch
    FloatingPlatform p2;
    p2.x = startX + 230.0f;
    p2.y = groundY - 78.0f; // y ~ 147
    p2.width = 140.0f;
    p2.height = 14.0f;
    seg.platforms.push_back(p2);

    if (length >= 520.0f) {
        // Platform 3: Exit step
        FloatingPlatform p3;
        p3.x = startX + 410.0f;
        p3.y = groundY - 58.0f;
        p3.width = 110.0f;
        p3.height = 14.0f;
        seg.platforms.push_back(p3);
    }

    return seg;
}

TerrainSegment Terrain::createSpikeSegment(float startX, float length, float groundY) {
    TerrainSegment seg;
    seg.type = TerrainType::Spikes;
    seg.startX = startX;
    seg.endX = startX + length;
    seg.startGroundY = groundY;
    seg.endGroundY = groundY;

    // Cluster 1
    SpikeHazard s1;
    s1.x = startX + 110.0f;
    s1.y = groundY;
    s1.width = 16.0f;
    s1.height = 20.0f;
    s1.count = 2; // 2 spike teeth
    s1.onCeiling = false;
    seg.spikes.push_back(s1);

    // Cluster 2 (if segment long enough)
    if (length >= 420.0f) {
        SpikeHazard s2;
        s2.x = startX + 290.0f;
        s2.y = groundY;
        s2.width = 16.0f;
        s2.height = 22.0f;
        s2.count = 1;
        s2.onCeiling = false;
        seg.spikes.push_back(s2);
    }

    return seg;
}

TerrainSegment Terrain::createLavaSegment(float startX, float length, float groundY) {
    TerrainSegment seg;
    seg.type = TerrainType::LavaPool;
    seg.startX = startX;
    seg.endX = startX + length;
    seg.startGroundY = groundY;
    seg.endGroundY = groundY;

    float pitStartX = startX + 110.0f;
    float pitWidth = 135.0f;

    LavaPool pool;
    pool.x = pitStartX;
    pool.width = pitWidth;
    pool.surfaceY = groundY + 14.0f; // y ~ 239
    pool.depth = 45.0f;
    pool.waveTimer = static_cast<float>(m_rng() % 100);
    pool.bubbleSpawnTimer = 0.0f;
    seg.lavaPools.push_back(pool);

    // Floating stepping stone platform centered above the lava gorge
    FloatingPlatform bridge;
    bridge.x = pitStartX + (pitWidth - 76.0f) * 0.5f;
    bridge.y = groundY - 48.0f; // y ~ 177
    bridge.width = 76.0f;
    bridge.height = 14.0f;
    seg.platforms.push_back(bridge);

    return seg;
}

TerrainSegment Terrain::createCaveSegment(float startX, float length, float groundY) {
    TerrainSegment seg;
    seg.type = TerrainType::Cave;
    seg.startX = startX;
    seg.endX = startX + length;
    seg.startGroundY = groundY;
    seg.endGroundY = groundY;

    CaveZone cave;
    cave.startX = startX;
    cave.endX = startX + length;
    cave.ceilingY = 144.0f;
    cave.floorY = groundY;

    // Stalactite 1: High ceiling decoration (safe to run under)
    Stalactite st1;
    st1.x = startX + 160.0f;
    st1.y = cave.ceilingY;
    st1.width = 16.0f;
    st1.height = 22.0f; // tip at 166 (clears 178 standing Dino)
    cave.stalactites.push_back(st1);

    // Stalactite 2: Low danger hazard (MUST DUCK! Tip at 190, duck Dino height is 195)
    Stalactite st2;
    st2.x = startX + 320.0f;
    st2.y = cave.ceilingY;
    st2.width = 18.0f;
    st2.height = 46.0f; // tip at 190
    cave.stalactites.push_back(st2);

    if (length >= 580.0f) {
        // Stalactite 3: Medium stalactite
        Stalactite st3;
        st3.x = startX + 470.0f;
        st3.y = cave.ceilingY;
        st3.width = 16.0f;
        st3.height = 28.0f;
        cave.stalactites.push_back(st3);
    }

    seg.caves.push_back(cave);
    return seg;
}

void Terrain::generateNextSegment(int score) {
    float startX = m_segments.empty() ? 0.0f : m_segments.back().endX;
    float startY = m_segments.empty() ? m_defaultGroundY : m_segments.back().endGroundY;

    if (m_biomeCooldown > 0) {
        m_biomeCooldown--;
        // Pacing buffer: flats or rolling dunes
        if ((m_rng() % 100) < 55) {
            float len = 320.0f + static_cast<float>(m_rng() % 150);
            m_segments.push_back(createFlatSegment(startX, len, startY));
        } else {
            float len = 400.0f + static_cast<float>(m_rng() % 160);
            float amp = 24.0f + static_cast<float>(m_rng() % 16);
            m_segments.push_back(createDuneSegment(startX, len, startY, amp, 1));
        }
        return;
    }

    // Roll for special biome
    std::vector<TerrainType> candidates = {
        TerrainType::Dunes,
        TerrainType::FloatingPlatforms,
        TerrainType::Spikes,
        TerrainType::LavaPool,
        TerrainType::Cave
    };

    // Pick a candidate not matching last special biome
    TerrainType chosen = candidates[m_rng() % candidates.size()];
    if (chosen == m_lastSpecialBiome && candidates.size() > 1) {
        chosen = candidates[(m_rng() + 1) % candidates.size()];
    }

    m_lastSpecialBiome = chosen;
    m_biomeCooldown = 1; // Always place at least one buffer after a hazard/special zone

    switch (chosen) {
        case TerrainType::Flat: {
            float len = 350.0f + static_cast<float>(m_rng() % 140);
            m_segments.push_back(createFlatSegment(startX, len, startY));
            break;
        }
        case TerrainType::Dunes: {
            float len = 460.0f + static_cast<float>(m_rng() % 160);
            float amp = 28.0f + static_cast<float>(m_rng() % 18);
            int waves = ((m_rng() % 100) < 40) ? 2 : 1;
            m_segments.push_back(createDuneSegment(startX, len, startY, amp, waves));
            break;
        }
        case TerrainType::FloatingPlatforms: {
            float len = 480.0f + static_cast<float>(m_rng() % 140);
            m_segments.push_back(createPlatformSegment(startX, len, startY));
            break;
        }
        case TerrainType::Spikes: {
            float len = 420.0f + static_cast<float>(m_rng() % 120);
            m_segments.push_back(createSpikeSegment(startX, len, startY));
            break;
        }
        case TerrainType::LavaPool: {
            float len = 500.0f + static_cast<float>(m_rng() % 120);
            m_segments.push_back(createLavaSegment(startX, len, startY));
            break;
        }
        case TerrainType::Cave: {
            float len = 580.0f + static_cast<float>(m_rng() % 160);
            m_segments.push_back(createCaveSegment(startX, len, startY));
            break;
        }
    }
}

void Terrain::update(float dt, float speed, int score, ParticleSystem& particles) {
    float dx = speed * dt;

    for (auto& seg : m_segments) {
        seg.startX -= dx;
        seg.endX -= dx;

        for (auto& plat : seg.platforms) {
            plat.x -= dx;
        }
        for (auto& spk : seg.spikes) {
            spk.x -= dx;
        }
        for (auto& cave : seg.caves) {
            cave.startX -= dx;
            cave.endX -= dx;
            for (auto& stal : cave.stalactites) {
                stal.x -= dx;
            }
        }
        for (auto& pool : seg.lavaPools) {
            pool.x -= dx;
            pool.waveTimer += dt;
            pool.bubbleSpawnTimer += dt;

            // Spawn bubbling magma particles
            if (pool.bubbleSpawnTimer >= 0.22f) {
                pool.bubbleSpawnTimer = 0.0f;
                if (pool.x + pool.width > 0.0f && pool.x < static_cast<float>(m_width)) {
                    LavaBubble b;
                    float span = std::max(10.0f, pool.width - 20.0f);
                    b.x = pool.x + 10.0f + static_cast<float>(m_rng() % static_cast<int>(span));
                    b.y = pool.surfaceY + pool.depth - 4.0f;
                    b.vy = -18.0f - static_cast<float>(m_rng() % 14);
                    b.r = 1.5f + static_cast<float>(m_rng() % 2);
                    b.maxR = b.r + 1.5f;
                    b.life = 0.9f;
                    b.maxLife = b.life;
                    pool.bubbles.push_back(b);
                }
            }

            // Update rising bubbles
            for (size_t bi = 0; bi < pool.bubbles.size(); ) {
                auto& b = pool.bubbles[bi];
                b.life -= dt;
                b.y += b.vy * dt;

                if (b.y <= pool.surfaceY || b.life <= 0.0f) {
                    // Bubble pops into tiny fire sparks!
                    if (b.x >= 0.0f && b.x <= static_cast<float>(m_width)) {
                        particles.emitCollisionSparks(b.x, pool.surfaceY, 2, 0xFFFF7700);
                    }
                    pool.bubbles.erase(pool.bubbles.begin() + bi);
                } else {
                    ++bi;
                }
            }
        }
    }

    // Prune segments that have scrolled completely off screen
    while (!m_segments.empty() && m_segments.front().endX < -250.0f) {
        m_segments.erase(m_segments.begin());
    }

    // Generate upcoming segments seamlessly to horizon
    while (m_segments.empty() || m_segments.back().endX < static_cast<float>(m_width + 850)) {
        generateNextSegment(score);
    }
}

float Terrain::getGroundHeightAt(float screenX) const {
    for (const auto& seg : m_segments) {
        if (screenX >= seg.startX && screenX <= seg.endX) {
            float len = seg.endX - seg.startX;
            if (len <= 0.001f) return seg.startGroundY;

            if (seg.type == TerrainType::Dunes) {
                float u = std::clamp((screenX - seg.startX) / len, 0.0f, 1.0f);
                float waveFactor = 1.0f + 0.32f * std::sin(2.0f * 3.14159265f * static_cast<float>(seg.duneWaves) * u);
                float h = seg.duneAmplitude * std::sin(3.14159265f * u) * waveFactor;
                return seg.startGroundY - h;
            }

            if (seg.type == TerrainType::LavaPool) {
                for (const auto& pool : seg.lavaPools) {
                    if (screenX >= pool.x && screenX <= pool.x + pool.width) {
                        return pool.surfaceY + 35.0f; // Drop into chasm
                    }
                }
                return seg.startGroundY;
            }

            return seg.startGroundY;
        }
    }
    return m_defaultGroundY;
}

float Terrain::getSurfaceHeightAt(float screenX, float footY, float /*vy*/, bool& onPlatform, FloatingPlatform*& outPlatform) {
    onPlatform = false;
    outPlatform = nullptr;

    // Check if Dino is currently resting on a floating platform
    for (auto& seg : m_segments) {
        for (auto& plat : seg.platforms) {
            if (screenX >= plat.x - 4.0f && screenX <= plat.x + plat.width + 4.0f) {
                if (std::abs(footY - plat.y) <= 8.0f) {
                    onPlatform = true;
                    outPlatform = &plat;
                    return plat.y;
                }
            }
        }
    }

    return getGroundHeightAt(screenX);
}

float Terrain::getLandingSurfaceAt(float screenX, float footY, float vy, bool& onPlatform, FloatingPlatform*& outPlatform) {
    onPlatform = false;
    outPlatform = nullptr;

    float bestY = getGroundHeightAt(screenX);

    // One-way landing check on platforms
    for (auto& seg : m_segments) {
        for (auto& plat : seg.platforms) {
            if (screenX >= plat.x - 6.0f && screenX <= plat.x + plat.width + 6.0f) {
                if (footY >= plat.y - 8.0f && footY <= plat.y + 16.0f && vy >= 0.0f) {
                    if (plat.y < bestY) {
                        bestY = plat.y;
                        onPlatform = true;
                        outPlatform = &plat;
                    }
                }
            }
        }
    }

    return bestY;
}

float Terrain::getSlopeAngleAt(float screenX) const {
    float y1 = getGroundHeightAt(screenX - 8.0f);
    float y2 = getGroundHeightAt(screenX + 8.0f);
    float angleRad = std::atan2(y2 - y1, 16.0f);
    return angleRad * (180.0f / 3.14159265f);
}

float Terrain::getCeilingHeightAt(float screenX) const {
    for (const auto& seg : m_segments) {
        for (const auto& cave : seg.caves) {
            if (screenX >= cave.startX && screenX <= cave.endX) {
                float entranceArchLen = 80.0f;
                // Arch down at entrance
                if (screenX < cave.startX + entranceArchLen) {
                    float u = (screenX - cave.startX) / entranceArchLen;
                    return cave.ceilingY * u;
                }
                // Arch up at exit
                if (screenX > cave.endX - entranceArchLen) {
                    float u = (cave.endX - screenX) / entranceArchLen;
                    return cave.ceilingY * u;
                }
                // Main cavern body
                return cave.ceilingY;
            }
        }
    }
    return 0.0f;
}

bool Terrain::isInCave(float screenX) const {
    for (const auto& seg : m_segments) {
        for (const auto& cave : seg.caves) {
            if (screenX >= cave.startX + 80.0f && screenX <= cave.endX - 80.0f) {
                return true;
            }
        }
    }
    return false;
}

bool Terrain::isOverLava(float screenX) const {
    for (const auto& seg : m_segments) {
        for (const auto& pool : seg.lavaPools) {
            if (screenX >= pool.x && screenX <= pool.x + pool.width) {
                return true;
            }
        }
    }
    return false;
}

TerrainType Terrain::currentBiomeAt(float screenX) const {
    for (const auto& seg : m_segments) {
        if (screenX >= seg.startX && screenX <= seg.endX) {
            return seg.type;
        }
    }
    return TerrainType::Flat;
}

const char* Terrain::currentBiomeName(float screenX) const {
    TerrainType t = currentBiomeAt(screenX);
    switch (t) {
        case TerrainType::Flat: return "DESERT FLATS";
        case TerrainType::Dunes: return "SAND DUNES";
        case TerrainType::FloatingPlatforms: return "MESA PLATFORMS";
        case TerrainType::Spikes: return "SPIKE FIELD";
        case TerrainType::LavaPool: return "LAVA GORGE";
        case TerrainType::Cave: return "CAVERN TUNNEL";
    }
    return "DESERT";
}

bool Terrain::checkHazardCollision(const AABB& dinoBox, bool /*isDucking*/, HazardType& outHazard, float& hitX, float& hitY) {
    outHazard = HazardType::None;

    // 1. Spikes Hazard
    for (const auto& seg : m_segments) {
        for (const auto& spk : seg.spikes) {
            AABB sb = spk.boundingBox();
            if (dinoBox.intersects(sb)) {
                outHazard = HazardType::Spike;
                hitX = spk.x + (spk.width * spk.count) * 0.5f;
                hitY = spk.onCeiling ? (spk.y + spk.height * 0.5f) : (spk.y - spk.height * 0.5f);
                return true;
            }
        }
    }

    // 2. Lava Pool Hazard
    float dinoFeetY = static_cast<float>(dinoBox.y + dinoBox.h);
    float dinoCenterX = static_cast<float>(dinoBox.x + dinoBox.w / 2);
    for (const auto& seg : m_segments) {
        for (const auto& pool : seg.lavaPools) {
            if (dinoCenterX >= pool.x + 6.0f && dinoCenterX <= pool.x + pool.width - 6.0f) {
                if (dinoFeetY >= pool.surfaceY - 2.0f) {
                    outHazard = HazardType::Lava;
                    hitX = dinoCenterX;
                    hitY = pool.surfaceY;
                    return true;
                }
            }
        }
    }

    // 3. Cave Stalactite Hazard (Low hanging rock stalactites)
    for (const auto& seg : m_segments) {
        for (const auto& cave : seg.caves) {
            for (const auto& stal : cave.stalactites) {
                AABB stb = stal.boundingBox();
                if (dinoBox.intersects(stb)) {
                    outHazard = HazardType::Stalactite;
                    hitX = stal.x + stal.width * 0.5f;
                    hitY = stal.y + stal.height;
                    return true;
                }
            }
        }
    }

    return false;
}

void Terrain::renderBackgroundCave(SoftwareRasterizer& rasterizer, bool nightMode) {
    // Render cavern back wall inside cave sections
    for (const auto& seg : m_segments) {
        for (const auto& cave : seg.caves) {
            int cx0 = std::max(0, static_cast<int>(cave.startX));
            int cx1 = std::min(m_width, static_cast<int>(cave.endX));
            if (cx0 >= cx1) continue;

            uint32_t caveBgCol = nightMode ? 0xFF101118 : 0xFF282732;
            uint32_t strataCol = nightMode ? 0xFF191B24 : 0xFF353442;

            rasterizer.fillRectScanline(cx0, 0, cx1 - cx0, m_height, caveBgCol);

            // Cavern rock strata lines
            rasterizer.drawDottedLineBresenham(cx0, 60, cx1, 60, strataCol, 8);
            rasterizer.drawDottedLineBresenham(cx0, 110, cx1, 110, strataCol, 12);
            rasterizer.drawDottedLineBresenham(cx0, 175, cx1, 175, strataCol, 6);
        }
    }
}

void Terrain::renderForeground(SoftwareRasterizer& rasterizer, bool nightMode, bool cyberpunk) {
    uint32_t lineCol = nightMode ? 0xFFACACAC : 0xFF535353;
    uint32_t groundBodyCol = nightMode ? 0xFF15161C : 0xFFEFEFEF;
    uint32_t platCol = nightMode ? 0xFF2C2E3B : 0xFF656570;
    uint32_t platTopCol = nightMode ? 0xFF84889C : 0xFFA5A5B5;
    uint32_t platBotCol = nightMode ? 0xFF181920 : 0xFF3C3C44;
    uint32_t spikeCol = nightMode ? 0xFF585B6C : 0xFF42424D;
    uint32_t spikeLineCol = nightMode ? 0xFFD8DCEE : 0xFF1C1C24;

    if (cyberpunk) {
        lineCol = 0xFFFF007F;       // Neon Pink
        groundBodyCol = 0xFF120822; // Deep Indigo
        platCol = 0xFF1F0D3D;
        platTopCol = 0xFF00E5FF;    // Cyan Highlight
        platBotCol = 0xFF080214;
        spikeCol = 0xFFBD00FF;      // Laser Violet
        spikeLineCol = 0xFF00FFFF;  // Neon Cyan Edge
    }

    // 1. Render Ground & Dunes (Pure Software Rasterization)
    for (int x = 0; x < m_width; ++x) {
        float fx = static_cast<float>(x);

        // Check if inside a lava pool
        bool inLava = false;
        for (const auto& seg : m_segments) {
            for (const auto& pool : seg.lavaPools) {
                if (fx >= pool.x && fx <= pool.x + pool.width) {
                    inLava = true;
                    break;
                }
            }
            if (inLava) break;
        }

        if (inLava) continue;

        int gy = static_cast<int>(std::round(getGroundHeightAt(fx)));
        gy = std::clamp(gy, 50, m_height - 1);

        // Ground surface contour pixel
        rasterizer.setPixel(x, gy, lineCol);

        // Solid ground body fill below surface
        for (int y = gy + 1; y < m_height; ++y) {
            rasterizer.setPixel(x, y, groundBodyCol);
        }

        // Procedural rock/sediment specks
        if ((x * 37 + gy * 19) % 47 == 0 && gy + 5 < m_height) {
            rasterizer.setPixel(x, gy + 4, lineCol);
        }
        if ((x * 13 + gy * 7) % 61 == 0 && gy + 9 < m_height) {
            rasterizer.setPixel(x, gy + 8, lineCol);
            rasterizer.setPixel(x + 1, gy + 8, lineCol);
        }
    }

    // 2. Render Lava Pools
    for (const auto& seg : m_segments) {
        for (const auto& pool : seg.lavaPools) {
            if (pool.x + pool.width < 0.0f || pool.x >= static_cast<float>(m_width)) continue;

            int px = static_cast<int>(pool.x);
            int pw = static_cast<int>(pool.width);
            int sy = static_cast<int>(pool.surfaceY);
            int pyBottom = std::min(m_height - 1, sy + static_cast<int>(pool.depth));

            // Deep magma red basin
            uint32_t magmaCol = cyberpunk ? 0xFF800030 : 0xFFB31200;
            rasterizer.fillRectScanline(px, sy + 3, pw, pyBottom - (sy + 3), magmaCol);

            // Sub-surface orange band
            uint32_t orangeCol = cyberpunk ? 0xFFFF0066 : 0xFFFF4500;
            rasterizer.fillRectScanline(px, sy + 1, pw, 3, orangeCol);

            // Animated undulating molten surface line
            uint32_t goldCol = cyberpunk ? 0xFF00FFFF : 0xFFFFEE44;
            for (int x = px; x < px + pw; ++x) {
                if (x < 0 || x >= m_width) continue;
                float fx = static_cast<float>(x);
                float wy = pool.surfaceY + 2.2f * std::sin(pool.waveTimer * 5.0f + fx * 0.09f)
                                         + 1.2f * std::cos(pool.waveTimer * 3.2f + fx * 0.15f);
                int iwy = static_cast<int>(wy);
                rasterizer.setPixel(x, iwy, goldCol);
                rasterizer.setPixel(x, iwy + 1, orangeCol);
            }

            // Rocky bank borders (left and right edges of chasm)
            rasterizer.drawLineBresenham(px, static_cast<int>(m_defaultGroundY), px, pyBottom, lineCol);
            rasterizer.drawLineBresenham(px + pw, static_cast<int>(m_defaultGroundY), px + pw, pyBottom, lineCol);

            // Rising animated bubbles
            for (const auto& b : pool.bubbles) {
                int bx = static_cast<int>(b.x);
                int by = static_cast<int>(b.y);
                int br = static_cast<int>(b.r);
                if (bx >= 0 && bx < m_width && by >= sy && by < m_height) {
                    rasterizer.fillCircleMidpoint(bx, by, br, goldCol);
                    rasterizer.drawCircleMidpoint(bx, by, br, orangeCol);
                }
            }
        }
    }

    // 3. Render Floating Platforms
    for (const auto& seg : m_segments) {
        for (const auto& plat : seg.platforms) {
            if (plat.x + plat.width < 0.0f || plat.x >= static_cast<float>(m_width)) continue;

            int px = static_cast<int>(plat.x);
            int py = static_cast<int>(plat.y);
            int pw = static_cast<int>(plat.width);
            int ph = static_cast<int>(plat.height);

            // Platform slab body
            rasterizer.fillRectScanline(px, py, pw, ph, platCol);

            // Top highlight edge
            rasterizer.drawLineBresenham(px, py, px + pw - 1, py, platTopCol);

            // Bottom shadow edge
            rasterizer.drawLineBresenham(px, py + ph - 1, px + pw - 1, py + ph - 1, platBotCol);

            // Bevel ends
            rasterizer.drawLineBresenham(px, py, px, py + ph - 1, platTopCol);
            rasterizer.drawLineBresenham(px + pw - 1, py, px + pw - 1, py + ph - 1, platBotCol);

            // Stone strata texture stripes
            rasterizer.drawDottedLineBresenham(px + 4, py + 4, px + pw - 4, py + 4, platTopCol, 4);
            rasterizer.drawDottedLineBresenham(px + 4, py + 8, px + pw - 4, py + 8, platBotCol, 6);

            // Underside hanging fossil teeth / small triangular stalactites
            int teethCount = pw / 28;
            for (int t = 0; t < teethCount; ++t) {
                int tx = px + 12 + t * 28;
                std::vector<QPoint> tooth = {
                    QPoint(tx, py + ph),
                    QPoint(tx + 4, py + ph + 5),
                    QPoint(tx + 8, py + ph)
                };
                rasterizer.fillPolygonScanline(tooth, platBotCol);
            }
        }
    }

    // 4. Render Spikes
    for (const auto& seg : m_segments) {
        for (const auto& spk : seg.spikes) {
            if (spk.x + spk.width * spk.count < 0.0f || spk.x >= static_cast<float>(m_width)) continue;

            for (int i = 0; i < spk.count; ++i) {
                int sx = static_cast<int>(spk.x + i * spk.width);
                int sy = static_cast<int>(spk.y);
                int sw = static_cast<int>(spk.width);
                int sh = static_cast<int>(spk.height);

                QPoint p1, p2, p3;
                if (spk.onCeiling) {
                    p1 = QPoint(sx, sy);
                    p2 = QPoint(sx + sw / 2, sy + sh);
                    p3 = QPoint(sx + sw, sy);
                } else {
                    p1 = QPoint(sx, sy);
                    p2 = QPoint(sx + sw / 2, sy - sh);
                    p3 = QPoint(sx + sw, sy);
                }

                rasterizer.fillPolygonScanline({p1, p2, p3}, spikeCol);
                rasterizer.drawLineBresenham(p1.x(), p1.y(), p2.x(), p2.y(), spikeLineCol);
                rasterizer.drawLineBresenham(p2.x(), p2.y(), p3.x(), p3.y(), spikeLineCol);
                rasterizer.drawLineBresenham(p3.x(), p3.y(), p1.x(), p1.y(), spikeLineCol);

                // Crystalline center ridge highlight
                rasterizer.drawLineBresenham(p2.x(), p2.y(), (p1.x() + p3.x()) / 2, (p1.y() + p3.y()) / 2, 0xFFFFFFFF);
            }
        }
    }

    // 5. Render Cave Ceiling and Stalactites
    for (const auto& seg : m_segments) {
        for (const auto& cave : seg.caves) {
            int cx0 = std::max(0, static_cast<int>(cave.startX));
            int cx1 = std::min(m_width - 1, static_cast<int>(cave.endX));
            if (cx0 >= cx1) continue;

            uint32_t caveRockCol = nightMode ? 0xFF14151E : 0xFF353440;
            uint32_t caveLipCol = nightMode ? 0xFF8A8E9E : 0xFF585764;
            if (cyberpunk) {
                caveRockCol = 0xFF180A28;
                caveLipCol = 0xFFBD00FF;
            }

            // Ceiling rock slab fill down to cave ceiling line
            for (int x = cx0; x <= cx1; ++x) {
                int cy = static_cast<int>(std::round(getCeilingHeightAt(static_cast<float>(x))));
                if (cy > 0) {
                    for (int y = 0; y < cy; ++y) {
                        rasterizer.setPixel(x, y, caveRockCol);
                    }
                    rasterizer.setPixel(x, cy, caveLipCol);
                }
            }

            // Stalactites hanging from ceiling
            for (const auto& stal : cave.stalactites) {
                if (stal.x + stal.width < 0.0f || stal.x >= static_cast<float>(m_width)) continue;

                int sx = static_cast<int>(stal.x);
                int sy = static_cast<int>(stal.y);
                int sw = static_cast<int>(stal.width);
                int sh = static_cast<int>(stal.height);

                QPoint p1(sx, sy);
                QPoint p2(sx + sw / 2, sy + sh);
                QPoint p3(sx + sw, sy);

                rasterizer.fillPolygonScanline({p1, p2, p3}, caveRockCol);
                rasterizer.drawLineBresenham(p1.x(), p1.y(), p2.x(), p2.y(), caveLipCol);
                rasterizer.drawLineBresenham(p2.x(), p2.y(), p3.x(), p3.y(), caveLipCol);

                // Water drop glint at stalactite tip
                rasterizer.setPixel(p2.x(), p2.y(), 0xFF80D8FF);
            }
        }
    }
}
