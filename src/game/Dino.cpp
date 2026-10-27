#include "Dino.h"
#include "Terrain.h"
#include "../graphics/SpriteData.h"

Dino::Dino(int groundY)
    : m_groundY(groundY), m_currentGroundY(static_cast<float>(groundY))
{
    reset();
}

void Dino::reset() {
    m_state = DinoState::Idle;
    m_currentGroundY = static_cast<float>(m_groundY);
    m_isOnPlatform = false;
    m_x = 50;
    m_y = static_cast<float>(m_currentGroundY - SpriteData::dinoIdle().height());
    m_vy = 0.0f;
    m_tiltAngle = 0.0f;
    m_isDuckingInput = false;
    m_isHoldingJump = false;
    m_hasShield = false;
    m_hasDoubleJump = false;
    m_didDoubleJump = false;
    m_doubleJumpCharges = 0;
    m_jumpBufferTimer = 0.0f;
    m_coyoteTimer = 0.0f;
    m_blinkTimer = 0.0f;
    m_isBlinking = false;
    m_animTimer = 0.0f;
    m_animFrame = 0;
    m_dustTimer = 0.0f;
}

bool Dino::startJump() {
    if (m_state == DinoState::Running || m_state == DinoState::Idle || m_state == DinoState::Ducking || m_coyoteTimer > 0.0f) {
        m_state = DinoState::Jumping;
        m_vy = m_jumpVelocity;
        m_isHoldingJump = true;
        m_jumpBufferTimer = 0.0f;
        m_coyoteTimer = 0.0f;
        m_didDoubleJump = false;
        m_isOnPlatform = false;
        return true;
    } else if (m_state == DinoState::Jumping) {
        // Double Jump power-up execution (consumes 1 charge)
        if (m_hasDoubleJump && !m_didDoubleJump && m_doubleJumpCharges > 0) {
            m_vy = m_jumpVelocity * 0.88f;
            m_didDoubleJump = true;
            m_doubleJumpCharges--;
            if (m_doubleJumpCharges <= 0) {
                m_hasDoubleJump = false;
            }
            return true;
        }
        // Buffer jump input if pressed close to landing
        m_jumpBufferTimer = 0.14f;
    }
    return false;
}

void Dino::endJump() {
    m_isHoldingJump = false;
    // Variable jump: cutting upward velocity gives short hop
    if (m_vy < -200.0f) {
        m_vy = -200.0f;
    }
}

void Dino::setDucking(bool ducking) {
    m_isDuckingInput = ducking;
    if (m_state == DinoState::Running && ducking) {
        m_state = DinoState::Ducking;
        m_y = m_currentGroundY - static_cast<float>(SpriteData::dinoDuck1().height());
    } else if (m_state == DinoState::Ducking && !ducking) {
        m_state = DinoState::Running;
        m_y = m_currentGroundY - static_cast<float>(SpriteData::dinoIdle().height());
    }
}

void Dino::kill() {
    m_state = DinoState::Dead;
    m_vy = 0.0f;
    m_y = m_currentGroundY - static_cast<float>(SpriteData::dinoDead().height());
}

void Dino::boostUpward(float vy) {
    m_state = DinoState::Jumping;
    m_vy = vy;
    m_didDoubleJump = false;
}

