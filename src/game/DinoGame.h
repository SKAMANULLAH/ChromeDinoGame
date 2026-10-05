#ifndef DINOGAME_H
#define DINOGAME_H

#include <vector>
#include <memory>
#include "../graphics/SoftwareRasterizer.h"
#include "Dino.h"
#include "Obstacle.h"
#include "Environment.h"
#include "ParticleSystem.h"
#include "../audio/RetroAudio.h"

#include "Weather.h"
#include "PowerUp.h"
#include "Achievements.h"

enum class PaletteMode {
    Classic,
    GameBoy,
    Cyberpunk,
    BayerDither,
    AmberCRT
};

enum class GameState {
    StartWait,
    Playing,
    Paused,
    GameOver
};

class DinoGame {
public:
    DinoGame(int width = 800, int height = 300);

    void reset();
    void update(float dt);
    void render(SoftwareRasterizer& rasterizer);

    // Input handlers
    void onJumpPressed();
    void onJumpReleased();
    void onDuckPressed();
    void onDuckReleased();
    void onRestartPressed();

    // Mobile Touch & Pause Controls
    void togglePause();
    void pauseGame();
    void resumeGame();
    bool isPaused() const { return m_state == GameState::Paused; }

    void toggleSettingsModal();
    bool isSettingsOpen() const { return m_showSettings; }

    void toggleVirtualButtons() { m_showVirtualButtons = !m_showVirtualButtons; }
    bool isVirtualButtonsActive() const { return m_showVirtualButtons; }

    bool handleTouchPress(int x, int y);
    bool handleTouchRelease(int x, int y);

    // Palette & Visual Filters
    void cyclePaletteMode();
    PaletteMode paletteMode() const { return m_paletteMode; }
    const char* paletteModeName() const;

    // Weather & Atmosphere
    void cycleWeather() { m_weather.cycleWeather(); }
    const char* weatherName() const { return m_weather.weatherName(); }

    // Vignette Flashlight
    void toggleVignette() { m_radialVignetteActive = !m_radialVignetteActive; }
    bool isVignetteActive() const { return m_radialVignetteActive; }

    // Xiaolin Wu Anti-Aliasing
    void toggleAntiAliasing() { m_useAntiAliasing = !m_useAntiAliasing; }
    bool isAntiAliasing() const { return m_useAntiAliasing; }

    // Turbo Mode
    void toggleTurboMode();
    bool isTurboMode() const { return m_turboMode; }

    // Leaderboard
    void toggleLeaderboard() { m_showLeaderboard = !m_showLeaderboard; }
    bool isLeaderboardOpen() const { return m_showLeaderboard; }

    // Audio & BGM
    void toggleScanlines() { m_scanlinesActive = !m_scanlinesActive; }
    bool isScanlinesActive() const { return m_scanlinesActive; }

    void toggleMute() { m_audio.setMuted(!m_audio.isMuted()); }
    bool isMuted() const { return m_audio.isMuted(); }

    void toggleBgm() { m_audio.toggleBgm(); }
    bool isBgmActive() const { return m_audio.isBgmActive(); }

    // Queries
    GameState state() const { return m_state; }
    int score() const { return m_score; }
    int highScore() const { return m_highScore; }
    bool isNightMode() const { return m_env.isNightMode(); }

    const Dino& dino() const { return m_dino; }
    const std::vector<Obstacle>& obstacles() const { return m_obstacles; }
    bool collisionDetected() const { return m_collisionDetected; }
    int hitX() const { return m_hitX; }
    int hitY() const { return m_hitY; }

private:
    int m_width;
    int m_height;
    int m_groundY{225};

    GameState m_state{GameState::StartWait};
    Dino m_dino;
    Environment m_env;
    ParticleSystem m_particles;
    RetroAudio m_audio;
    Weather m_weather;
    Achievements m_achievements;
    std::vector<Obstacle> m_obstacles;
    std::vector<PowerUp> m_powerups;

    float m_scoreAcc{0.0f};
    int m_score{0};
    int m_highScore{0};
    float m_speed{340.0f};

    PaletteMode m_paletteMode{PaletteMode::Classic};
    bool m_turboMode{false};
    bool m_showLeaderboard{false};
    bool m_radialVignetteActive{false};
    bool m_useAntiAliasing{false};
    float m_bulletTimeTimer{0.0f};
    float m_powerupSpawnTimer{0.0f};
    std::vector<int> m_topScores;

    // Screen Shake
    float m_shakeTimer{0.0f};

    // Milestone announcement banner
    std::string m_milestoneText;
    float m_milestoneBannerTimer{0.0f};

    // Score milestone flash
    float m_flashTimer{0.0f};
    bool m_isScoreFlashing{false};

    // Obstacle spawn timer / distance
    float m_spawnTimer{0.0f};
    float m_nextSpawnInterval{2.0f};

    // Collision information
    bool m_collisionDetected{false};
    int m_hitX{-1};
    int m_hitY{-1};

    // Scanline CRT effect
    bool m_scanlinesActive{false};

    // Mobile & Touch State
    bool m_showSettings{false};
    bool m_showVirtualButtons{true};

    // Game Over blink & prompt pulsation timers
    float m_gameOverBlinkTimer{0.0f};
    float m_promptTimer{0.0f};

    void spawnObstacle();
    void spawnPowerUp();
    void checkCollisions();
    void checkPowerUpCollisions();
    void loadScores();
    void saveScores();
    void drawScore(SoftwareRasterizer& rasterizer);
    void drawGameOverScreen(SoftwareRasterizer& rasterizer);
    void drawStartPrompt(SoftwareRasterizer& rasterizer);
    void drawPauseScreen(SoftwareRasterizer& rasterizer);
    void drawSettingsModal(SoftwareRasterizer& rasterizer);
    void drawVirtualButtons(SoftwareRasterizer& rasterizer);
    void drawLeaderboard(SoftwareRasterizer& rasterizer);
    void drawMilestoneBanner(SoftwareRasterizer& rasterizer);
};

#endif // DINOGAME_H
