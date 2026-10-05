#include "DinoGame.h"
#include "../graphics/SpriteData.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <random>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <algorithm>

DinoGame::DinoGame(int width, int height)
    : m_width(width), m_height(height),
      m_dino(m_groundY), m_env(width, height, m_groundY)
{
    loadScores();
    reset();
}

void DinoGame::loadScores() {
    m_topScores.clear();

    // 1. Check OS-standard writable app data directory (Mobile / Android / Desktop)
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    std::string scoreFile = (dataDir + "/highscores.txt").toStdString();

    std::ifstream file(scoreFile);
    if (!file.is_open()) {
        // Fallback to applicationDirPath for desktop portable mode
        std::string localFile = (QCoreApplication::applicationDirPath() + "/highscores.txt").toStdString();
        file.open(localFile);
        if (!file.is_open()) {
            file.open("highscores.txt");
        }
    }
    int s;
    while (file >> s) {
        if (s > 0) {
            m_topScores.push_back(s);
        }
    }
    std::sort(m_topScores.rbegin(), m_topScores.rend());
    if (m_topScores.size() > 5) {
        m_topScores.resize(5);
    }
    m_highScore = m_topScores.empty() ? 0 : m_topScores.front();
}

void DinoGame::saveScores() {
    if (m_score <= 0) return; // Only record genuine scores

    m_topScores.push_back(m_score);
    std::sort(m_topScores.rbegin(), m_topScores.rend());
    // Remove duplicates and keep top 5
    m_topScores.erase(std::unique(m_topScores.begin(), m_topScores.end()), m_topScores.end());
    if (m_topScores.size() > 5) {
        m_topScores.resize(5);
    }
    m_highScore = m_topScores.empty() ? 0 : m_topScores.front();

    // 1. Save to OS-standard app data location
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    std::string scoreFile = (dataDir + "/highscores.txt").toStdString();
    std::ofstream file(scoreFile);
    for (int sc : m_topScores) {
        file << sc << "\n";
    }

    // 2. Also mirror to local application directory for desktop convenience
    std::string localFile = (QCoreApplication::applicationDirPath() + "/highscores.txt").toStdString();
    std::ofstream localOut(localFile);
    for (int sc : m_topScores) {
        localOut << sc << "\n";
    }
}

void DinoGame::reset() {
    m_state = GameState::StartWait;
    m_dino.reset();
    m_env.reset();
    m_particles.clear();
    m_obstacles.clear();
    m_powerups.clear();
    m_weather.reset();
    m_achievements.reset();

    m_scoreAcc = 0.0f;
    m_score = 0;
    m_speed = m_turboMode ? 520.0f : 340.0f;

    m_flashTimer = 0.0f;
    m_isScoreFlashing = false;

    m_spawnTimer = 0.0f;
    m_nextSpawnInterval = m_turboMode ? 1.2f : 1.8f;
    m_powerupSpawnTimer = 0.0f;
    m_bulletTimeTimer = 0.0f;

    m_collisionDetected = false;
    m_hitX = -1;
    m_hitY = -1;
    m_shakeTimer = 0.0f;
    m_milestoneBannerTimer = 0.0f;

    m_gameOverBlinkTimer = 0.0f;
    m_promptTimer = 0.0f;
}

void DinoGame::cyclePaletteMode() {
    int p = static_cast<int>(m_paletteMode);
    p = (p + 1) % 5;
    m_paletteMode = static_cast<PaletteMode>(p);
}

const char* DinoGame::paletteModeName() const {
    switch (m_paletteMode) {
        case PaletteMode::Classic: return "Classic Monochrome";
        case PaletteMode::GameBoy: return "Game Boy DMG-01 (4 Greens)";
        case PaletteMode::Cyberpunk: return "Synthwave Neon";
        case PaletteMode::BayerDither: return "1-Bit Bayer 4x4 Dithered";
        case PaletteMode::AmberCRT: return "Vintage Amber Phosphor";
    }
    return "Classic";
}

void DinoGame::toggleTurboMode() {
    m_turboMode = !m_turboMode;
    reset();
}

void DinoGame::onJumpPressed() {
    if (m_state == GameState::StartWait) {
        m_state = GameState::Playing;
        if (m_dino.startJump()) {
            m_audio.playJump();
        }
        m_achievements.trigger("first_jump", &m_audio);
    } else if (m_state == GameState::Playing) {
        if (m_dino.startJump()) {
            m_audio.playJump();
        }
        m_achievements.trigger("first_jump", &m_audio);
    } else if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
    } else if (m_state == GameState::GameOver) {
        reset();
        m_state = GameState::Playing;
        if (m_dino.startJump()) {
            m_audio.playJump();
        }
        m_achievements.trigger("first_jump", &m_audio);
    }
}

void DinoGame::onJumpReleased() {
    if (m_state == GameState::Playing) {
        m_dino.endJump();
    }
}

void DinoGame::onDuckPressed() {
    if (m_state == GameState::Playing) {
        m_dino.setDucking(true);
    }
}

void DinoGame::onDuckReleased() {
    if (m_state == GameState::Playing) {
        m_dino.setDucking(false);
    }
}

void DinoGame::onRestartPressed() {
    reset();
    m_state = GameState::Playing;
}

