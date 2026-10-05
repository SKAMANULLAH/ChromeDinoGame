#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>
#include "graphics/Framebuffer.h"
#include "graphics/SoftwareRasterizer.h"
#include "game/DinoGame.h"
#include "lab/LabInspector.h"
#include "lab/AlgorithmShowcase.h"

class GameCanvas : public QWidget {
    Q_OBJECT
public:
    explicit GameCanvas(QWidget* parent = nullptr);

    DinoGame& game() { return m_game; }
    LabInspector& inspector() { return m_inspector; }
    AlgorithmShowcase& showcase() { return m_showcase; }
    bool isShowcaseActive() const { return m_inShowcase; }
    void toggleShowcase() { m_inShowcase = !m_inShowcase; }

    void gameLoop();

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    bool event(QEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    static constexpr int NATIVE_WIDTH = 800;
    static constexpr int NATIVE_HEIGHT = 300;

    Framebuffer m_framebuffer;
    SoftwareRasterizer m_rasterizer;
    DinoGame m_game;
    LabInspector m_inspector;
    AlgorithmShowcase m_showcase;

    bool m_inShowcase{false};

    // Mobile touch & gesture tracking
    QPoint m_touchStartPos;
    bool m_touchActive{false};
    bool m_isSwipingDown{false};

    QElapsedTimer m_frameTimer;
    float m_fps{60.0f};
    float m_frameTimeMs{16.6f};
    int m_frameCount{0};
    float m_fpsAccumulator{0.0f};

    QPoint mapToNative(const QPoint& screenPos) const;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    void setMobilePreview(bool mobile);

private slots:
    void updateStatusBar();

private:
    GameCanvas* m_canvas{nullptr};
    QTimer* m_timer{nullptr};
    QLabel* m_statusLabel{nullptr};
    class QToolBar* m_toolbar{nullptr};
    bool m_isMobilePreview{false};

    // Toolbar Action Pointers for live toggle synchronization
    class QAction* m_actPlay{nullptr};
    class QAction* m_actShowcase{nullptr};
    class QAction* m_actInspector{nullptr};
    class QAction* m_actWeather{nullptr};
    class QAction* m_actVignette{nullptr};
    class QAction* m_actAA{nullptr};
    class QAction* m_actBgm{nullptr};
    class QAction* m_actScanlines{nullptr};
    class QAction* m_actPalette{nullptr};
    class QAction* m_actTurbo{nullptr};
    class QAction* m_actLeaderboard{nullptr};
    class QAction* m_actMute{nullptr};
};

#endif // MAINWINDOW_H
