#include "MainWindow.h"
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTouchEvent>
#include <QFocusEvent>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>

GameCanvas::GameCanvas(QWidget* parent)
    : QWidget(parent),
      m_framebuffer(NATIVE_WIDTH, NATIVE_HEIGHT),
      m_rasterizer(m_framebuffer),
      m_game(NATIVE_WIDTH, NATIVE_HEIGHT)
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setMinimumSize(NATIVE_WIDTH, NATIVE_HEIGHT);
    m_frameTimer.start();
}

QPoint GameCanvas::mapToNative(const QPoint& screenPos) const {
    int w = width();
    int h = height();
    double targetRatio = static_cast<double>(NATIVE_WIDTH) / NATIVE_HEIGHT;
    double currentRatio = static_cast<double>(w) / std::max(1, h);

    int drawX = 0, drawY = 0, drawW = w, drawH = h;
    if (currentRatio > targetRatio) {
        drawW = static_cast<int>(h * targetRatio);
        drawX = (w - drawW) / 2;
    } else {
        drawH = static_cast<int>(w / targetRatio);
        drawY = (h - drawH) / 2;
    }

    int clampedX = std::clamp(screenPos.x() - drawX, 0, std::max(1, drawW));
    int clampedY = std::clamp(screenPos.y() - drawY, 0, std::max(1, drawH));
    return QPoint(
        std::clamp(static_cast<int>(clampedX * static_cast<double>(NATIVE_WIDTH) / std::max(1, drawW)), 0, NATIVE_WIDTH - 1),
        std::clamp(static_cast<int>(clampedY * static_cast<double>(NATIVE_HEIGHT) / std::max(1, drawH)), 0, NATIVE_HEIGHT - 1)
    );
}

void GameCanvas::gameLoop() {
    qint64 elapsedNs = m_frameTimer.nsecsElapsed();
    m_frameTimer.restart();

    float dt = static_cast<float>(elapsedNs) / 1e9f;
    dt = std::clamp(dt, 0.001f, 0.050f); // Clamp against hitches

    m_frameTimeMs = dt * 1000.0f;
    m_frameCount++;
    m_fpsAccumulator += dt;
    if (m_fpsAccumulator >= 0.5f) {
        m_fps = m_frameCount / m_fpsAccumulator;
        m_frameCount = 0;
        m_fpsAccumulator = 0.0f;
    }

    if (m_inShowcase) {
        m_showcase.update(dt);
    } else {
        m_game.update(dt);
    }

    update();
}