void DinoGame::togglePause() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
    } else if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
    }
}

void DinoGame::pauseGame() {
    if (m_state == GameState::Playing) {
        m_state = GameState::Paused;
    }
}

void DinoGame::resumeGame() {
    if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
    }
}

void DinoGame::toggleSettingsModal() {
    m_showSettings = !m_showSettings;
    if (m_showSettings && m_state == GameState::Playing) {
        m_state = GameState::Paused;
    }
}

bool DinoGame::handleTouchPress(int x, int y) {
    // 1. Settings Modal Interactivity
    if (m_showSettings) {
        int cardW = 460;
        int cardH = 228;
        int cardX = (m_width - cardW) / 2;
        int cardY = 35;

        if (x >= cardX && x <= cardX + cardW && y >= cardY && y <= cardY + cardH) {
            int relY = y - cardY;
            if (relY >= 20 && relY < 40) {
                cycleWeather();
            } else if (relY >= 40 && relY < 60) {
                cyclePaletteMode();
            } else if (relY >= 60 && relY < 80) {
                toggleBgm();
            } else if (relY >= 80 && relY < 100) {
                toggleMute();
            } else if (relY >= 100 && relY < 120) {
                toggleScanlines();
            } else if (relY >= 120 && relY < 140) {
                toggleAntiAliasing();
            } else if (relY >= 140 && relY < 160) {
                toggleTurboMode();
            } else if (relY >= 160 && relY < 180) {
                toggleVirtualButtons();
            } else if (relY >= 185) {
                m_showSettings = false;
                if (m_state == GameState::Paused) m_state = GameState::Playing;
            }
        } else {
            // Tapping outside modal closes it
            m_showSettings = false;
            if (m_state == GameState::Paused) m_state = GameState::Playing;
        }
        return true;
    }

    // 2. Settings button in top-right (x in [750, 796], y in [5, 36])
    if (x >= 750 && x <= 796 && y >= 5 && y <= 36) {
        toggleSettingsModal();
        return true;
    }

    // 3. If Game is Paused -> tap anywhere resumes
    if (m_state == GameState::Paused) {
        m_state = GameState::Playing;
        return true;
    }

    // 4. If StartWait -> start jumping
    if (m_state == GameState::StartWait) {
        onJumpPressed();
        return true;
    }

    // 5. If GameOver -> restart run
    if (m_state == GameState::GameOver) {
        reset();
        m_state = GameState::Playing;
        onJumpPressed();
        return true;
    }

    // 6. Virtual touch pads during Playing
    if (m_state == GameState::Playing && m_showVirtualButtons) {
        // Duck Pad: left side
        if (x <= 95 && y >= 200) {
            onDuckPressed();
            return true;
        }
        // Jump Pad: right side
        if (x >= 705 && y >= 200) {
            onJumpPressed();
            return true;
        }
    }

    return false;
}

bool DinoGame::handleTouchRelease(int x, int y) {
    if (m_showSettings || m_state == GameState::Paused) {
        return true;
    }
    if (m_state == GameState::Playing && m_showVirtualButtons) {
        if (x <= 95 && y >= 200) {
            onDuckReleased();
            return true;
        }
        if (x >= 705 && y >= 200) {
            onJumpReleased();
            return true;
        }
    }
    return false;
}

void DinoGame::spawnObstacle() {
    static std::mt19937 rng(12345);
    std::uniform_int_distribution<int> cactusDist(0, 4);
    std::uniform_int_distribution<int> pteroHeightDist(0, 2);
    std::uniform_int_distribution<int> rollDist(0, 100);

    float spawnX = static_cast<float>(m_width + 40);

    // Obstacle Combo (High score / Turbo mode challenge):
    // Spawns a small cactus followed closely by low pterodactyl (Jump -> Duck combo!)
    if ((m_score >= 450 || m_turboMode) && rollDist(rng) < 25) {
        m_obstacles.emplace_back(ObstacleType::CactusSmall1, spawnX, m_groundY);
        m_obstacles.emplace_back(ObstacleType::Pterodactyl, spawnX + 175.0f, m_groundY, 0); // Low fly height
    } else if (m_score >= 350 && rollDist(rng) < 40) {
        int heightLevel = pteroHeightDist(rng);
        m_obstacles.emplace_back(ObstacleType::Pterodactyl, spawnX, m_groundY, heightLevel);
    } else {
        int cType = cactusDist(rng);
        ObstacleType ot = ObstacleType::CactusSmall1;
        if (cType == 0) ot = ObstacleType::CactusSmall1;
        else if (cType == 1) ot = ObstacleType::CactusSmall2;
        else if (cType == 2) ot = ObstacleType::CactusSmall3;
        else if (cType == 3) ot = ObstacleType::CactusBig1;
        else if (cType == 4) ot = ObstacleType::CactusBig2;

        m_obstacles.emplace_back(ot, spawnX, m_groundY);
    }

    std::uniform_real_distribution<float> jitter(0.0f, 0.8f);
    float baseInterval = m_turboMode ? 1.0f : std::max(1.1f, 1.8f - (m_speed - 340.0f) * 0.0015f);
    m_nextSpawnInterval = baseInterval + jitter(rng);
    m_spawnTimer = 0.0f;
}

