#include "Terrain.h"
#include "../graphics/SpriteData.h"
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
    if (length >= 440.0f) {
        SpikeHazard s2;
        s2.x = startX + 320.0f;
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
    cave.normalFloorY = groundY;
    cave.deepFloorY = groundY + 155.0f; // 380.0f: deep subterranean cavern floor below current ground!
    cave.ceilingY = 220.0f; // Underground cavern roof
    cave.entranceSlopeLen = 380.0f; // Smooth, gradual ramp descending into the earth
    cave.exitSlopeLen = 340.0f;     // Smooth, gradual ramp ascending back up to desert

    // ~45% of deep subterranean caves contain bubbling magma pools
    cave.hasLava = ((m_rng() % 100) < 45);

    float interiorStart = startX + cave.entranceSlopeLen;
    float interiorEnd = cave.endX - cave.exitSlopeLen;
    float interiorLen = interiorEnd - interiorStart;

    if (cave.hasLava && interiorLen >= 280.0f) {
        LavaPool pool;
        pool.x = interiorStart + 60.0f + static_cast<float>(m_rng() % 35);
        pool.width = 96.0f + static_cast<float>(m_rng() % 16); // 96-112px jumpable molten chasm
        pool.surfaceY = cave.deepFloorY + 2.0f; // 382.0f
        pool.depth = 80.0f;
        pool.waveTimer = 0.0f;
        pool.bubbleSpawnTimer = 0.0f;
        seg.lavaPools.push_back(pool);

        // Keep the air above the lava pool clear for jumping!
        // Place hazard stalactites on flat ground after the lava chasm
        float runwayAfterLava = interiorEnd - (pool.x + pool.width);
        if (runwayAfterLava >= 260.0f) {
            Stalactite stDuck;
            stDuck.x = pool.x + pool.width + 140.0f;
            stDuck.y = cave.ceilingY;
            stDuck.width = 18.0f;
            stDuck.height = (cave.deepFloorY - 38.0f) - cave.ceilingY; // tip at 342: duck required
            stDuck.isHazard = true;
            cave.stalactites.push_back(stDuck);
        }
        if (runwayAfterLava >= 420.0f) {
            Stalactite stHigh;
            stHigh.x = interiorEnd - 100.0f;
            stHigh.y = cave.ceilingY;
            stHigh.width = 18.0f;
            stHigh.height = 54.0f; // tip at 274: standing Dino (337) runs under, jumping Dino triggers collision!
            stHigh.isHazard = true;
            cave.stalactites.push_back(stHigh);
        }
    } else {
        // Cave without lava: low hazard stalactite requiring ducking
        Stalactite st1;
        st1.x = interiorStart + 110.0f;
        st1.y = cave.ceilingY;
        st1.width = 18.0f;
        st1.height = (cave.deepFloorY - 38.0f) - cave.ceilingY; // tip at 342: duck required
        st1.isHazard = true;
        cave.stalactites.push_back(st1);

        // High hazard stalactite: safe to run under, but triggers collision if jumped into!
        if (interiorLen >= 400.0f) {
            Stalactite st2;
            st2.x = interiorEnd - 120.0f;
            st2.y = cave.ceilingY;
            st2.width = 18.0f;
            st2.height = 54.0f; // tip at 274: duck/run clears, jumping triggers!
            st2.isHazard = true;
            cave.stalactites.push_back(st2);
        }
    }

    // Roosting Bats hanging from the cave ceiling
    int roostCount = 2 + (m_rng() % 3); // 2 to 4 bats
    float batSpacing = interiorLen / static_cast<float>(roostCount + 1);
    for (int i = 0; i < roostCount; ++i) {
        RoostingBat bat;
        bat.x = interiorStart + (i + 1) * batSpacing + static_cast<float>(m_rng() % 30) - 15.0f;
        bat.y = cave.ceilingY;
        cave.roostingBats.push_back(bat);
    }

    // Glowing subterranean mineral crystals
    int crystalCount = 3 + (m_rng() % 4);
    uint32_t crystalPalette[] = { 0xFF00E5FF, 0xFF00FF88, 0xFFBD00FF, 0xFFFF007F };
    for (int i = 0; i < crystalCount; ++i) {
        CaveCrystal cr;
        float offsetSpan = std::max(10.0f, interiorLen - 20.0f);
        cr.x = interiorStart + static_cast<float>(m_rng() % static_cast<int>(offsetSpan));
        cr.onCeiling = ((m_rng() % 100) < 40);
        cr.y = cr.onCeiling ? cave.ceilingY : cave.deepFloorY;
        cr.width = 6 + (m_rng() % 4);
        cr.height = 10 + (m_rng() % 6);
        cr.color = crystalPalette[m_rng() % 4];
        cave.crystals.push_back(cr);
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
            float len = 1250.0f + static_cast<float>(m_rng() % 300);
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
            for (auto& bat : cave.roostingBats) {
                bat.x -= dx;
                if (!bat.startled && bat.x < 240.0f) {
                    bat.startled = true;
                    bat.flyVx = -speed * 0.45f - 40.0f;
                    bat.flyVy = -22.0f - static_cast<float>(m_rng() % 20);
                }
                if (bat.startled) {
                    bat.x += bat.flyVx * dt;
                    bat.y += bat.flyVy * dt;
                    bat.flapTimer += dt;
                }
            }
            for (auto& cr : cave.crystals) {
                cr.x -= dx;
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

            if (seg.type == TerrainType::Cave) {
                // Drop into subterranean lava chasm if over pool inside cave
                for (const auto& pool : seg.lavaPools) {
                    if (screenX >= pool.x && screenX <= pool.x + pool.width) {
                        return pool.surfaceY + 35.0f;
                    }
                }
                for (const auto& cave : seg.caves) {
                    if (screenX >= cave.startX && screenX <= cave.endX) {
                        // Smooth descent slope at entrance
                        if (screenX < cave.startX + cave.entranceSlopeLen) {
                            float u = (screenX - cave.startX) / cave.entranceSlopeLen;
                            float t = 0.5f * (1.0f - std::cos(u * 3.14159265f));
                            return cave.normalFloorY + (cave.deepFloorY - cave.normalFloorY) * t;
                        }
                        // Smooth ascent slope at exit
                        if (screenX > cave.endX - cave.exitSlopeLen) {
                            float u = (cave.endX - screenX) / cave.exitSlopeLen;
                            float t = 0.5f * (1.0f - std::cos(u * 3.14159265f));
                            return cave.normalFloorY + (cave.deepFloorY - cave.normalFloorY) * t;
                        }
                        // Deep subterranean cavern floor
                        return cave.deepFloorY;
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
                float entranceArchLen = cave.entranceSlopeLen;
                float exitArchLen = cave.exitSlopeLen;
                float portalMouthY = 80.0f;
                // Arch down smoothly at entrance
                if (screenX < cave.startX + entranceArchLen) {
                    float u = (screenX - cave.startX) / entranceArchLen;
                    float t = 0.5f * (1.0f - std::cos(u * 3.14159265f));
                    return portalMouthY + (cave.ceilingY - portalMouthY) * t;
                }
                // Arch up smoothly at exit
                if (screenX > cave.endX - exitArchLen) {
                    float u = (cave.endX - screenX) / exitArchLen;
                    float t = 0.5f * (1.0f - std::cos(u * 3.14159265f));
                    return portalMouthY + (cave.ceilingY - portalMouthY) * t;
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
            if (screenX >= cave.startX + cave.entranceSlopeLen &&
                screenX <= cave.endX - cave.exitSlopeLen) {
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

bool Terrain::isNearLava(float screenX, float margin) const {
    for (const auto& seg : m_segments) {
        for (const auto& pool : seg.lavaPools) {
            if (screenX >= pool.x - margin && screenX <= pool.x + pool.width + margin) {
                return true;
            }
        }
    }
    return false;
}

bool Terrain::isNearSpikes(float screenX, float margin) const {
    for (const auto& seg : m_segments) {
        for (const auto& spk : seg.spikes) {
            float spkW = spk.width * static_cast<float>(spk.count);
            if (screenX >= spk.x - margin && screenX <= spk.x + spkW + margin) {
                return true;
            }
        }
    }
    return false;
}

bool Terrain::isNearStalactite(float screenX, float margin) const {
    for (const auto& seg : m_segments) {
        for (const auto& cave : seg.caves) {
            for (const auto& stal : cave.stalactites) {
                if (!stal.isHazard) continue; // Only duck-hazard stalactites require clearance
                // If stalactite is behind screenX: player needs runway to stand/recover from ducking
                if (stal.x <= screenX && (screenX - (stal.x + stal.width)) < 220.0f) {
                    return true;
                }
                // If stalactite is ahead of screenX: player jumping would jump into or land near stalactite
                if (stal.x > screenX && (stal.x - screenX) < margin) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Terrain::isNearCavePortal(float screenX, float margin) const {
    for (const auto& seg : m_segments) {
        for (const auto& cave : seg.caves) {
            if ((screenX >= cave.startX - margin && screenX <= cave.startX + cave.entranceSlopeLen + margin) ||
                (screenX >= cave.endX - cave.exitSlopeLen - margin && screenX <= cave.endX + margin)) {
                return true;
            }
        }
    }
    return false;
}

bool Terrain::isNearHazard(float screenX, float margin) const {
    return isNearLava(screenX, margin) || isNearSpikes(screenX, margin) || isNearStalactite(screenX, margin);
}

bool Terrain::hasSafeHeadroom(float startX, float endX, float minHeadroom) const {
    for (float x = startX; x <= endX; x += 15.0f) {
        float gY = getGroundHeightAt(x);

        // 1. Cave ceiling check
        float cY = getCeilingHeightAt(x);
        if (cY > 0.0f) {
            if ((gY - cY) < minHeadroom) {
                return false;
            }
        }

        // 2. Stalactites check
        for (const auto& seg : m_segments) {
            for (const auto& cave : seg.caves) {
                for (const auto& stal : cave.stalactites) {
                    if (!stal.isHazard) continue;
                    if (x >= stal.x - 10.0f && x <= stal.x + stal.width + 10.0f) {
                        float tipY = stal.y + stal.height;
                        if ((gY - tipY) < minHeadroom) {
                            return false;
                        }
                    }
                }
            }
            // 3. Floating platforms above ground check
            for (const auto& plat : seg.platforms) {
                if (x >= plat.x - 10.0f && x <= plat.x + plat.width + 10.0f) {
                    float platBot = plat.y + plat.height;
                    if (platBot < gY && (gY - platBot) < minHeadroom) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
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
        case TerrainType::Cave: {
            for (const auto& seg : m_segments) {
                if (screenX >= seg.startX && screenX <= seg.endX) {
                    for (const auto& cave : seg.caves) {
                        if (cave.hasLava) return "VOLCANIC CAVERN";
                        return "DEEP BAT GROTTO";
                    }
                }
            }
            return "DEEP CAVERN";
        }
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

void Terrain::renderBackgroundCave(SoftwareRasterizer& rasterizer, bool nightMode, float cameraY) {
    // Render cavern back wall inside cave sections
    for (const auto& seg : m_segments) {
        for (const auto& cave : seg.caves) {
            int cx0 = std::max(0, static_cast<int>(cave.startX));
            int cx1 = std::min(m_width, static_cast<int>(cave.endX));
            if (cx0 >= cx1) continue;

            // Cave back wall is only slightly different from the ground/ceiling color
            uint32_t caveBgCol = nightMode ? 0xFF242630 : 0xFFD8D8DF;
            uint32_t strataCol = nightMode ? 0xFF323644 : 0xFFC6C6CF;
            uint32_t magmaGlowCol = nightMode ? 0xFF3D1610 : 0xFFFFD2BE;

            for (int x = cx0; x < cx1; ++x) {
                float fx = static_cast<float>(x);
                int cy = static_cast<int>(std::round(getCeilingHeightAt(fx) - cameraY));
                int gy = static_cast<int>(std::round(getGroundHeightAt(fx) - cameraY));

                int topY = std::max(0, cy);
                int botY = std::min(m_height, gy);

                for (int y = topY; y < botY; ++y) {
                    uint32_t col = caveBgCol;
                    // If cave has lava, add a warm ambient glow rising from the bottom
                    if (cave.hasLava && y > botY - 70) {
                        if ((x + y) % 3 == 0) {
                            col = magmaGlowCol;
                        }
                    }
                    rasterizer.setPixel(x, y, col);
                }

                // Cavern rock strata lines
                int s1 = static_cast<int>(std::round(260.0f - cameraY));
                int s2 = static_cast<int>(std::round(315.0f - cameraY));
                int s3 = static_cast<int>(std::round(365.0f - cameraY));
                if (topY <= s1 && botY >= s1 && s1 >= 0 && s1 < m_height && x % 4 != 0) {
                    rasterizer.setPixel(x, s1, strataCol);
                }
                if (topY <= s2 && botY >= s2 && s2 >= 0 && s2 < m_height && x % 6 != 0) {
                    rasterizer.setPixel(x, s2, strataCol);
                }
                if (topY <= s3 && botY >= s3 && s3 >= 0 && s3 < m_height && x % 5 != 0) {
                    rasterizer.setPixel(x, s3, strataCol);
                }
            }

            // Render roosting bats hanging from the ceiling
            for (const auto& bat : cave.roostingBats) {
                if (bat.x < -30.0f || bat.x >= static_cast<float>(m_width) + 20.0f) continue;
                int bx = static_cast<int>(bat.x);
                int by = static_cast<int>(bat.y - cameraY);

                if (bat.startled) {
                    const Sprite& sp = ((static_cast<int>(bat.flapTimer * 12.0f) % 2) == 0)
                                           ? SpriteData::batWingUp()
                                           : SpriteData::batWingDown();
                    Sprite finalSp = nightMode ? sp.invertedColors() : sp;
                    rasterizer.blitSprite(finalSp, bx, by);
                } else {
                    const Sprite& roostSp = SpriteData::batRoost();
                    Sprite finalSp = nightMode ? roostSp.invertedColors() : roostSp;
                    rasterizer.blitSprite(finalSp, bx, by);
                    // Glowing ruby red bat eyes
                    uint32_t eyeCol = 0xFFFF1744;
                    rasterizer.setPixel(bx + 4, by + 9, eyeCol);
                    rasterizer.setPixel(bx + 7, by + 9, eyeCol);
                }
            }
        }
    }
}

void Terrain::renderForeground(SoftwareRasterizer& rasterizer, bool nightMode, bool cyberpunk, float cameraY) {
    uint32_t lineCol = nightMode ? 0xFFACACAC : 0xFF535353;
    uint32_t groundBodyCol = nightMode ? 0xFF15161C : 0xFFEFEFEF;
    uint32_t platCol = nightMode ? 0xFF2C2E3B : 0xFF656570;
    uint32_t platTopCol = nightMode ? 0xFF84889C : 0xFFA5A5B5;
    uint32_t platBotCol = nightMode ? 0xFF181920 : 0xFF3C3C44;
    uint32_t spikeCol = nightMode ? 0xFF585B6C : 0xFF25252B;
    uint32_t spikeLineCol = nightMode ? 0xFFD8DCEE : 0xFF111111;

    if (cyberpunk) {
        lineCol = 0xFFFF007F;       // Neon Pink
        groundBodyCol = 0xFF120822; // Deep Indigo
        platCol = 0xFF1F0D3D;
        platTopCol = 0xFF00E5FF;    // Cyan Highlight
        platBotCol = 0xFF080214;
        spikeCol = 0xFFBD00FF;      // Laser Violet
        spikeLineCol = 0xFF00FFFF;  // Neon Cyan Edge
    }

    // 1. Render Ground & Dunes (using cameraY offset)
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

        int gy = static_cast<int>(std::round(getGroundHeightAt(fx) - cameraY));
        if (gy >= m_height) continue;

        int drawGy = std::max(0, gy);

        // Consistent ground across desert surface and subterranean cave floor
        uint32_t curLineCol = lineCol;
        uint32_t curBodyCol = groundBodyCol;

        // Ground surface contour pixel
        if (gy >= 0 && gy < m_height) {
            rasterizer.setPixel(x, gy, curLineCol);
        }

        // Solid ground body fill below surface
        for (int y = drawGy + 1; y < m_height; ++y) {
            rasterizer.setPixel(x, y, curBodyCol);
        }

        // Procedural rock/sediment specks
        if ((x * 37 + drawGy * 19) % 47 == 0 && drawGy + 5 < m_height) {
            rasterizer.setPixel(x, drawGy + 4, curLineCol);
        }
        if ((x * 13 + drawGy * 7) % 61 == 0 && drawGy + 9 < m_height) {
            rasterizer.setPixel(x, drawGy + 8, curLineCol);
            rasterizer.setPixel(x + 1, drawGy + 8, curLineCol);
        }
    }

    // 2. Render Lava Pools
    for (const auto& seg : m_segments) {
        for (const auto& pool : seg.lavaPools) {
            if (pool.x + pool.width < 0.0f || pool.x >= static_cast<float>(m_width)) continue;

            int px = static_cast<int>(pool.x);
            int pw = static_cast<int>(pool.width);
            int sy = static_cast<int>(pool.surfaceY - cameraY);
            int pyBottom = std::min(m_height - 1, sy + static_cast<int>(pool.depth));

            // Deep magma red basin
            uint32_t magmaCol = cyberpunk ? 0xFF800030 : 0xFFB31200;
            if (sy + 3 < pyBottom) {
                rasterizer.fillRectScanline(px, sy + 3, pw, pyBottom - (sy + 3), magmaCol);
            }

            // Sub-surface orange band
            uint32_t orangeCol = cyberpunk ? 0xFFFF0066 : 0xFFFF4500;
            if (sy + 1 >= 0 && sy + 1 < m_height) {
                rasterizer.fillRectScanline(px, sy + 1, pw, 3, orangeCol);
            }

            // Animated undulating molten surface line
            uint32_t goldCol = cyberpunk ? 0xFF00FFFF : 0xFFFFEE44;
            for (int x = px; x < px + pw; ++x) {
                if (x < 0 || x >= m_width) continue;
                float fx = static_cast<float>(x);
                float wy = pool.surfaceY - cameraY + 2.2f * std::sin(pool.waveTimer * 5.0f + fx * 0.09f)
                                                    + 1.2f * std::cos(pool.waveTimer * 3.2f + fx * 0.15f);
                int iwy = static_cast<int>(wy);
                if (iwy >= 0 && iwy < m_height) {
                    rasterizer.setPixel(x, iwy, goldCol);
                }
                if (iwy + 1 >= 0 && iwy + 1 < m_height) {
                    rasterizer.setPixel(x, iwy + 1, orangeCol);
                }
            }

            // Rocky bank borders (left and right edges of chasm)
            int bankYLeft = static_cast<int>(std::round(getGroundHeightAt(pool.x - 1.0f) - cameraY));
            int bankYRight = static_cast<int>(std::round(getGroundHeightAt(pool.x + pool.width + 1.0f) - cameraY));
            rasterizer.drawLineBresenham(px, bankYLeft, px, pyBottom, lineCol);
            rasterizer.drawLineBresenham(px + pw, bankYRight, px + pw, pyBottom, lineCol);

            // Rising animated bubbles
            for (const auto& b : pool.bubbles) {
                int bx = static_cast<int>(b.x);
                int by = static_cast<int>(b.y - cameraY);
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
            int py = static_cast<int>(plat.y - cameraY);
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

            uint32_t curSpikeFill = spikeCol;
            uint32_t curSpikeEdge = spikeLineCol;
            if (cyberpunk) {
                curSpikeFill = 0xFFBD00FF;
                curSpikeEdge = 0xFF00FFFF;
            }

            for (int i = 0; i < spk.count; ++i) {
                int sx = static_cast<int>(spk.x + i * spk.width);
                int sy = static_cast<int>(spk.y - cameraY);
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

                rasterizer.fillPolygonScanline({p1, p2, p3}, curSpikeFill);
                rasterizer.drawLineBresenham(p1.x(), p1.y(), p2.x(), p2.y(), curSpikeEdge);
                rasterizer.drawLineBresenham(p2.x(), p2.y(), p3.x(), p3.y(), curSpikeEdge);
                rasterizer.drawLineBresenham(p3.x(), p3.y(), p1.x(), p1.y(), curSpikeEdge);

                // Crystalline center ridge
                rasterizer.drawLineBresenham(p2.x(), p2.y(), (p1.x() + p3.x()) / 2, (p1.y() + p3.y()) / 2, nightMode ? 0xFFFFFFFF : 0xFF535353);
            }
        }
    }

    // 5. Render Cave Ceiling and Stalactites
    for (const auto& seg : m_segments) {
        for (const auto& cave : seg.caves) {
            int cx0 = std::max(0, static_cast<int>(cave.startX));
            int cx1 = std::min(m_width - 1, static_cast<int>(cave.endX));
            if (cx0 >= cx1) continue;

            // Cave ceiling matches the exact ground color
            uint32_t caveRockCol = groundBodyCol;
            uint32_t caveLipCol = lineCol;

            // Ceiling rock slab fill down to cave ceiling line
            for (int x = cx0; x <= cx1; ++x) {
                float ceilWorldY = getCeilingHeightAt(static_cast<float>(x));
                if (ceilWorldY > 0.0f) {
                    int cy = static_cast<int>(std::round(ceilWorldY - cameraY));
                    int fillCy = std::clamp(cy, 0, m_height);
                    for (int y = 0; y < fillCy; ++y) {
                        rasterizer.setPixel(x, y, caveRockCol);
                    }
                    if (cy >= 0 && cy < m_height) {
                        rasterizer.setPixel(x, cy, caveLipCol);
                    }
                }
            }

            // Stalactites hanging from ceiling
            for (const auto& stal : cave.stalactites) {
                if (stal.x + stal.width < 0.0f || stal.x >= static_cast<float>(m_width)) continue;

                int sx = static_cast<int>(stal.x);
                int sy = static_cast<int>(stal.y - cameraY);
                int sw = static_cast<int>(stal.width);
                int sh = static_cast<int>(stal.height);

                QPoint p1(sx, sy);
                QPoint p2(sx + sw / 2, sy + sh);
                QPoint p3(sx + sw, sy);

                uint32_t stalFillCol, stalEdgeCol, stalRidgeCol;
                if (cyberpunk) {
                    stalFillCol = 0xFFBD00FF;
                    stalEdgeCol = 0xFF00FFFF;
                    stalRidgeCol = 0xFFFFFFFF;
                } else if (nightMode) {
                    stalFillCol = 0xFF585B6C;
                    stalEdgeCol = 0xFFD8DCEE;
                    stalRidgeCol = 0xFFFFFFFF;
                } else {
                    // Day mode: black spikes (like ground spikes and obstacles)
                    stalFillCol = 0xFF25252B;
                    stalEdgeCol = 0xFF111111;
                    stalRidgeCol = 0xFF535353;
                }

                rasterizer.fillPolygonScanline({p1, p2, p3}, stalFillCol);
                rasterizer.drawLineBresenham(p1.x(), p1.y(), p2.x(), p2.y(), stalEdgeCol);
                rasterizer.drawLineBresenham(p2.x(), p2.y(), p3.x(), p3.y(), stalEdgeCol);
                rasterizer.drawLineBresenham((p1.x() + p3.x()) / 2, p1.y(), p2.x(), p2.y(), stalRidgeCol);

                // Water drop glint / tip pixel
                int tipY = sy + sh;
                if (tipY >= 0 && tipY < m_height) {
                    rasterizer.setPixel(p2.x(), tipY, stalEdgeCol);
                }
            }

            // Glowing subterranean mineral crystals
            for (const auto& cr : cave.crystals) {
                if (cr.x + cr.width < 0.0f || cr.x >= static_cast<float>(m_width)) continue;
                int cx = static_cast<int>(cr.x);
                int cy = static_cast<int>(cr.y - cameraY);
                int cw = cr.width;
                int ch = cr.height;

                uint32_t crCol = cyberpunk ? 0xFF00FFFF : cr.color;
                QPoint p1, p2, p3;
                if (cr.onCeiling) {
                    p1 = QPoint(cx, cy);
                    p2 = QPoint(cx + cw / 2, cy + ch);
                    p3 = QPoint(cx + cw, cy);
                } else {
                    p1 = QPoint(cx, cy);
                    p2 = QPoint(cx + cw / 2, cy - ch);
                    p3 = QPoint(cx + cw, cy);
                }

                rasterizer.fillPolygonScanline({p1, p2, p3}, crCol);
                rasterizer.drawLineBresenham(p1.x(), p1.y(), p2.x(), p2.y(), 0xFFFFFFFF);
                rasterizer.drawLineBresenham(p2.x(), p2.y(), p3.x(), p3.y(), crCol);
            }
        }
    }
}