void GameCanvas::paintEvent(QPaintEvent* /*event*/) {
    // Reset frame telemetry for current frame
    m_rasterizer.resetTelemetry();

    if (m_inShowcase) {
        m_showcase.render(m_rasterizer);
    } else {
        m_game.render(m_rasterizer);

        // Overlay CG Lab Inspector if toggled
        if (m_inspector.isActive()) {
            std::vector<AABB> obsBoxes;
            for (const auto& obs : m_game.obstacles()) {
                obsBoxes.push_back(obs.boundingBox());
            }

            m_inspector.renderHUD(
                m_rasterizer,
                m_fps, m_frameTimeMs,
                m_game.score(), m_game.highScore(),
                m_game.dino().boundingBox(),
                obsBoxes,
                m_game.collisionDetected(),
                m_game.hitX(), m_game.hitY()
            );
        }
    }

    // 100% PURE RASTER BLIT TO SCREEN:
    // Present internal raw pixel buffer as a single blit using nearest-neighbor scaling.
    // Preserves native 800x300 aspect ratio with clean matte letterbox bars when resized or fullscreen.
    int w = width();
    int h = height();
    double targetRatio = static_cast<double>(NATIVE_WIDTH) / NATIVE_HEIGHT;
    double currentRatio = static_cast<double>(w) / std::max(1, h);

    QRect targetRect;
    if (currentRatio > targetRatio) {
        int drawW = static_cast<int>(h * targetRatio);
        int drawX = (w - drawW) / 2;
        targetRect = QRect(drawX, 0, drawW, h);
    } else {
        int drawH = static_cast<int>(w / targetRatio);
        int drawY = (h - drawH) / 2;
        targetRect = QRect(0, drawY, w, drawH);
    }

    QPainter painter(this);
    painter.fillRect(rect(), QColor(10, 12, 16)); // Retro dark obsidian matte border

    // 100% PURE RASTER PRESENTATION: single 1:1 image blit
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false); // Crisp pixel art!
    QImage frameImg = m_framebuffer.toQImage();
    painter.drawImage(targetRect, frameImg);

    // Glowing Cybernetic Arcade Bezel around the active 800x300 viewport
    painter.setPen(QColor(0, 229, 255, 65));
    painter.drawRect(targetRect.adjusted(-1, -1, 1, 1));

    // Corner brackets for authentic retro cabinet styling
    int bLen = 14;
    painter.setPen(QPen(QColor(0, 229, 255, 200), 2));
    // Top-Left
    painter.drawLine(targetRect.left() - 2, targetRect.top() - 2, targetRect.left() + bLen, targetRect.top() - 2);
    painter.drawLine(targetRect.left() - 2, targetRect.top() - 2, targetRect.left() - 2, targetRect.top() + bLen);
    // Top-Right
    painter.drawLine(targetRect.right() + 2, targetRect.top() - 2, targetRect.right() - bLen, targetRect.top() - 2);
    painter.drawLine(targetRect.right() + 2, targetRect.top() - 2, targetRect.right() + 2, targetRect.top() + bLen);
    // Bottom-Left
    painter.drawLine(targetRect.left() - 2, targetRect.bottom() + 2, targetRect.left() + bLen, targetRect.bottom() + 2);
    painter.drawLine(targetRect.left() - 2, targetRect.bottom() + 2, targetRect.left() - 2, targetRect.bottom() - bLen);
    // Bottom-Right
    painter.drawLine(targetRect.right() + 2, targetRect.bottom() + 2, targetRect.right() - bLen, targetRect.bottom() + 2);
    painter.drawLine(targetRect.right() + 2, targetRect.bottom() + 2, targetRect.right() + 2, targetRect.bottom() - bLen);
}

void GameCanvas::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;

    int key = event->key();

    if (key == Qt::Key_Space || key == Qt::Key_Up) {
        if (!m_inShowcase) {
            m_game.onJumpPressed();
        }
    } else if (key == Qt::Key_Down || key == Qt::Key_S) {
        if (!m_inShowcase) {
            m_game.onDuckPressed();
        }
    } else if (key == Qt::Key_W) {
        if (!m_inShowcase) {
            m_game.cycleWeather();
        }
    } else if (key == Qt::Key_V) {
        if (!m_inShowcase) {
            m_game.toggleVignette();
        }
    } else if (key == Qt::Key_A) {
        if (!m_inShowcase) {
            m_game.toggleAntiAliasing();
        }
    } else if (key == Qt::Key_B) {
        if (!m_inShowcase) {
            m_game.toggleBgm();
        }
    } else if (key == Qt::Key_F1 || key == Qt::Key_I) {
        m_inspector.toggle();
    } else if (key == Qt::Key_F2 || key == Qt::Key_Tab) {
        toggleShowcase();
    } else if (key == Qt::Key_C) {
        m_game.toggleScanlines();
    } else if (key == Qt::Key_P) {
        m_game.cyclePaletteMode();
    } else if (key == Qt::Key_T) {
        m_game.toggleTurboMode();
    } else if (key == Qt::Key_L) {
        m_game.toggleLeaderboard();
    } else if (key == Qt::Key_M) {
        m_game.toggleMute();
    } else if (key == Qt::Key_R) {
        m_game.onRestartPressed();
    } else if (key == Qt::Key_Left) {
        if (m_inShowcase) m_showcase.prevMode();
    } else if (key == Qt::Key_Right) {
        if (m_inShowcase) m_showcase.nextMode();
    } else if (key == Qt::Key_Escape) {
        if (!m_inShowcase) {
            m_game.togglePause();
        }
    } else if (key == Qt::Key_F11) {
        if (window()->isFullScreen()) {
            window()->showNormal();
        } else {
            window()->showFullScreen();
        }
    }
}