void DinoGame::checkCollisions() {
    if (m_state != GameState::Playing) return;

    // Multi-box sub-hitboxes for precise, fair collision
    std::vector<AABB> subBoxes = m_dino.subHitboxes();

    for (const auto& obs : m_obstacles) {
        AABB obsBox = obs.boundingBox();

        bool broadIntersect = false;
        for (const auto& sb : subBoxes) {
            if (sb.intersects(obsBox)) {
                broadIntersect = true;
                break;
            }
        }

        if (broadIntersect) {
            // Narrowphase Pixel-Perfect Raster Mask Check
            int px = -1, py = -1;
            bool hit = checkPixelCollision(
                m_dino.currentSprite(), m_dino.x(), m_dino.y(),
                obs.currentSprite(), obs.x(), obs.y(),
                px, py
            );

            if (hit) {
                if (m_dino.hasShield()) {
                    // Shield absorbs impact and shatters!
                    m_dino.setShield(false);
                    m_audio.playShieldBreak();
                    m_shakeTimer = 0.16f;
                    m_particles.emitCollisionSparks(static_cast<float>(px), static_cast<float>(py), 25);
                    // Erase obstacle so player passes through safely
                    for (auto it = m_obstacles.begin(); it != m_obstacles.end(); ++it) {
                        if (&(*it) == &obs) {
                            m_obstacles.erase(it);
                            break;
                        }
                    }
                    break;
                }

                m_collisionDetected = true;
                m_hitX = px;
                m_hitY = py;
                m_state = GameState::GameOver;
                m_dino.kill();
                m_audio.playGameOver();
                m_particles.emitCollisionSparks(static_cast<float>(px), static_cast<float>(py), 20);

                // Trigger Screen Shake (180ms)
                m_shakeTimer = 0.22f;

                saveScores();
                break;
            }
        }
    }
}

void DinoGame::spawnPowerUp() {
    static std::mt19937 rng(54321);
    std::uniform_int_distribution<int> typeDist(0, 2);
    PowerUpType t = static_cast<PowerUpType>(typeDist(rng));
    float spawnX = static_cast<float>(m_width + 60);
    m_powerups.emplace_back(t, spawnX, m_groundY);
}

void DinoGame::checkPowerUpCollisions() {
    if (m_state != GameState::Playing) return;

    AABB dinoBox = m_dino.boundingBox();
    for (auto& pu : m_powerups) {
        if (pu.isCollected()) continue;
        if (dinoBox.intersects(pu.boundingBox())) {
            pu.collect();
            m_audio.playPowerUp();
            m_particles.emitDustPuff(static_cast<float>(pu.x() + 7), static_cast<float>(pu.y() + 7), 10);

            if (pu.type() == PowerUpType::Shield) {
                m_dino.setShield(true);
                m_achievements.trigger("shield", &m_audio);
            } else if (pu.type() == PowerUpType::BulletTime) {
                m_bulletTimeTimer = 5.0f;
                m_achievements.trigger("bullet_time", &m_audio);
            } else if (pu.type() == PowerUpType::DoubleJump) {
                m_dino.addDoubleJumpCharges(3);
                m_achievements.trigger("double_jump", &m_audio);
            }
        }
    }
}

