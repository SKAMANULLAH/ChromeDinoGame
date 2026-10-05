#ifndef ALGORITHMSHOWCASE_H
#define ALGORITHMSHOWCASE_H

#include "../graphics/SoftwareRasterizer.h"
#include <QPoint>

enum class ShowcaseMode {
    BresenhamLine,
    MidpointCircle,
    MidpointEllipse,
    ScanlinePolygon,
    SpriteBlit,
    XiaolinWuLine,
    AffineRotation
};

/**
 * @file AlgorithmShowcase.h
 * @brief Interactive Sandbox for demonstrating isolated CG lab algorithms to the examiner.
 */
class AlgorithmShowcase {
public:
    AlgorithmShowcase();

    void update(float dt);
    void render(SoftwareRasterizer& rasterizer);

    void nextMode();
    void prevMode();
    void setMode(ShowcaseMode mode) { m_mode = mode; }
    ShowcaseMode mode() const { return m_mode; }

    // Mouse interaction for examiner testing
    void handleMousePress(int x, int y);
    void handleMouseMove(int x, int y);
    void handleMouseRelease(int x, int y);

private:
    ShowcaseMode m_mode{ShowcaseMode::BresenhamLine};
    float m_animTimer{0.0f};

    // Mode 1: Bresenham Line & Mode 6: Xiaolin Wu
    QPoint m_lineP0{120, 180};
    QPoint m_lineP1{680, 80};
    int m_draggingPoint{-1};

    // Mode 2: Midpoint Circle
    QPoint m_circleCenter{400, 150};
    int m_circleRadius{80};

    // Mode 3: Midpoint Ellipse
    QPoint m_ellipseCenter{400, 150};
    int m_ellipseRx{140};
    int m_ellipseRy{70};

    // Mode 4: Scanline Polygon
    std::vector<QPoint> m_polygonVertices;
    int m_activeScanlineY{0};

    // Mode 7: Affine Rotation
    float m_rotationAngle{0.0f};

    void renderBresenhamLineDemo(SoftwareRasterizer& rasterizer);
    void renderMidpointCircleDemo(SoftwareRasterizer& rasterizer);
    void renderMidpointEllipseDemo(SoftwareRasterizer& rasterizer);
    void renderScanlinePolygonDemo(SoftwareRasterizer& rasterizer);
    void renderSpriteBlitDemo(SoftwareRasterizer& rasterizer);
    void renderXiaolinWuDemo(SoftwareRasterizer& rasterizer);
    void renderAffineRotationDemo(SoftwareRasterizer& rasterizer);
};

#endif // ALGORITHMSHOWCASE_H