void GameCanvas::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;

    int key = event->key();
    if (key == Qt::Key_Space || key == Qt::Key_Up) {
        if (!m_inShowcase) {
            m_game.onJumpReleased();
        }
    } else if (key == Qt::Key_Down || key == Qt::Key_S) {
        if (!m_inShowcase) {
            m_game.onDuckReleased();
        }
    }
}

void GameCanvas::mousePressEvent(QMouseEvent* event) {
    QPoint nativePt = mapToNative(event->pos());
    if (m_inShowcase) {
        if (event->button() == Qt::LeftButton) {
            m_showcase.handleMousePress(nativePt.x(), nativePt.y());
            update();
        }
        return;
    }

    if (event->button() == Qt::LeftButton) {
        bool consumed = m_game.handleTouchPress(nativePt.x(), nativePt.y());
        if (!consumed) {
            m_touchStartPos = nativePt;
            m_isSwipingDown = false;
            m_touchActive = true;
            m_game.onJumpPressed();
        }
        update();
    }
}

void GameCanvas::mouseMoveEvent(QMouseEvent* event) {
    QPoint nativePt = mapToNative(event->pos());
    if (m_inShowcase) {
        if (event->buttons() & Qt::LeftButton) {
            m_showcase.handleMouseMove(nativePt.x(), nativePt.y());
            update();
        }
        return;
    }

    if (m_touchActive && (event->buttons() & Qt::LeftButton)) {
        int dy = nativePt.y() - m_touchStartPos.y();
        if (dy > 20 && !m_isSwipingDown) {
            m_isSwipingDown = true;
            m_game.onDuckPressed();
        }
    }
}

void GameCanvas::mouseReleaseEvent(QMouseEvent* event) {
    QPoint nativePt = mapToNative(event->pos());
    if (m_inShowcase) {
        if (event->button() == Qt::LeftButton) {
            m_showcase.handleMouseRelease(nativePt.x(), nativePt.y());
            update();
        }
        return;
    }

    if (event->button() == Qt::LeftButton) {
        bool consumed = m_game.handleTouchRelease(nativePt.x(), nativePt.y());
        if (!consumed && m_touchActive) {
            if (m_isSwipingDown) {
                m_game.onDuckReleased();
            } else {
                m_game.onJumpReleased();
            }
        }
        m_touchActive = false;
        m_isSwipingDown = false;
        update();
    }
}

bool GameCanvas::event(QEvent* event) {
    if (event->type() == QEvent::TouchBegin) {
        auto* te = static_cast<QTouchEvent*>(event);
        if (!te->points().isEmpty()) {
            QPoint pos = te->points().first().position().toPoint();
            QPoint nativePt = mapToNative(pos);
            if (!m_inShowcase) {
                bool consumed = m_game.handleTouchPress(nativePt.x(), nativePt.y());
                if (!consumed) {
                    m_touchStartPos = nativePt;
                    m_isSwipingDown = false;
                    m_touchActive = true;
                    m_game.onJumpPressed();
                }
                update();
            }
        }
        event->accept();
        return true;
    } else if (event->type() == QEvent::TouchUpdate) {
        auto* te = static_cast<QTouchEvent*>(event);
        if (!te->points().isEmpty()) {
            QPoint pos = te->points().first().position().toPoint();
            QPoint nativePt = mapToNative(pos);
            if (!m_inShowcase && m_touchActive) {
                int dy = nativePt.y() - m_touchStartPos.y();
                if (dy > 20 && !m_isSwipingDown) {
                    m_isSwipingDown = true;
                    m_game.onDuckPressed();
                }
            }
        }
        event->accept();
        return true;
    } else if (event->type() == QEvent::TouchEnd || event->type() == QEvent::TouchCancel) {
        auto* te = static_cast<QTouchEvent*>(event);
        if (!te->points().isEmpty()) {
            QPoint pos = te->points().first().position().toPoint();
            QPoint nativePt = mapToNative(pos);
            if (!m_inShowcase) {
                bool consumed = m_game.handleTouchRelease(nativePt.x(), nativePt.y());
                if (!consumed && m_touchActive) {
                    if (m_isSwipingDown) {
                        m_game.onDuckReleased();
                    } else {
                        m_game.onJumpReleased();
                    }
                }
                m_touchActive = false;
                m_isSwipingDown = false;
                update();
            }
        }
        event->accept();
        return true;
    }
    return QWidget::event(event);
}