void DinoGame::update(float dt) {
    // Bullet Time slow-motion mode (dt scaled)
    float effectiveDt = dt;
    if (m_bulletTimeTimer > 0.0f) {
        m_bulletTimeTimer -= dt;
        effectiveDt = dt * 0.45f;
    }

    m_particles.update(effectiveDt);

    if (m_shakeTimer > 0.0f) {
        m_shakeTimer -= dt;
    }

    if (m_milestoneBannerTimer > 0.0f) {
        m_milestoneBannerTimer -= dt;
    }

    m_achievements.update(dt);
    m_promptTimer += dt;

    if (m_state == GameState::StartWait) {
        m_dino.update(dt, m_particles);
        return;
    }

    if (m_state == GameState::Paused) {
        return; // Physics frozen while paused
    }

    if (m_state == GameState::GameOver) {
        m_gameOverBlinkTimer += dt;
        return;
    }

    // Dynamic Weather progression
    m_weather.update(effectiveDt, m_speed);
    if (m_weather.type() == WeatherType::Thunderstorm) {
        m_achievements.trigger("storm", &m_audio);
    }

    // Achievements milestone checking
    if (m_env.isNightMode()) {
        m_achievements.trigger("night_owl", &m_audio);
    }

    // Speed scaling
    float maxSpeed = m_turboMode ? 950.0f : 800.0f;
    float baseSpeed = m_turboMode ? 520.0f : 340.0f;
    m_speed = std::min(maxSpeed, baseSpeed + m_score * (m_turboMode ? 0.35f : 0.22f));

    // Update score
    int oldMilestone = m_score / 100;
    m_scoreAcc += 10.0f * effectiveDt;
    m_score = static_cast<int>(m_scoreAcc);
    int newMilestone = m_score / 100;

    // Milestone fanfare every 100 points
    if (newMilestone > oldMilestone && m_score > 0) {
        m_audio.playScoreMilestone();
        m_isScoreFlashing = true;
        m_flashTimer = 0.0f;

        // Unlock announcements & achievements
        if (m_score >= 500 && m_score < 600) {
            m_milestoneText = "SUNGLASSES UNLOCKED!";
            m_milestoneBannerTimer = 2.5f;
            m_achievements.trigger("sunglasses", &m_audio);
        } else if (m_score >= 1000 && m_score < 1100) {
            m_milestoneText = "PARTY HAT UNLOCKED!";
            m_milestoneBannerTimer = 2.5f;
            m_achievements.trigger("party_hat", &m_audio);
        } else if (m_score >= 2000 && m_score < 2100) {
            m_milestoneText = "GOLDEN DINO UNLOCKED!";
            m_milestoneBannerTimer = 3.0f;
            m_achievements.trigger("golden_dino", &m_audio);
        }
    }

    if (m_isScoreFlashing) {
        m_flashTimer += effectiveDt;
        if (m_flashTimer >= 1.2f) {
            m_isScoreFlashing = false;
        }
    }

    // Update Dino & Environment
    m_dino.update(effectiveDt, m_particles);
    m_env.update(effectiveDt, m_speed, m_score);

    // Spawning Power-ups (every ~16 seconds)
    m_powerupSpawnTimer += effectiveDt;
    if (m_powerupSpawnTimer >= 16.0f) {
        m_powerupSpawnTimer = 0.0f;
        spawnPowerUp();
    }

    // Update & Prune Power-ups
    for (size_t i = 0; i < m_powerups.size(); ) {
        m_powerups[i].update(effectiveDt, m_speed);
        if (m_powerups[i].isOffScreen() || m_powerups[i].isCollected()) {
            m_powerups.erase(m_powerups.begin() + i);
        } else {
            ++i;
        }
    }

    // Check power-up pickup
    checkPowerUpCollisions();

    // Update & Prune Obstacles
    m_spawnTimer += effectiveDt;
    if (m_spawnTimer >= m_nextSpawnInterval) {
        spawnObstacle();
    }

    for (size_t i = 0; i < m_obstacles.size(); ) {
        m_obstacles[i].update(effectiveDt, m_speed);
        if (m_obstacles[i].isOffScreen()) {
            m_obstacles.erase(m_obstacles.begin() + i);
        } else {
            ++i;
        }
    }

    // Check collisions
    checkCollisions();
}

void DinoGame::render(SoftwareRasterizer& rasterizer) {
    // Configure Xiaolin Wu anti-aliasing flag
    rasterizer.setAntiAliasing(m_useAntiAliasing);

    uint32_t bgCol = m_turboMode ? 0xFF2A1015 : m_env.backgroundColor();
    rasterizer.framebuffer().clear(bgCol);

    bool night = m_env.isNightMode() || m_turboMode;

    // 1. Background parallax layers (Mountains, Clouds, Stars, Meteors)
    m_env.renderBackground(rasterizer);

    // 2. Ground & terrain bumps
    m_env.renderForeground(rasterizer);

    // 3. Particles
    m_particles.render(rasterizer);

    // 4. Power-ups (Floating collectibles)
    for (auto& pu : m_powerups) {
        pu.render(rasterizer, night);
    }

    // 5. Obstacles
    for (auto& obs : m_obstacles) {
        obs.render(rasterizer, night);
    }

    // 6. Dino (Drop Shadow, 2D Affine Rotation, Bone Armor, Milestone Cosmetics)
    m_dino.render(rasterizer, night, m_score);

    // 7. Dynamic Weather (Rain drops, splash ripples, Thunderstorm flash, Sandstorm)
    m_weather.render(rasterizer);

    // 8. Lantern / Radial Vignette Shading (if active)
    if (m_radialVignetteActive) {
        int lx = m_dino.x() + 22;
        int ly = m_dino.y() + 18;
        rasterizer.applyRadialVignette(lx, ly, 60, 260, 0.72f);
    }

    // 9. Score & HUD
    drawScore(rasterizer);

    // 10. Milestone Unlock Banner
    if (m_milestoneBannerTimer > 0.0f) {
        drawMilestoneBanner(rasterizer);
    }

    // 11. Start / Game Over / Pause screens
    if (m_state == GameState::StartWait) {
        drawStartPrompt(rasterizer);
    } else if (m_state == GameState::GameOver) {
        drawGameOverScreen(rasterizer);
    } else if (m_state == GameState::Paused && !m_showSettings) {
        drawPauseScreen(rasterizer);
    }

    // 12. Touch Settings Modal
    if (m_showSettings) {
        drawSettingsModal(rasterizer);
    }

    // 13. Virtual On-Screen Touch Buttons (Jump/Duck pads & Gear button)
    drawVirtualButtons(rasterizer);

    // 14. Leaderboard Overlay (if toggled)
    if (m_showLeaderboard) {
        drawLeaderboard(rasterizer);
    }

    // 13. Achievements Toast Popup
    m_achievements.render(rasterizer);

    // 14. Screen Shake on Impact (Viewport Shift)
    if (m_shakeTimer > 0.0f) {
        int shakeX = (rand() % 9) - 4;
        int shakeY = (rand() % 9) - 4;
        rasterizer.framebuffer().applyScreenShake(shakeX, shakeY, bgCol);
    }

    // 15. CRT Scanlines (if active)
    if (m_scanlinesActive) {
        rasterizer.applyScanlines(0.25f);
    }

    // 16. Retro Color Palette Post-Processing
    switch (m_paletteMode) {
        case PaletteMode::Classic:
            break;
        case PaletteMode::GameBoy:
            rasterizer.framebuffer().applyGameBoyPalette();
            break;
        case PaletteMode::Cyberpunk:
            rasterizer.framebuffer().applyCyberpunkPalette();
            break;
        case PaletteMode::BayerDither:
            rasterizer.framebuffer().applyBayerDithering();
            break;
        case PaletteMode::AmberCRT:
            rasterizer.framebuffer().applyAmberPalette();
            break;
    }
}

