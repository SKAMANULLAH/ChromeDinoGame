#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <memory>
#include <random>
#include <cstdint>
#include <QPoint>
#include "Entity.h"
#include "../graphics/SoftwareRasterizer.h"
#include "ParticleSystem.h"

enum class TerrainType {
    Flat,
    Dunes,
    FloatingPlatforms,
    Spikes,
    LavaPool,
    Cave
};

enum class HazardType {
    None,
    Spike,
    Lava,
    Stalactite
};

struct FloatingPlatform {
    float x{0.0f};
    float y{0.0f};      // top landing surface Y
    float width{120.0f};
    float height{14.0f}; // slab thickness

    AABB boundingBox() const {
        return {
            static_cast<int>(x),
            static_cast<int>(y),
            static_cast<int>(width),
            static_cast<int>(height)
        };
    }
};

struct SpikeHazard {
    float x{0.0f};
    float y{0.0f};      // base line Y
    float width{16.0f}; // width per single spike horn
    float height{20.0f};
    int count{1};       // number of horn teeth (1 to 3)
    bool onCeiling{false};

    AABB boundingBox() const {
        int ry = onCeiling ? static_cast<int>(y) : static_cast<int>(y - height);
        return {
            static_cast<int>(x),
            ry,
            static_cast<int>(width * count),
            static_cast<int>(height)
        };
    }
};

struct LavaBubble {
    float x{0.0f};
    float y{0.0f};
    float r{2.0f};
    float maxR{3.5f};
    float vy{-22.0f};
    float life{0.8f};
    float maxLife{0.8f};
};

struct LavaPool {
    float x{0.0f};
    float width{110.0f};
    float surfaceY{238.0f};
    float depth{40.0f};
    float waveTimer{0.0f};
    float bubbleSpawnTimer{0.0f};
    std::vector<LavaBubble> bubbles;

    AABB boundingBox() const {
        return {
            static_cast<int>(x),
            static_cast<int>(surfaceY),
            static_cast<int>(width),
            static_cast<int>(depth)
        };
    }
};

struct Stalactite {
    float x{0.0f};
    float y{0.0f};      // base at ceiling Y
    float width{14.0f};
    float height{28.0f}; // length hanging downwards
    bool isHazard{true};

    AABB boundingBox() const {
        if (!isHazard) {
            return {0, 0, 0, 0};
        }
        int insetX = 2;
        return {
            static_cast<int>(x) + insetX,
            static_cast<int>(y),
            std::max(4, static_cast<int>(width) - insetX * 2),
            static_cast<int>(height)
        };
    }
};

struct RoostingBat {
    float x{0.0f};
    float y{0.0f};
    bool startled{false};
    float flapTimer{0.0f};
    float flyVx{0.0f};
    float flyVy{0.0f};
};

struct CaveCrystal {
    float x{0.0f};
    float y{0.0f};
    int width{7};
    int height{12};
    uint32_t color{0xFF00E5FF};
    bool onCeiling{false};
};

struct CaveZone {
    float startX{0.0f};
    float endX{0.0f};
    float ceilingY{220.0f};
    float normalFloorY{225.0f};
    float deepFloorY{380.0f};
    float entranceSlopeLen{380.0f};
    float exitSlopeLen{340.0f};
    bool hasLava{false};
    std::vector<Stalactite> stalactites;
    std::vector<RoostingBat> roostingBats;
    std::vector<CaveCrystal> crystals;
};

struct TerrainSegment {
    TerrainType type{TerrainType::Flat};
    float startX{0.0f};
    float endX{0.0f};
    float startGroundY{225.0f};
    float endGroundY{225.0f};

    // Dunes parameters
    float duneAmplitude{0.0f};
    int duneWaves{1};

    // Sub-elements attached to segment
    std::vector<FloatingPlatform> platforms;
    std::vector<SpikeHazard> spikes;
    std::vector<LavaPool> lavaPools;
    std::vector<CaveZone> caves;
};

/**
 * @file Terrain.h
 * @brief Infinite Procedural Diversified Terrain Engine for Chrome Dino.
 * 
 * Manages procedural terrain generation, surface height sampling, kinematics/physics,
 * platform collisions, and software rasterization for:
 * 1. Flats (standard desert ground baseline)
 * 2. Dunes (undulating rolling sand dunes with dynamic slope tilt)
 * 3. Floating Platforms (multi-tiered one-way jumpable ledges)
 * 4. Spikes (sharp mineral/bone hazards)
 * 5. Lava Pools (boiling magma chasms with fluid oscillation & rising bubbles)
 * 6. Caves (caverns with ceiling arches, hanging stalactites requiring ducking)
 */
class Terrain {
public:
    explicit Terrain(int width = 800, int height = 300, float defaultGroundY = 225.0f);

    void reset();
    void update(float dt, float speed, int score, ParticleSystem& particles);

    // Rasterization
    void renderBackgroundCave(SoftwareRasterizer& rasterizer, bool nightMode, float cameraY = 0.0f);
    void renderForeground(SoftwareRasterizer& rasterizer, bool nightMode, bool cyberpunk = false, float cameraY = 0.0f);

    // Kinematic / Physics Surface Queries
    float getGroundHeightAt(float screenX) const;
    float getSurfaceHeightAt(float screenX, float footY, float vy, bool& onPlatform, FloatingPlatform*& outPlatform);
    float getLandingSurfaceAt(float screenX, float footY, float vy, bool& onPlatform, FloatingPlatform*& outPlatform);
    float getSlopeAngleAt(float screenX) const;
    float getCeilingHeightAt(float screenX) const;
    bool isInCave(float screenX) const;
    bool isOverLava(float screenX) const;

    // Safety & Dodging Clearance Queries
    bool isNearLava(float screenX, float margin = 260.0f) const;
    bool isNearSpikes(float screenX, float margin = 260.0f) const;
    bool isNearStalactite(float screenX, float margin = 280.0f) const;
    bool isNearCavePortal(float screenX, float margin = 100.0f) const;
    bool isNearHazard(float screenX, float margin = 260.0f) const;
    bool hasSafeHeadroom(float startX, float endX, float minHeadroom = 125.0f) const;

    TerrainType currentBiomeAt(float screenX) const;
    const char* currentBiomeName(float screenX) const;

    // Hazard Collisions (Spikes, Lava, Stalactites)
    bool checkHazardCollision(const AABB& dinoBox, bool isDucking, HazardType& outHazard, float& hitX, float& hitY);

    float defaultGroundY() const { return m_defaultGroundY; }

private:
    int m_width;
    int m_height;
    float m_defaultGroundY{225.0f};
    std::mt19937 m_rng;

    std::vector<TerrainSegment> m_segments;
    TerrainType m_lastSpecialBiome{TerrainType::Flat};
    int m_biomeCooldown{1};

    void generateInitialSegments();
    void generateNextSegment(int score);

    TerrainSegment createFlatSegment(float startX, float length, float groundY);
    TerrainSegment createDuneSegment(float startX, float length, float groundY, float amplitude, int waves);
    TerrainSegment createPlatformSegment(float startX, float length, float groundY);
    TerrainSegment createSpikeSegment(float startX, float length, float groundY);
    TerrainSegment createLavaSegment(float startX, float length, float groundY);
    TerrainSegment createCaveSegment(float startX, float length, float groundY);
};

#endif // TERRAIN_H