void Dino::update(float dt, ParticleSystem& particles, Terrain* terrain, float /*gameSpeed*/) {
    if (m_state == DinoState::Dead) return;

    // Eye blinking timer
    m_blinkTimer += dt;
    if (m_blinkTimer >= 3.6f) {
        m_isBlinking = true;
        if (m_blinkTimer >= 3.75f) {
            m_isBlinking = false;
            m_blinkTimer = 0.0f;
        }
    }

    if (m_jumpBufferTimer > 0.0f) {
        m_jumpBufferTimer -= dt;
    }

    float footX = (m_state == DinoState::Ducking) ? (m_x + 24.0f) : (m_x + 16.0f);
    int currentH = currentSprite().height();
    float footY = m_y + static_cast<float>(currentH);
    float fallbackGroundY = static_cast<float>(m_groundY);

    if (m_state == DinoState::Idle) {
        m_currentGroundY = terrain ? terrain->getGroundHeightAt(footX) : fallbackGroundY;
        m_y = m_currentGroundY - static_cast<float>(SpriteData::dinoIdle().height());
        m_vy = 0.0f;
        m_tiltAngle = 0.0f;
        return;
    }

    // Running animation & alternating foot dust
    if (m_state == DinoState::Running || m_state == DinoState::Ducking) {
        m_coyoteTimer = 0.08f; // Grounded window
        m_animTimer += dt;
        if (m_animTimer >= 0.10f) {
            m_animTimer = 0.0f;
            m_animFrame = (m_animFrame + 1) % 2;
        }

        // Surface tracking (Dunes, Flats, Platforms)
        if (terrain) {
            bool onPlat = false;
            FloatingPlatform* plat = nullptr;
            float targetGroundY = terrain->getSurfaceHeightAt(footX, footY, m_vy, onPlat, plat);

            // Did the ground drop away (walked off ledge or floating platform edge)?
            bool inCaveSlope = terrain && (terrain->currentBiomeAt(footX) == TerrainType::Cave);
            float dropTolerance = inCaveSlope ? 35.0f : 14.0f;
            if (targetGroundY - footY > dropTolerance) {
                m_state = DinoState::Jumping;
                m_vy = 0.0f;
                m_coyoteTimer = 0.08f;
                m_isOnPlatform = false;
            } else {
                m_currentGroundY = targetGroundY;
                m_y = m_currentGroundY - static_cast<float>(currentH);
                m_isOnPlatform = onPlat;

                // Dynamic slope pitch matching (Dino Run DX feel!)
                if (!onPlat) {
                    float slopeDeg = terrain->getSlopeAngleAt(footX);
                    slopeDeg = std::clamp(slopeDeg, -28.0f, 28.0f);
                    m_tiltAngle = m_tiltAngle * 0.70f + slopeDeg * 0.30f;
                } else {
                    m_tiltAngle = m_tiltAngle * 0.80f;
                }
            }
        } else {
            m_currentGroundY = fallbackGroundY;
            m_y = m_currentGroundY - static_cast<float>(currentH);
            m_tiltAngle = 0.0f;
        }

        m_dustTimer += dt;
        if (m_dustTimer >= 0.16f) {
            m_dustTimer = 0.0f;
            float puffX = (m_animFrame == 0) ? (m_x + 8.0f) : (m_x + 18.0f);
            particles.emitDustPuff(puffX, m_currentGroundY - 1.0f, 2);
        }
    } else {
        if (m_coyoteTimer > 0.0f) {
            m_coyoteTimer -= dt;
        }
    }

    // Airborne physics (Jumping / Falling)
    if (m_state == DinoState::Jumping) {
        float effectiveGravity = (m_isDuckingInput && m_vy > 0.0f) ? m_fastFallGravity : m_gravity;
        m_vy += effectiveGravity * dt;
        m_y += m_vy * dt;
        footY = m_y + static_cast<float>(currentH);

        // Ceiling collision in caves (bonking head on cave ceiling)
        if (terrain) {
            float ceilY = terrain->getCeilingHeightAt(footX);
            if (ceilY > 0.0f && m_y < ceilY) {
                m_y = ceilY;
                if (m_vy < 0.0f) {
                    m_vy = 0.0f;
                    particles.emitDustPuff(footX, ceilY + 1.0f, 3, 0xFF888888);
                }
            }
        }

        // Landing surface query
        bool onPlat = false;
        FloatingPlatform* plat = nullptr;
        float landingY = terrain ? terrain->getLandingSurfaceAt(footX, footY, m_vy, onPlat, plat)
                                 : fallbackGroundY;
        m_currentGroundY = landingY;

        float groundStandY = landingY - static_cast<float>(SpriteData::dinoIdle().height());
        float groundDuckY = landingY - static_cast<float>(SpriteData::dinoDuck1().height());

        // Landed on surface
        if (m_vy >= 0.0f && m_y >= (m_isDuckingInput ? groundDuckY : groundStandY)) {
            m_vy = 0.0f;
            m_tiltAngle = 0.0f;
            m_isOnPlatform = onPlat;
            particles.emitDustPuff(m_x + 12.0f, landingY - 1.0f, 4);

            if (m_isDuckingInput) {
                m_state = DinoState::Ducking;
                m_y = groundDuckY;
            } else {
                m_state = DinoState::Running;
                m_y = groundStandY;
            }

            // Execute buffered jump if queued!
            if (m_jumpBufferTimer > 0.0f) {
                m_jumpBufferTimer = 0.0f;
                startJump();
            }
        } else {
            // Dynamic tilt based on vertical velocity
            m_tiltAngle = std::clamp(m_vy * 0.035f, -14.0f, 20.0f);
        }
    }
}

const Sprite& Dino::currentSprite() const {
    switch (m_state) {
        case DinoState::Idle:
            return m_isBlinking ? SpriteData::dinoBlink() : SpriteData::dinoIdle();
        case DinoState::Running:
            if (m_isBlinking) return SpriteData::dinoBlink();
            return (m_animFrame == 0) ? SpriteData::dinoRun1() : SpriteData::dinoRun2();
        case DinoState::Ducking:
            return (m_animFrame == 0) ? SpriteData::dinoDuck1() : SpriteData::dinoDuck2();
        case DinoState::Jumping:
            return SpriteData::dinoIdle();
        case DinoState::Dead:
            return SpriteData::dinoDead();
    }
    return SpriteData::dinoIdle();
}