void DinoGame::drawScore(SoftwareRasterizer& rasterizer) {
    uint32_t textCol = (m_env.isNightMode() || m_turboMode) ? 0xFFFFFFFF : 0xFF2A2D34;
    uint32_t hiCol = (m_env.isNightMode() || m_turboMode) ? 0xFFB0BEC5 : 0xFF78909C;

    char scoreBuf[32];
    std::snprintf(scoreBuf, sizeof(scoreBuf), "%05d", m_score);

    char hiBuf[32];
    std::snprintf(hiBuf, sizeof(hiBuf), "HI %05d", m_highScore);

    // 1. Top-Right Score Card (positioned to the left of the [SET] button with 8px margin)
    int scoreBoxW = (m_highScore > 0) ? 170 : 85;
    int scoreBoxH = 26;
    int gearW = 38;
    int gearX = m_width - gearW - 8;
    int scoreBoxX = gearX - scoreBoxW - 8;
    int scoreBoxY = 8;

    for (int y = scoreBoxY; y < scoreBoxY + scoreBoxH; ++y) {
        for (int x = scoreBoxX; x < scoreBoxX + scoreBoxW; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xBB0F131C);
        }
    }
    rasterizer.drawRectBresenham(scoreBoxX, scoreBoxY, scoreBoxW, scoreBoxH, 0x4400E5FF);

    int curTextX = scoreBoxX + 8;
    if (m_highScore > 0) {
        rasterizer.drawTextBitmap(curTextX, scoreBoxY + 5, hiBuf, hiCol, 2);
        curTextX += 95;
    }
    if (!m_isScoreFlashing || (static_cast<int>(m_flashTimer * 8.0f) % 2 == 0)) {
        rasterizer.drawTextBitmap(curTextX, scoreBoxY + 5, scoreBuf, textCol, 2);
    }

    // 2. Top-Left Live Dashboard (Speedometer, Weather tag, Active Power-Ups)
    if (m_state == GameState::Playing) {
        int dashW = 245;
        int dashH = 26;
        int dashX = 8;
        int dashY = 8;

        for (int y = dashY; y < dashY + dashH; ++y) {
            for (int x = dashX; x < dashX + dashW; ++x) {
                rasterizer.framebuffer().setPixelBlend(x, y, 0xBB0F131C);
            }
        }
        rasterizer.drawRectBresenham(dashX, dashY, dashW, dashH, 0x4400E5FF);

        float spdRatio = m_speed / 340.0f;
        char spdBuf[32];
        std::snprintf(spdBuf, sizeof(spdBuf), "SPD: %.1fx (%d)", spdRatio, static_cast<int>(m_speed));
        rasterizer.drawTextBitmap(dashX + 6, dashY + 4, spdBuf, 0xFF00FF88, 1);

        const char* wShort = "CLEAR";
        uint32_t wCol = 0xFF80D8FF;
        if (m_weather.type() == WeatherType::Rain) { wShort = "RAIN"; wCol = 0xFF40C4FF; }
        else if (m_weather.type() == WeatherType::Thunderstorm) { wShort = "STORM"; wCol = 0xFFFFD700; }
        else if (m_weather.type() == WeatherType::Sandstorm) { wShort = "SAND"; wCol = 0xFFFFAB40; }
        char wxBuf[32];
        std::snprintf(wxBuf, sizeof(wxBuf), "WX: %s", wShort);
        rasterizer.drawTextBitmap(dashX + 138, dashY + 4, wxBuf, wCol, 1);

        // Power-up mini status indicators on row 2
        int subTagX = dashX + 6;
        int subTagY = dashY + 15;
        if (m_turboMode) {
            rasterizer.drawTextBitmap(subTagX, subTagY, "[TURBO]", 0xFFFF3333, 1);
            subTagX += 48;
        }
        if (m_dino.hasShield()) {
            rasterizer.drawTextBitmap(subTagX, subTagY, "[SHIELD]", 0xFF00FFCC, 1);
            subTagX += 54;
        }
        if (m_bulletTimeTimer > 0.0f) {
            char btBuf[32];
            std::snprintf(btBuf, sizeof(btBuf), "[SLOW-MO %.1fs]", m_bulletTimeTimer);
            rasterizer.drawTextBitmap(subTagX, subTagY, btBuf, 0xFFFFD700, 1);
            subTagX += 98;
        }
        if (m_dino.hasDoubleJump()) {
            char djBuf[32];
            std::snprintf(djBuf, sizeof(djBuf), "[2X JUMP x%d]", m_dino.doubleJumpCharges());
            rasterizer.drawTextBitmap(subTagX, subTagY, djBuf, 0xFFFF00CC, 1);
        }
    }
}