void GameCanvas::focusOutEvent(QFocusEvent* event) {
    QWidget::focusOutEvent(event);
    if (!m_inShowcase && m_game.state() == GameState::Playing) {
        m_game.pauseGame();
    }
}

// -------------------------------------------------------------
// MainWindow Implementation
// -------------------------------------------------------------

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Computer Graphics Lab: Chrome Dino (Pure Software Raster Engine)");
    resize(1200, 480);
    setMinimumSize(960, 420);

    m_canvas = new GameCanvas(this);
    setCentralWidget(m_canvas);

    // Setup Sleek Cyberpunk Top Toolbar
    m_toolbar = addToolBar("CG Lab Controls");
    QToolBar* toolbar = m_toolbar;
    toolbar->setMovable(false);
    toolbar->setStyleSheet(R"(
        QToolBar {
            background-color: #0E1118;
            border-bottom: 2px solid #1C2333;
            spacing: 5px;
            padding: 4px 6px;
        }
        QToolButton {
            background-color: #161A24;
            color: #C5CBD8;
            border: 1px solid #252D3D;
            border-radius: 5px;
            padding: 4px 8px;
            font-size: 11px;
            font-weight: 600;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
        }
        QToolButton:hover {
            background-color: #202738;
            color: #FFFFFF;
            border: 1px solid #00E5FF;
        }
        QToolButton:pressed {
            background-color: #00E5FF;
            color: #0E1016;
        }
        QToolButton:checked {
            background-color: #00363D;
            color: #00E5FF;
            border: 1px solid #00E5FF;
        }
    )");

    m_actPlay = toolbar->addAction("🎮 Game Mode");
    m_actPlay->setCheckable(true);
    connect(m_actPlay, &QAction::triggered, this, [this]() {
        if (m_canvas->isShowcaseActive()) m_canvas->toggleShowcase();
        m_canvas->setFocus();
    });

    m_actShowcase = toolbar->addAction("📐 CG Algorithm Lab [F2]");
    m_actShowcase->setCheckable(true);
    connect(m_actShowcase, &QAction::triggered, this, [this]() {
        if (!m_canvas->isShowcaseActive()) m_canvas->toggleShowcase();
        m_canvas->setFocus();
    });

    toolbar->addSeparator();

    m_actInspector = toolbar->addAction("🔍 Inspector HUD [F1]");
    m_actInspector->setCheckable(true);
    connect(m_actInspector, &QAction::triggered, this, [this]() {
        m_canvas->inspector().toggle();
        m_canvas->setFocus();
    });

    m_actWeather = toolbar->addAction("🌧️ Weather [W]");
    connect(m_actWeather, &QAction::triggered, this, [this]() {
        m_canvas->game().cycleWeather();
        m_canvas->setFocus();
    });

    m_actVignette = toolbar->addAction("🏮 Lantern [V]");
    m_actVignette->setCheckable(true);
    connect(m_actVignette, &QAction::triggered, this, [this]() {
        m_canvas->game().toggleVignette();
        m_canvas->setFocus();
    });

    m_actAA = toolbar->addAction("📐 Anti-Alias [A]");
    m_actAA->setCheckable(true);
    connect(m_actAA, &QAction::triggered, this, [this]() {
        m_canvas->game().toggleAntiAliasing();
        m_canvas->setFocus();
    });

    m_actBgm = toolbar->addAction("🎵 8-Bit BGM [B]");
    m_actBgm->setCheckable(true);
    connect(m_actBgm, &QAction::triggered, this, [this]() {
        m_canvas->game().toggleBgm();
        m_canvas->setFocus();
    });

    m_actScanlines = toolbar->addAction("📺 CRT Scanlines [C]");
    m_actScanlines->setCheckable(true);
    connect(m_actScanlines, &QAction::triggered, this, [this]() {
        m_canvas->game().toggleScanlines();
        m_canvas->setFocus();
    });

    m_actPalette = toolbar->addAction("🎨 Palette [P]");
    connect(m_actPalette, &QAction::triggered, this, [this]() {
        m_canvas->game().cyclePaletteMode();
        m_canvas->setFocus();
    });

    m_actTurbo = toolbar->addAction("⚡ Turbo Mode [T]");
    m_actTurbo->setCheckable(true);
    connect(m_actTurbo, &QAction::triggered, this, [this]() {
        m_canvas->game().toggleTurboMode();
        m_canvas->setFocus();
    });

    m_actLeaderboard = toolbar->addAction("🏆 Leaderboard [L]");
    m_actLeaderboard->setCheckable(true);
    connect(m_actLeaderboard, &QAction::triggered, this, [this]() {
        m_canvas->game().toggleLeaderboard();
        m_canvas->setFocus();
    });

    m_actMute = toolbar->addAction("🔊 Audio Mute [M]");
    m_actMute->setCheckable(true);
    connect(m_actMute, &QAction::triggered, this, [this]() {
        m_canvas->game().toggleMute();
        m_canvas->setFocus();
    });

    QAction* actRestart = toolbar->addAction("🔄 Restart [R]");
    connect(actRestart, &QAction::triggered, this, [this]() {
        m_canvas->game().onRestartPressed();
        m_canvas->setFocus();
    });

    // Sleek Status Bar
    m_statusLabel = new QLabel(this);
    m_statusLabel->setTextFormat(Qt::RichText);
    statusBar()->addWidget(m_statusLabel);
    statusBar()->setStyleSheet(
        "QStatusBar { background-color: #0E1118; border-top: 1px solid #1C2333; padding: 2px 8px; }"
        "QLabel { font-family: monospace; font-size: 11px; }"
    );

    // Game loop timer at 60 FPS (~16ms)
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_canvas->gameLoop();
        updateStatusBar();
    });
    m_timer->start(16);
}