AABB Dino::boundingBox() const {
    const Sprite& sp = currentSprite();
    int insetX = 4;
    int insetY = 3;
    return {
        m_x + insetX,
        static_cast<int>(m_y) + insetY,
        sp.width() - (insetX * 2),
        sp.height() - (insetY * 2)
    };
}

std::vector<AABB> Dino::subHitboxes() const {
    std::vector<AABB> boxes;
    int curY = static_cast<int>(m_y);

    if (m_state == DinoState::Ducking) {
        // Low elongated hitbox
        boxes.push_back({m_x + 4, curY + 6, 50, 18});
    } else {
        // 3 sub-hitboxes: Head, Body, Feet
        boxes.push_back({m_x + 22, curY + 2, 16, 16});  // Head
        boxes.push_back({m_x + 8, curY + 16, 22, 18});  // Body
        boxes.push_back({m_x + 10, curY + 34, 18, 8});  // Feet
    }
    return boxes;
}

void Dino::render(SoftwareRasterizer& rasterizer, bool nightMode, int score, float cameraY) {
    int renderY = static_cast<int>(m_y - cameraY);

    // 1. Dynamic Drop Shadow (Midpoint Ellipse with distance scaling)
    if (m_state != DinoState::Dead) {
        float standY = m_currentGroundY - static_cast<float>(SpriteData::dinoIdle().height());
        float jumpHeight = std::max(0.0f, standY - m_y);

        int shadowRx = (m_state == DinoState::Ducking) ? 24 : std::max(6, 18 - static_cast<int>(jumpHeight * 0.12f));
        int shadowRy = std::max(2, 4 - static_cast<int>(jumpHeight * 0.025f));
        float alphaFactor = std::clamp(0.40f - jumpHeight * 0.0025f, 0.10f, 0.40f);

        uint8_t shadowAlpha = static_cast<uint8_t>(alphaFactor * 255.0f);
        uint32_t shadowCol = Framebuffer::packARGB(shadowAlpha, 30, 30, 35);
        int shadowCenterX = (m_state == DinoState::Ducking) ? (m_x + 28) : (m_x + 20);

        rasterizer.fillEllipseMidpoint(shadowCenterX, static_cast<int>(m_currentGroundY - cameraY) + 1, shadowRx, shadowRy, shadowCol);
    }

    // 2. Base Dino Sprite Blit (with 2D Affine Rotation during jumps!)
    const Sprite& baseSprite = currentSprite();

    if (score >= 2000) {
        // Golden Aura at 2000+ points!
        Sprite gold = baseSprite;
        for (int gy = 0; gy < gold.height(); ++gy) {
            for (int gx = 0; gx < gold.width(); ++gx) {
                if (gold.isSolid(gx, gy)) {
                    gold.setPixel(gx, gy, 0xFFFFD700); // Shimmering Gold
                }
            }
        }
        if (std::abs(m_tiltAngle) > 0.5f) {
            rasterizer.blitSpriteRotated(gold, m_x + gold.width() / 2, renderY + gold.height() / 2, m_tiltAngle);
        } else {
            rasterizer.blitSprite(gold, m_x, renderY);
        }
    } else if (nightMode) {
        Sprite inv = baseSprite.invertedColors();
        if (std::abs(m_tiltAngle) > 0.5f) {
            rasterizer.blitSpriteRotated(inv, m_x + inv.width() / 2, renderY + inv.height() / 2, m_tiltAngle);
        } else {
            rasterizer.blitSprite(inv, m_x, renderY);
        }
    } else {
        if (std::abs(m_tiltAngle) > 0.5f) {
            rasterizer.blitSpriteRotated(baseSprite, m_x + baseSprite.width() / 2, renderY + baseSprite.height() / 2, m_tiltAngle);
        } else {
            rasterizer.blitSprite(baseSprite, m_x, renderY);
        }
    }

    // 3. Power-Up: Bone Helmet (1-Hit Shield Protection)
    if (m_hasShield && m_state != DinoState::Dead) {
        int hx = (m_state == DinoState::Ducking) ? (m_x + 28) : (m_x + 18);
        int hy = renderY - 3;
        rasterizer.blitSprite(SpriteData::dinoHelmet(), hx, hy);
    }

    // 4. Milestone Cosmetics (Score Rewards)
    if (m_state != DinoState::Dead) {
        // Score >= 500: Pixel Sunglasses 😎
        if (score >= 500) {
            int glassesX = (m_state == DinoState::Ducking) ? (m_x + 36) : (m_x + 21);
            int glassesY = renderY + 2;
            rasterizer.blitSprite(SpriteData::sunglasses(), glassesX, glassesY);
        }

        // Score >= 1000: Party Hat 🥳
        if (score >= 1000) {
            int hatX = (m_state == DinoState::Ducking) ? (m_x + 32) : (m_x + 18);
            int hatY = renderY - 11;
            rasterizer.blitSprite(SpriteData::partyHat(), hatX, hatY);
        }
    }
}