void DinoGame::drawMilestoneBanner(SoftwareRasterizer& rasterizer) {
    int bw = 320;
    int bh = 34;
    int bx = (m_width - bw) / 2;
    int by = 35;

    // Semi-transparent banner panel
    for (int y = by; y < by + bh; ++y) {
        for (int x = bx; x < bx + bw; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xDD111115);
        }
    }
    rasterizer.drawRectBresenham(bx, by, bw, bh, 0xFFFFD700);

    int textX = bx + (bw - static_cast<int>(m_milestoneText.length()) * 12) / 2;
    rasterizer.drawTextBitmap(textX, by + 10, m_milestoneText, 0xFFFFD700, 2);
}

void DinoGame::drawStartPrompt(SoftwareRasterizer& rasterizer) {
    int cardW = 580;
    int cardH = 152;
    int cardX = (m_width - cardW) / 2;
    int cardY = 44;

    // Dark translucent glass card
    for (int y = cardY; y < cardY + cardH; ++y) {
        for (int x = cardX; x < cardX + cardW; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xEE0E1119);
        }
    }

    // Glowing cyber double-border
    rasterizer.drawRectBresenham(cardX, cardY, cardW, cardH, 0xFF00E5FF);
    rasterizer.drawRectBresenham(cardX + 2, cardY + 2, cardW - 4, cardH - 4, 0x4400E5FF);

    // 4 golden corner brackets
    uint32_t gold = 0xFFFFD700;
    for (int i = 0; i < 5; ++i) {
        rasterizer.setPixel(cardX + i, cardY, gold);
        rasterizer.setPixel(cardX, cardY + i, gold);
        rasterizer.setPixel(cardX + cardW - 1 - i, cardY, gold);
        rasterizer.setPixel(cardX + cardW - 1, cardY + i, gold);
        rasterizer.setPixel(cardX + i, cardY + cardH - 1, gold);
        rasterizer.setPixel(cardX, cardY + cardH - 1 - i, gold);
        rasterizer.setPixel(cardX + cardW - 1 - i, cardY + cardH - 1, gold);
        rasterizer.setPixel(cardX + cardW - 1, cardY + cardH - 1 - i, gold);
    }

    // Header Title
    const char* title = "CHROME DINO: PURE SOFTWARE RASTER ENGINE";
    int titleLen = static_cast<int>(std::strlen(title)) * 6;
    rasterizer.drawTextBitmap(cardX + (cardW - titleLen) / 2, cardY + 8, title, gold, 1);
    rasterizer.drawLineBresenham(cardX + 16, cardY + 20, cardX + cardW - 16, cardY + 20, 0x4400E5FF);

    // Blinking Call to Action
    bool pulse = (static_cast<int>(m_promptTimer * 3.5f) % 2 == 0);
    uint32_t promptCol = pulse ? 0xFF00FFCC : 0xFF00BFA5;
    const char* prompt = "► PRESS SPACE OR TAP TO JUMP ◄";
    int promptLen = static_cast<int>(std::strlen(prompt)) * 12;
    rasterizer.drawTextBitmap(cardX + (cardW - promptLen) / 2, cardY + 26, prompt, promptCol, 2);

    // Sub-instruction
    const char* subText = "HOLD TO JUMP HIGHER  |  DOWN / SWIPE DOWN TO DUCK";
    int subLen = static_cast<int>(std::strlen(subText)) * 6;
    rasterizer.drawTextBitmap(cardX + (cardW - subLen) / 2, cardY + 52, subText, 0xFFB0BEC5, 1);
    rasterizer.drawLineBresenham(cardX + 24, cardY + 66, cardX + cardW - 24, cardY + 66, 0x33FFFFFF);

    // Categorized Badges
    const char* row1 = "[P] PALETTES    [W] WEATHER    [V] LANTERN    [C] SCANLINES";
    int r1Len = static_cast<int>(std::strlen(row1)) * 6;
    rasterizer.drawTextBitmap(cardX + (cardW - r1Len) / 2, cardY + 74, row1, 0xFF00E5FF, 1);

    const char* row2 = "[T] TURBO MODE    [A] ANTI-ALIAS    [B] 8-BIT BGM    [M] MUTE";
    int r2Len = static_cast<int>(std::strlen(row2)) * 6;
    rasterizer.drawTextBitmap(cardX + (cardW - r2Len) / 2, cardY + 92, row2, 0xFFFFD700, 1);

    const char* row3 = "[F1] CG INSPECTOR HUD       [F2] 7-ALGORITHM LAB SANDBOX";
    int r3Len = static_cast<int>(std::strlen(row3)) * 6;
    rasterizer.drawTextBitmap(cardX + (cardW - r3Len) / 2, cardY + 110, row3, 0xFF00FF88, 1);

    const char* row4 = "[L] LEADERBOARD       [R] RESTART RUN       [F11] FULLSCREEN";
    int r4Len = static_cast<int>(std::strlen(row4)) * 6;
    rasterizer.drawTextBitmap(cardX + (cardW - r4Len) / 2, cardY + 128, row4, 0xFF90A4AE, 1);
}

