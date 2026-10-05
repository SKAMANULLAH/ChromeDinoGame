#ifndef DINO_H
#define DINO_H

#include "Entity.h"
#include "../graphics/SoftwareRasterizer.h"
#include "ParticleSystem.h"

enum class DinoState {
    Idle,
    Running,
    Jumping,
    Ducking,
    Dead
};

class Dino {
public:
    explicit Dino(int groundY = 220);

    void reset();
    void update(float dt, ParticleSystem& particles);
    void render(SoftwareRasterizer& rasterizer, bool nightMode, int score = 0);

    bool startJump(); // returns true if jump or double-jump occurred
    void endJump(); // For variable jump height
    void setDucking(bool ducking);
    void kill();

    // Power-up states
    bool hasShield() const { return m_hasShield; }
    void setShield(bool s) { m_hasShield = s; }
    bool hasDoubleJump() const { return m_hasDoubleJump && m_doubleJumpCharges > 0; }
    int doubleJumpCharges() const { return m_doubleJumpCharges; }
    void addDoubleJumpCharges(int charges = 3) {
        m_doubleJumpCharges += charges;
        m_hasDoubleJump = true;
    }
    void setDoubleJump(bool dj) {
        m_hasDoubleJump = dj;
        m_doubleJumpCharges = dj ? 3 : 0;
    }

    // State queries
    DinoState state() const { return m_state; }
    bool isAlive() const { return m_state != DinoState::Dead; }
    bool isJumping() const { return m_state == DinoState::Jumping; }
    bool isDucking() const { return m_state == DinoState::Ducking; }

    // Collision geometry
    AABB boundingBox() const;
    std::vector<AABB> subHitboxes() const;
    const Sprite& currentSprite() const;
    int x() const { return m_x; }
    int y() const { return static_cast<int>(m_y); }
    float tiltAngle() const { return m_tiltAngle; }

private:
    int m_groundY;
    int m_x{50};
    float m_y{0.0f};
    float m_vy{0.0f};
    float m_gravity{1200.0f};
    float m_jumpVelocity{-540.0f};
    float m_fastFallGravity{2400.0f};
    float m_tiltAngle{0.0f};

    DinoState m_state{DinoState::Idle};
    bool m_isDuckingInput{false};
    bool m_isHoldingJump{false};

    // Power-ups
    bool m_hasShield{false};
    bool m_hasDoubleJump{false};
    bool m_didDoubleJump{false};
    int m_doubleJumpCharges{0};

    // Input buffering & Coyote time
    float m_jumpBufferTimer{0.0f};
    float m_coyoteTimer{0.0f};

    // Eye blinking
    float m_blinkTimer{0.0f};
    bool m_isBlinking{false};

    float m_animTimer{0.0f};
    int m_animFrame{0};
    float m_dustTimer{0.0f};
};

#endif // DINO_H