void MainWindow::setMobilePreview(bool mobile) {
    m_isMobilePreview = mobile;
    if (mobile) {
        setWindowTitle("Chrome Dino [Mobile Touch Preview] - Tap/Click to Jump, Drag to Duck");
        resize(960, 420);
        if (m_toolbar) {
            m_toolbar->hide();
        }
    } else {
        setWindowTitle("Computer Graphics Lab: Chrome Dino (Pure Software Raster Engine)");
        resize(1200, 480);
        if (m_toolbar) {
            m_toolbar->show();
        }
    }
}

void MainWindow::updateStatusBar() {
    // 1. Synchronize Toolbar Checked Toggles
    if (m_actPlay) m_actPlay->setChecked(!m_canvas->isShowcaseActive());
    if (m_actShowcase) m_actShowcase->setChecked(m_canvas->isShowcaseActive());
    if (m_actInspector) m_actInspector->setChecked(m_canvas->inspector().isActive());
    if (m_actVignette) m_actVignette->setChecked(m_canvas->game().isVignetteActive());
    if (m_actAA) m_actAA->setChecked(m_canvas->game().isAntiAliasing());
    if (m_actBgm) m_actBgm->setChecked(m_canvas->game().isBgmActive());
    if (m_actScanlines) m_actScanlines->setChecked(m_canvas->game().isScanlinesActive());
    if (m_actTurbo) m_actTurbo->setChecked(m_canvas->game().isTurboMode());
    if (m_actLeaderboard) m_actLeaderboard->setChecked(m_canvas->game().isLeaderboardOpen());
    if (m_actMute) m_actMute->setChecked(m_canvas->game().isMuted());

    // In Mobile Preview, show mobile touch instructions
    if (m_isMobilePreview && !m_canvas->isShowcaseActive()) {
        m_statusLabel->setText(
            "<span style='color:#00E5FF; font-weight:bold;'>[📱 MOBILE PREVIEW]</span> "
            "<span style='color:#546E7A;'>|</span> "
            "<b style='color:#FFD700;'>Tap / Click: Jump</b> "
            "<span style='color:#546E7A;'>|</span> "
            "<b style='color:#00FFCC;'>Swipe Down / Left Pad: Duck</b> "
            "<span style='color:#546E7A;'>|</span> "
            "<span style='color:#80D8FF;'>Tap [SET] Top-Right for Menu</span>"
        );
        return;
    }

    // 2. Render Rich-Text Status Bar
    if (m_canvas->isShowcaseActive()) {
        const char* modeName = "Unknown";
        switch (m_canvas->showcase().mode()) {
            case ShowcaseMode::BresenhamLine: modeName = "Bresenham's Integer Line Algorithm (All 8 Octants)"; break;
            case ShowcaseMode::MidpointCircle: modeName = "Midpoint Circle Algorithm (8-Way Symmetry Proof)"; break;
            case ShowcaseMode::MidpointEllipse: modeName = "Midpoint Ellipse Algorithm (Dual Region Proof)"; break;
            case ShowcaseMode::ScanlinePolygon: modeName = "Scanline Polygon Fill Algorithm (Edge Table Parity)"; break;
            case ShowcaseMode::SpriteBlit: modeName = "Sprite Bitmap Blitting & Nearest-Neighbor Resampling"; break;
            case ShowcaseMode::XiaolinWuLine: modeName = "Xiaolin Wu Anti-Aliased Line vs Bresenham Comparison"; break;
            case ShowcaseMode::AffineRotation: modeName = "2D Affine Transformation (Software Raster Inverse Rotation)"; break;
        }
        m_statusLabel->setText(QString("<span style='color:#00E5FF; font-weight:bold;'>[CG LAB SHOWCASE]</span> "
                                       "<b style='color:#FFD700;'>%1</b>  "
                                       "<span style='color:#78909C;'>|</span>  "
                                       "<span style='color:#B0BEC5;'>[LEFT/RIGHT] Next Demo  |  [F2/TAB] Back to Game</span>")
                               .arg(modeName));
    } else {
        const char* palName = m_canvas->game().paletteModeName();
        const char* weatherName = m_canvas->game().weatherName();
        bool aaOn = m_canvas->game().isAntiAliasing();
        bool vOn = m_canvas->game().isVignetteActive();
        bool bgmOn = m_canvas->game().isBgmActive();
        bool crtOn = m_canvas->game().isScanlinesActive();
        bool turboOn = m_canvas->game().isTurboMode();

        QString html = QString(
            "<span style='color:#00E5FF; font-weight:bold;'>[%1]</span> "
            "<span style='color:#546E7A;'>|</span> "
            "Weather: <b style='color:#80D8FF;'>%2</b> "
            "<span style='color:#546E7A;'>|</span> "
            "Line: <b style='color:%3;'>%4</b> "
            "<span style='color:#546E7A;'>|</span> "
            "Lantern: <b style='color:%5;'>%6</b> "
            "<span style='color:#546E7A;'>|</span> "
            "BGM: <b style='color:%7;'>%8</b> "
            "<span style='color:#546E7A;'>|</span> "
            "CRT: <b style='color:%9;'>%10</b> "
            "<span style='color:#546E7A;'>|</span> "
            "Palette: <b style='color:#FFD700;'>%11</b> "
            "<span style='color:#546E7A;'>|</span> "
            "<span style='color:#90A4AE;'>[W] Wx  [V] Lnt  [A] AA  [B] BGM  [P] Pal</span>"
        ).arg(
            turboOn ? "DINO TURBO" : "DINO",
            weatherName,
            aaOn ? "#00E5FF" : "#B0BEC5",
            aaOn ? "Xiaolin Wu (AA ON)" : "Bresenham (Aliased)",
            vOn ? "#FFD700" : "#78909C",
            vOn ? "ON" : "OFF",
            bgmOn ? "#00FF88" : "#78909C",
            bgmOn ? "ON" : "OFF",
            crtOn ? "#B388FF" : "#78909C",
            crtOn ? "ON" : "OFF",
            palName
        );
        m_statusLabel->setText(html);
    }
}