void DinoGame::drawGameOverScreen(SoftwareRasterizer& rasterizer) {
    int cardW = 380;
    int cardH = 144;
    int cardX = (m_width - cardW) / 2;
    int cardY = 56;

    // Dark crimson frosted glass
    for (int y = cardY; y < cardY + cardH; ++y) {
        for (int x = cardX; x < cardX + cardW; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xEE160E12);
        }
    }

    // Double crimson border
    rasterizer.drawRectBresenham(cardX, cardY, cardW, cardH, 0xFFFF0055);
    rasterizer.drawRectBresenham(cardX + 2, cardY + 2, cardW - 4, cardH - 4, 0x44FF0055);

    // "G A M E   O V E R"
    const char* goText = "G A M E   O V E R";
    int goLen = static_cast<int>(std::strlen(goText)) * 18;
    rasterizer.drawTextBitmap(cardX + (cardW - goLen) / 2, cardY + 12, goText, 0xFFFF2266, 3);

    // Scores summary
    char statBuf[64];
    std::snprintf(statBuf, sizeof(statBuf), "FINAL SCORE: %05d   BEST: %05d", m_score, m_highScore);
    int statLen = static_cast<int>(std::strlen(statBuf)) * 6;
    rasterizer.drawTextBitmap(cardX + (cardW - statLen) / 2, cardY + 48, statBuf, 0xFFFFD700, 1);

    // Restart Icon
    int btnX = cardX + (cardW - SpriteData::restartIcon().width()) / 2;
    int btnY = cardY + 68;
    if (m_env.isNightMode() || m_turboMode) {
        Sprite inv = SpriteData::restartIcon().invertedColors();
        rasterizer.blitSprite(inv, btnX, btnY);
    } else {
        rasterizer.blitSprite(SpriteData::restartIcon(), btnX, btnY);
    }

    // Blinking prompt
    if (static_cast<int>(m_gameOverBlinkTimer * 3.0f) % 2 == 0) {
        const char* retryText = "► PRESS SPACE, R OR TAP TO RETRY ◄";
        int retryLen = static_cast<int>(std::strlen(retryText)) * 6;
        rasterizer.drawTextBitmap(cardX + (cardW - retryLen) / 2, cardY + 118, retryText, 0xFF00FFCC, 1);
    }
}

void DinoGame::drawLeaderboard(SoftwareRasterizer& rasterizer) {
    int lw = 260;
    int lh = 170;
    int lx = (m_width - lw) / 2;
    int ly = (m_height - lh) / 2;

    for (int y = ly; y < ly + lh; ++y) {
        for (int x = lx; x < lx + lw; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xEE111318);
        }
    }
    rasterizer.drawRectBresenham(lx, ly, lw, lh, 0xFF00E5FF);
    rasterizer.drawTextBitmap(lx + 55, ly + 12, "LOCAL LEADERBOARD", 0xFF00E5FF, 2);

    for (size_t i = 0; i < m_topScores.size(); ++i) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "#%zu  ......  %05d PTS", i + 1, m_topScores[i]);
        uint32_t rankCol = (i == 0) ? 0xFFFFD700 : ((i == 1) ? 0xFFC0C0C0 : 0xFFFFFFFF);
        rasterizer.drawTextBitmap(lx + 45, ly + 45 + static_cast<int>(i) * 20, buf, rankCol, 1);
    }

    rasterizer.drawTextBitmap(lx + 60, ly + lh - 18, "PRESS [L] TO CLOSE", 0xFF888888, 1);
}

void DinoGame::drawPauseScreen(SoftwareRasterizer& rasterizer) {
    int cardW = 360;
    int cardH = 92;
    int cardX = (m_width - cardW) / 2;
    int cardY = 95;

    for (int y = cardY; y < cardY + cardH; ++y) {
        for (int x = cardX; x < cardX + cardW; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xEE0E1119);
        }
    }

    rasterizer.drawRectBresenham(cardX, cardY, cardW, cardH, 0xFF00E5FF);
    rasterizer.drawRectBresenham(cardX + 2, cardY + 2, cardW - 4, cardH - 4, 0x4400E5FF);

    const char* pTitle = "G A M E   P A U S E D";
    int pLen = static_cast<int>(std::strlen(pTitle)) * 12;
    rasterizer.drawTextBitmap(cardX + (cardW - pLen) / 2, cardY + 16, pTitle, 0xFFFFD700, 2);

    const char* pSub = "► TAP SCREEN OR PRESS ESC TO RESUME ◄";
    int sLen = static_cast<int>(std::strlen(pSub)) * 6;
    rasterizer.drawTextBitmap(cardX + (cardW - sLen) / 2, cardY + 54, pSub, 0xFF00FFCC, 1);
}

