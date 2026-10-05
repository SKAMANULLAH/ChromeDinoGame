#ifndef DINOWIDGET_H
#define DINOWIDGET_H

#include <QWidget>
#include <QTimer>
#include <vector>

/**
 * @file DinoWidget.h
 * @brief Phase 1 Milestone: Clean, Simple Qt 2D Dino Game.
 * Uses QPainter, QTimer, and basic discrete physics.
 */
class DinoWidget : public QWidget {
    Q_OBJECT

public:
    explicit DinoWidget(QWidget *parent = nullptr);
    ~DinoWidget() override = default;

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    void resetGame();
    void updatePhysics();
    void updateObstacles();
    void checkCollisions();

    enum class GameState { Playing, GameOver };
    GameState m_state{GameState::Playing};

    // Dino Kinematics
    const float m_dinoX{60.0f};
    float m_dinoY{176.0f};
    float m_dinoVy{0.0f};
    const float m_dinoW{40.0f};
    const float m_dinoH{44.0f};
    bool m_isGrounded{true};

    // Environment & Physics constants
    const float m_groundY{220.0f};
    const float m_gravity{0.7f};
    const float m_jumpForce{-12.5f};

    // Obstacle Structure
    struct Obstacle {
        float x;
        float width;
        float height;
    };
    std::vector<Obstacle> m_obstacles;
    float m_spawnTimer{0.0f};
    float m_gameSpeed{5.0f};

    // Score & Loop Timer
    int m_score{0};
    int m_highScore{0};
    QTimer *m_timer{nullptr};
};

#endif // DINOWIDGET_H
