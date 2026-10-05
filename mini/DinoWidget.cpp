#include "DinoWidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <cstdlib>
#include <algorithm>

DinoWidget::DinoWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(800, 300);
    setWindowTitle("Chrome Dino - Phase 1 (Qt Milestone)");
    setFocusPolicy(Qt::StrongFocus);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DinoWidget::gameLoop);
    m_timer->start(16); // ~60 FPS

    resetGame();
}

void DinoWidget::resetGame() {
    m_state = GameState::Playing;
    m_dinoY = m_groundY - m_dinoH;
    m_dinoVy = 0.0f;
    m_isGrounded = true;
    m_score = 0;
    m_gameSpeed = 5.5f;
    m_spawnTimer = 0.0f;
    m_obstacles.clear();
}

void DinoWidget::gameLoop() {
    if (m_state == GameState::Playing) {
        updatePhysics();
        updateObstacles();
        checkCollisions();

        // Increment score gradually
        m_score++;
        if (m_score > m_highScore) {
            m_highScore = m_score;
        }

        // Slightly increase game speed over time
        if (m_score % 500 == 0 && m_gameSpeed < 11.0f) {
            m_gameSpeed += 0.4f;
        }
    }
    update(); // Triggers paintEvent
}

void DinoWidget::updatePhysics() {
    // Kinematic motion: y = y + vy, vy = vy + gravity
    if (!m_isGrounded) {
        m_dinoVy += m_gravity;
        m_dinoY += m_dinoVy;

        // Ground collision
        if (m_dinoY >= m_groundY - m_dinoH) {
            m_dinoY = m_groundY - m_dinoH;
            m_dinoVy = 0.0f;
            m_isGrounded = true;
        }
    }
}

void DinoWidget::updateObstacles() {
    // Move existing obstacles left
    for (auto &obs : m_obstacles) {
        obs.x -= m_gameSpeed;
    }

    // Remove off-screen obstacles
    m_obstacles.erase(
        std::remove_if(m_obstacles.begin(), m_obstacles.end(), [](const Obstacle &o) {
            return o.x + o.width < 0;
        }),
        m_obstacles.end()
    );

    // Spawn new obstacles with random spacing
    m_spawnTimer += 1.0f;
    if (m_spawnTimer > 75.0f) {
        if (m_obstacles.empty() || (800.0f - m_obstacles.back().x) > (220.0f + (rand() % 160))) {
            float obsHeight = 36.0f + (rand() % 20);
            float obsWidth = 20.0f + (rand() % 16);
            m_obstacles.push_back({820.0f, obsWidth, obsHeight});
            m_spawnTimer = 0.0f;
        }
    }
}

void DinoWidget::checkCollisions() {
    QRectF dinoRect(m_dinoX + 4, m_dinoY + 4, m_dinoW - 8, m_dinoH - 8);

    for (const auto &obs : m_obstacles) {
        QRectF obsRect(obs.x + 3, m_groundY - obs.height, obs.width - 6, obs.height);
        if (dinoRect.intersects(obsRect)) {
            m_state = GameState::GameOver;
            break;
        }
    }
}

void DinoWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Up) {
        if (m_state == GameState::Playing) {
            if (m_isGrounded) {
                m_dinoVy = m_jumpForce;
                m_isGrounded = false;
            }
        } else if (m_state == GameState::GameOver) {
            resetGame();
        }
    }
}

void DinoWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // 1. Clear Background
    painter.fillRect(rect(), QColor(247, 247, 247));

    // 2. Draw Ground Line
    painter.setPen(QPen(QColor(83, 83, 83), 2));
    painter.drawLine(0, static_cast<int>(m_groundY), width(), static_cast<int>(m_groundY));

    // Subtle ground bumps
    painter.setPen(QColor(160, 160, 160));
    for (int gx = 20; gx < width(); gx += 80) {
        painter.drawPoint(gx, static_cast<int>(m_groundY) + 4);
        painter.drawPoint(gx + 8, static_cast<int>(m_groundY) + 6);
    }

    // 3. Draw Dino
    QColor dinoColor(83, 83, 83);
    painter.setBrush(dinoColor);
    painter.setPen(Qt::NoPen);

    // Body
    painter.drawRect(static_cast<int>(m_dinoX + 10), static_cast<int>(m_dinoY + 12), 24, 24);
    // Head
    painter.drawRect(static_cast<int>(m_dinoX + 22), static_cast<int>(m_dinoY), 18, 16);
    // Eye
    painter.fillRect(static_cast<int>(m_dinoX + 26), static_cast<int>(m_dinoY + 4), 3, 3, QColor(247, 247, 247));
    // Tail
    painter.drawRect(static_cast<int>(m_dinoX), static_cast<int>(m_dinoY + 18), 10, 8);
    // Legs
    if (m_isGrounded && (m_score / 6) % 2 == 0) {
        painter.drawRect(static_cast<int>(m_dinoX + 14), static_cast<int>(m_dinoY + 36), 4, 8);
        painter.drawRect(static_cast<int>(m_dinoX + 24), static_cast<int>(m_dinoY + 36), 4, 5);
    } else {
        painter.drawRect(static_cast<int>(m_dinoX + 14), static_cast<int>(m_dinoY + 36), 4, 5);
        painter.drawRect(static_cast<int>(m_dinoX + 24), static_cast<int>(m_dinoY + 36), 4, 8);
    }

    // 4. Draw Obstacles (Cacti)
    painter.setBrush(QColor(70, 110, 70));
    for (const auto &obs : m_obstacles) {
        int ox = static_cast<int>(obs.x);
        int oy = static_cast<int>(m_groundY - obs.height);
        int ow = static_cast<int>(obs.width);
        int oh = static_cast<int>(obs.height);

        // Main stem
        painter.drawRect(ox + ow / 4, oy, ow / 2, oh);
        // Left arm
        if (oh > 30) {
            painter.drawRect(ox, oy + oh / 3, ow / 4, oh / 3);
            painter.drawRect(ox, oy + oh / 4, ow / 4, oh / 4);
        }
        // Right arm
        if (oh > 35) {
            painter.drawRect(ox + 3 * ow / 4, oy + oh / 2, ow / 4, oh / 3);
            painter.drawRect(ox + 3 * ow / 4, oy + oh / 3, ow / 4, oh / 4);
        }
    }

    // 5. Draw HUD (Score)
    QFont scoreFont("Monospace", 11, QFont::Bold);
    painter.setFont(scoreFont);
    painter.setPen(QColor(83, 83, 83));

    QString scoreStr = QString("HI %1  %2")
                           .arg(m_highScore / 5, 5, 10, QChar('0'))
                           .arg(m_score / 5, 5, 10, QChar('0'));
    painter.drawText(width() - 200, 30, scoreStr);

    // 6. Game Over Overlay
    if (m_state == GameState::GameOver) {
        QFont titleFont("Arial", 16, QFont::Bold);
        painter.setFont(titleFont);
        painter.setPen(QColor(83, 83, 83));
        painter.drawText(rect(), Qt::AlignCenter, "G A M E   O V E R\n\n[ Press SPACE to Restart ]");
    }
}