void DinoGame::drawSettingsModal(SoftwareRasterizer& rasterizer) {
    int cardW = 460;
    int cardH = 228;
    int cardX = (m_width - cardW) / 2;
    int cardY = 35;

    for (int y = cardY; y < cardY + cardH; ++y) {
        for (int x = cardX; x < cardX + cardW; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xF20F121A);
        }
    }

    rasterizer.drawRectBresenham(cardX, cardY, cardW, cardH, 0xFFFFD700);
    rasterizer.drawRectBresenham(cardX + 2, cardY + 2, cardW - 4, cardH - 4, 0x4400E5FF);

    const char* sTitle = "TOUCH SETTINGS & PREFERENCES";
    int tLen = static_cast<int>(std::strlen(sTitle)) * 6;
    rasterizer.drawTextBitmap(cardX + (cardW - tLen) / 2, cardY + 8, sTitle, 0xFFFFD700, 1);
    rasterizer.drawLineBresenham(cardX + 16, cardY + 20, cardX + cardW - 16, cardY + 20, 0x4400E5FF);

    char rowBuf[64];

    std::snprintf(rowBuf, sizeof(rowBuf), "[1] WEATHER:  %s", weatherName());
    rasterizer.drawTextBitmap(cardX + 24, cardY + 26, rowBuf, 0xFF80D8FF, 1);

    std::snprintf(rowBuf, sizeof(rowBuf), "[2] PALETTE:  %s", paletteModeName());
    rasterizer.drawTextBitmap(cardX + 24, cardY + 46, rowBuf, 0xFFFFD700, 1);

    std::snprintf(rowBuf, sizeof(rowBuf), "[3] 8-BIT BGM:  %s", isBgmActive() ? "ENABLED [ON]" : "MUTED [OFF]");
    rasterizer.drawTextBitmap(cardX + 24, cardY + 66, rowBuf, isBgmActive() ? 0xFF00FF88 : 0xFF78909C, 1);

    std::snprintf(rowBuf, sizeof(rowBuf), "[4] SFX AUDIO:  %s", !isMuted() ? "ENABLED [ON]" : "MUTED [OFF]");
    rasterizer.drawTextBitmap(cardX + 24, cardY + 86, rowBuf, !isMuted() ? 0xFF00FF88 : 0xFF78909C, 1);

    std::snprintf(rowBuf, sizeof(rowBuf), "[5] CRT SCANLINES:  %s", isScanlinesActive() ? "ACTIVE [ON]" : "DISABLED [OFF]");
    rasterizer.drawTextBitmap(cardX + 24, cardY + 106, rowBuf, isScanlinesActive() ? 0xFFB388FF : 0xFF78909C, 1);

    std::snprintf(rowBuf, sizeof(rowBuf), "[6] ANTI-ALIASING:  %s", isAntiAliasing() ? "XIAOLIN WU [ON]" : "BRESENHAM [OFF]");
    rasterizer.drawTextBitmap(cardX + 24, cardY + 126, rowBuf, isAntiAliasing() ? 0xFF00E5FF : 0xFF78909C, 1);

    std::snprintf(rowBuf, sizeof(rowBuf), "[7] TURBO HARDCORE:  %s", isTurboMode() ? "ENABLED [ON]" : "NORMAL [OFF]");
    rasterizer.drawTextBitmap(cardX + 24, cardY + 146, rowBuf, isTurboMode() ? 0xFFFF3333 : 0xFF78909C, 1);

    std::snprintf(rowBuf, sizeof(rowBuf), "[8] TOUCH PADS:  %s", m_showVirtualButtons ? "VISIBLE [ON]" : "HIDDEN [OFF]");
    rasterizer.drawTextBitmap(cardX + 24, cardY + 166, rowBuf, m_showVirtualButtons ? 0xFF00FFCC : 0xFF78909C, 1);

    rasterizer.drawLineBresenham(cardX + 20, cardY + 186, cardX + cardW - 20, cardY + 186, 0x33FFFFFF);

    const char* closePrompt = "► [ TAP HERE OR RESUME GAME ] ◄";
    int cLen = static_cast<int>(std::strlen(closePrompt)) * 6;
    rasterizer.drawTextBitmap(cardX + (cardW - cLen) / 2, cardY + 198, closePrompt, 0xFF00FFCC, 1);
}

void DinoGame::drawVirtualButtons(SoftwareRasterizer& rasterizer) {
    // 1. Settings Gear button in top-right
    int gearW = 38;
    int gearH = 26;
    int gearX = m_width - gearW - 8;
    int gearY = 8;
    for (int y = gearY; y < gearY + gearH; ++y) {
        for (int x = gearX; x < gearX + gearW; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xCC10141D);
        }
    }
    rasterizer.drawRectBresenham(gearX, gearY, gearW, gearH, 0x5500E5FF);
    rasterizer.drawTextBitmap(gearX + 7, gearY + 5, "SET", 0xFF00E5FF, 2);

    // 2. On-screen Duck & Jump touch pads during gameplay
    if (m_state == GameState::Playing && m_showVirtualButtons) {
        // Left Duck Pad:
        int duckCx = 48;
        int duckCy = 248;
        rasterizer.fillCircleMidpoint(duckCx, duckCy, 24, 0x33202838);
        rasterizer.drawCircleMidpoint(duckCx, duckCy, 24, 0x6600E5FF);
        rasterizer.drawTextBitmap(duckCx - 12, duckCy - 4, "DUCK", 0xBB00E5FF, 1);

        // Right Jump Pad:
        int jumpCx = 752;
        int jumpCy = 248;
        rasterizer.fillCircleMidpoint(jumpCx, jumpCy, 24, 0x33202838);
        rasterizer.drawCircleMidpoint(jumpCx, jumpCy, 24, 0x6600FF88);
        rasterizer.drawTextBitmap(jumpCx - 12, jumpCy - 4, "JUMP", 0xBB00FF88, 1);
    }
}
