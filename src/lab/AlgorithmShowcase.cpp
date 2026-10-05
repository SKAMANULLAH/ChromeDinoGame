#include "AlgorithmShowcase.h"
#include "../graphics/SpriteData.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

AlgorithmShowcase::AlgorithmShowcase() {
    // Default polygon vertices (star / concave polygon)
    m_polygonVertices = {
        QPoint(400, 40),
        QPoint(460, 110),
        QPoint(550, 120),
        QPoint(480, 180),
        QPoint(500, 260),
        QPoint(400, 220),
        QPoint(300, 260),
        QPoint(320, 180),
        QPoint(250, 120),
        QPoint(340, 110)
    };
    m_activeScanlineY = 40;
}

void AlgorithmShowcase::nextMode() {
    int cur = static_cast<int>(m_mode);
    cur = (cur + 1) % 7;
    m_mode = static_cast<ShowcaseMode>(cur);
}

void AlgorithmShowcase::prevMode() {
    int cur = static_cast<int>(m_mode);
    cur = (cur - 1 + 7) % 7;
    m_mode = static_cast<ShowcaseMode>(cur);
}

void AlgorithmShowcase::update(float dt) {
    m_animTimer += dt;
    
    if (m_mode == ShowcaseMode::ScanlinePolygon) {
        m_activeScanlineY = 40 + static_cast<int>(std::fmod(m_animTimer * 60.0f, 230.0f));
    } else if (m_mode == ShowcaseMode::AffineRotation) {
        m_rotationAngle += dt * 45.0f;
        if (m_rotationAngle >= 360.0f) {
            m_rotationAngle -= 360.0f;
        }
    }
}

void AlgorithmShowcase::handleMousePress(int x, int y) {
    if (m_mode == ShowcaseMode::BresenhamLine || m_mode == ShowcaseMode::XiaolinWuLine) {
        if (std::hypot(x - m_lineP0.x(), y - m_lineP0.y()) < 15) {
            m_draggingPoint = 0;
        } else if (std::hypot(x - m_lineP1.x(), y - m_lineP1.y()) < 15) {
            m_draggingPoint = 1;
        }
    } else if (m_mode == ShowcaseMode::MidpointCircle) {
        if (std::hypot(x - (m_circleCenter.x() + m_circleRadius), y - m_circleCenter.y()) < 15) {
            m_draggingPoint = 1; // Radius handle
        } else {
            m_circleCenter = QPoint(x, y);
            m_draggingPoint = 0;
        }
    } else if (m_mode == ShowcaseMode::MidpointEllipse) {
        if (std::hypot(x - (m_ellipseCenter.x() + m_ellipseRx), y - m_ellipseCenter.y()) < 15) {
            m_draggingPoint = 1; // Rx handle
        } else if (std::hypot(x - m_ellipseCenter.x(), y - (m_ellipseCenter.y() + m_ellipseRy)) < 15) {
            m_draggingPoint = 2; // Ry handle
        } else {
            m_ellipseCenter = QPoint(x, y);
            m_draggingPoint = 0;
        }
    } else if (m_mode == ShowcaseMode::AffineRotation) {
        m_draggingPoint = 0;
        m_rotationAngle = static_cast<float>(x % 360);
    }
}

void AlgorithmShowcase::handleMouseMove(int x, int y) {
    if (m_draggingPoint == -1) return;

    if (m_mode == ShowcaseMode::BresenhamLine || m_mode == ShowcaseMode::XiaolinWuLine) {
        if (m_draggingPoint == 0) m_lineP0 = QPoint(x, y);
        else if (m_draggingPoint == 1) m_lineP1 = QPoint(x, y);
    } else if (m_mode == ShowcaseMode::MidpointCircle) {
        if (m_draggingPoint == 1) {
            m_circleRadius = std::clamp(static_cast<int>(std::hypot(x - m_circleCenter.x(), y - m_circleCenter.y())), 10, 140);
        } else if (m_draggingPoint == 0) {
            m_circleCenter = QPoint(x, y);
        }
    } else if (m_mode == ShowcaseMode::MidpointEllipse) {
        if (std::hypot(x - m_ellipseCenter.x(), y - m_ellipseCenter.y()) > 1) {
            if (m_draggingPoint == 1) {
                m_ellipseRx = std::clamp(std::abs(x - m_ellipseCenter.x()), 20, 250);
            } else if (m_draggingPoint == 2) {
                m_ellipseRy = std::clamp(std::abs(y - m_ellipseCenter.y()), 15, 120);
            } else if (m_draggingPoint == 0) {
                m_ellipseCenter = QPoint(x, y);
            }
        }
    } else if (m_mode == ShowcaseMode::AffineRotation) {
        m_rotationAngle = static_cast<float>(x % 360);
    }
}

void AlgorithmShowcase::handleMouseRelease(int /*x*/, int /*y*/) {
    m_draggingPoint = -1;
}

void AlgorithmShowcase::render(SoftwareRasterizer& rasterizer) {
    // Clear background to dark lab slate
    rasterizer.framebuffer().clear(0xFF1E222B);

    // Header Navigation Bar
    rasterizer.fillRectScanline(0, 0, rasterizer.framebuffer().width(), 26, 0xFF14171E);
    rasterizer.drawLineBresenham(0, 26, rasterizer.framebuffer().width() - 1, 26, 0xFF00E5FF);

    rasterizer.drawTextBitmap(10, 8, "CG LAB ALGORITHM SHOWCASE", 0xFF00E5FF, 1);
    rasterizer.drawTextBitmap(240, 8, "[TAB / F2] TOGGLE GAME | [LEFT/RIGHT] CHANGE ALGORITHM", 0xFF99AAB5, 1);

    // Mode-specific render
    switch (m_mode) {
        case ShowcaseMode::BresenhamLine:
            renderBresenhamLineDemo(rasterizer);
            break;
        case ShowcaseMode::MidpointCircle:
            renderMidpointCircleDemo(rasterizer);
            break;
        case ShowcaseMode::MidpointEllipse:
            renderMidpointEllipseDemo(rasterizer);
            break;
        case ShowcaseMode::ScanlinePolygon:
            renderScanlinePolygonDemo(rasterizer);
            break;
        case ShowcaseMode::SpriteBlit:
            renderSpriteBlitDemo(rasterizer);
            break;
        case ShowcaseMode::XiaolinWuLine:
            renderXiaolinWuDemo(rasterizer);
            break;
        case ShowcaseMode::AffineRotation:
            renderAffineRotationDemo(rasterizer);
            break;
    }
}

void AlgorithmShowcase::renderBresenhamLineDemo(SoftwareRasterizer& rasterizer) {
    rasterizer.drawTextBitmap(10, 36, "1. BRESENHAM'S LINE ALGORITHM (ALL 8 OCTANTS, INTEGER MATH)", 0xFFFFFFFF, 1);
    rasterizer.drawTextBitmap(10, 48, "Drag handles with mouse to test arbitrary angles and slopes.", 0xFF888888, 1);

    int dx = std::abs(m_lineP1.x() - m_lineP0.x());
    int dy = std::abs(m_lineP1.y() - m_lineP0.y());
    int sx = (m_lineP0.x() < m_lineP1.x()) ? 1 : -1;
    int sy = (m_lineP0.y() < m_lineP1.y()) ? 1 : -1;
    float slope = (dx != 0) ? static_cast<float>(m_lineP1.y() - m_lineP0.y()) / (m_lineP1.x() - m_lineP0.x()) : 9999.0f;

    char info[128];
    std::snprintf(info, sizeof(info), "P0=(%d,%d) P1=(%d,%d) | dx=%d dy=%d slope=%.3f | sx=%d sy=%d | err0=%d",
                  m_lineP0.x(), m_lineP0.y(), m_lineP1.x(), m_lineP1.y(), dx, dy, slope, sx, sy, dx - dy);
    rasterizer.drawTextBitmap(10, 62, info, 0xFF00FFCC, 1);

    // Draw coordinate axes
    rasterizer.drawDottedLineBresenham(m_lineP0.x(), 30, m_lineP0.x(), 280, 0xFF2F3542, 4);
    rasterizer.drawDottedLineBresenham(30, m_lineP0.y(), 770, m_lineP0.y(), 0xFF2F3542, 4);

    // Draw the Bresenham line
    rasterizer.drawThickLineBresenham(m_lineP0.x(), m_lineP0.y(), m_lineP1.x(), m_lineP1.y(), 3, 0xFFFF3333);

    // Draw handles using Midpoint Circles
    rasterizer.fillCircleMidpoint(m_lineP0.x(), m_lineP0.y(), 6, 0xFF00E5FF);
    rasterizer.drawCircleMidpoint(m_lineP0.x(), m_lineP0.y(), 7, 0xFFFFFFFF);
    rasterizer.drawTextBitmap(m_lineP0.x() + 9, m_lineP0.y() - 4, "P0", 0xFF00E5FF, 1);

    rasterizer.fillCircleMidpoint(m_lineP1.x(), m_lineP1.y(), 6, 0xFFFF0077);
    rasterizer.drawCircleMidpoint(m_lineP1.x(), m_lineP1.y(), 7, 0xFFFFFFFF);
    rasterizer.drawTextBitmap(m_lineP1.x() + 9, m_lineP1.y() - 4, "P1", 0xFFFF0077, 1);
}

void AlgorithmShowcase::renderMidpointCircleDemo(SoftwareRasterizer& rasterizer) {
    rasterizer.drawTextBitmap(10, 36, "2. MIDPOINT CIRCLE ALGORITHM (8-WAY SYMMETRY PROOF)", 0xFFFFFFFF, 1);
    rasterizer.drawTextBitmap(10, 48, "Each color shows one of the 8 symmetric octants plotted simultaneously!", 0xFF888888, 1);

    char info[128];
    std::snprintf(info, sizeof(info), "Center=(%d,%d) Radius=%d | Initial Decision Variable P0 = 1 - R = %d",
                  m_circleCenter.x(), m_circleCenter.y(), m_circleRadius, 1 - m_circleRadius);
    rasterizer.drawTextBitmap(10, 62, info, 0xFF00FFCC, 1);

    // Plot circle with 8 distinct octant colors to visually prove 8-way symmetry!
    int xc = m_circleCenter.x();
    int yc = m_circleCenter.y();
    int r = m_circleRadius;

    int x = 0;
    int y = r;
    int d = 1 - r;

    // 8 distinct vibrant colors for octants 1 through 8
    const uint32_t octantColors[8] = {
        0xFFFF3333, // 1: Red (+x, +y)
        0xFFFF9933, // 2: Orange (+y, +x)
        0xFFFFFF33, // 3: Yellow (-y, +x)
        0xFF33FF33, // 4: Green (-x, +y)
        0xFF33FFFF, // 5: Cyan (-x, -y)
        0xFF3399FF, // 6: Blue (-y, -x)
        0xFFFF33FF, // 7: Magenta (+y, -x)
        0xFFFFFFFF  // 8: White (+x, -y)
    };

    auto plot8Color = [&](int px, int py) {
        rasterizer.setPixel(xc + px, yc + py, octantColors[0]);
        rasterizer.setPixel(xc + py, yc + px, octantColors[1]);
        rasterizer.setPixel(xc - py, yc + px, octantColors[2]);
        rasterizer.setPixel(xc - px, yc + py, octantColors[3]);
        rasterizer.setPixel(xc - px, yc - py, octantColors[4]);
        rasterizer.setPixel(xc - py, yc - px, octantColors[5]);
        rasterizer.setPixel(xc + py, yc - px, octantColors[6]);
        rasterizer.setPixel(xc + px, yc - py, octantColors[7]);
    };

    plot8Color(x, y);

    while (x < y) {
        x++;
        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
        plot8Color(x, y);
    }

    // Draw octant dividing radial lines
    rasterizer.drawDottedLineBresenham(xc - r - 15, yc, xc + r + 15, yc, 0xFF555555, 4);
    rasterizer.drawDottedLineBresenham(xc, yc - r - 15, xc, yc + r + 15, 0xFF555555, 4);
    rasterizer.drawDottedLineBresenham(xc - r, yc - r, xc + r, yc + r, 0xFF444444, 4);
    rasterizer.drawDottedLineBresenham(xc - r, yc + r, xc + r, yc - r, 0xFF444444, 4);

    // Center handle & Radius handle
    rasterizer.fillCircleMidpoint(xc, yc, 4, 0xFF00E5FF);
    rasterizer.fillCircleMidpoint(xc + r, yc, 5, 0xFFFF0077);
    rasterizer.drawTextBitmap(xc + r + 8, yc - 4, "R Handle", 0xFFFF0077, 1);

    // Legend
    int legX = 580;
    int legY = 100;
    const char* legNames[8] = {
        "Octant 1 (+x, +y)", "Octant 2 (+y, +x)", "Octant 3 (-y, +x)", "Octant 4 (-x, +y)",
        "Octant 5 (-x, -y)", "Octant 6 (-y, -x)", "Octant 7 (+y, -x)", "Octant 8 (+x, -y)"
    };
    for (int i = 0; i < 8; ++i) {
        rasterizer.fillRectScanline(legX, legY + i * 16, 10, 10, octantColors[i]);
        rasterizer.drawTextBitmap(legX + 16, legY + i * 16 + 2, legNames[i], 0xFFE0E0E0, 1);
    }
}

void AlgorithmShowcase::renderMidpointEllipseDemo(SoftwareRasterizer& rasterizer) {
    rasterizer.drawTextBitmap(10, 36, "3. MIDPOINT ELLIPSE ALGORITHM (REGION 1 vs REGION 2)", 0xFFFFFFFF, 1);
    rasterizer.drawTextBitmap(10, 48, "Lime Green = Region 1 (|slope| < 1)  |  Cyan = Region 2 (|slope| >= 1)", 0xFF888888, 1);

    char info[128];
    std::snprintf(info, sizeof(info), "Center=(%d,%d) Rx=%d Ry=%d | Transition Condition: 2*Ry^2*x >= 2*Rx^2*y",
                  m_ellipseCenter.x(), m_ellipseCenter.y(), m_ellipseRx, m_ellipseRy);
    rasterizer.drawTextBitmap(10, 62, info, 0xFF00FFCC, 1);

    int xc = m_ellipseCenter.x();
    int yc = m_ellipseCenter.y();
    long long a = m_ellipseRx;
    long long b = m_ellipseRy;
    long long a2 = a * a;
    long long b2 = b * b;

    long long x = 0;
    long long y = b;

    // Region 1 (Lime Green)
    double p1 = b2 - (a2 * b) + (0.25 * a2);
    long long dx = 2 * b2 * x;
    long long dy = 2 * a2 * y;

    auto plot4 = [&](int px, int py, uint32_t col) {
        rasterizer.setPixel(xc + px, yc + py, col);
        rasterizer.setPixel(xc - px, yc + py, col);
        rasterizer.setPixel(xc + px, yc - py, col);
        rasterizer.setPixel(xc - px, yc - py, col);
    };

    plot4(x, y, 0xFF00FF66);

    while (dx < dy) {
        x++;
        dx += 2 * b2;
        if (p1 < 0) {
            p1 += dx + b2;
        } else {
            y--;
            dy -= 2 * a2;
            p1 += dx - dy + b2;
        }
        plot4(x, y, 0xFF00FF66); // Region 1
    }

    // Region 2 (Cyan)
    double p2 = (b2 * (x + 0.5) * (x + 0.5)) + (a2 * (y - 1) * (y - 1)) - (a2 * b2);
    while (y > 0) {
        y--;
        dy -= 2 * a2;
        if (p2 > 0) {
            p2 += a2 - dy;
        } else {
            x++;
            dx += 2 * b2;
            p2 += dx - dy + a2;
        }
        plot4(x, y, 0xFF00E5FF); // Region 2
    }

    // Handles
    rasterizer.fillCircleMidpoint(xc + a, yc, 5, 0xFFFF0077);
    rasterizer.drawTextBitmap(xc + a + 8, yc - 4, "Rx Handle", 0xFFFF0077, 1);

    rasterizer.fillCircleMidpoint(xc, yc + b, 5, 0xFFFFCC00);
    rasterizer.drawTextBitmap(xc + 8, yc + b - 4, "Ry Handle", 0xFFFFCC00, 1);
}

void AlgorithmShowcase::renderScanlinePolygonDemo(SoftwareRasterizer& rasterizer) {
    rasterizer.drawTextBitmap(10, 36, "4. SCANLINE POLYGON FILL ALGORITHM (EDGE TABLE & PARITY SPANS)", 0xFFFFFFFF, 1);
    rasterizer.drawTextBitmap(10, 48, "Demonstrates animated scanline intersecting edges and filling spans.", 0xFF888888, 1);

    // Draw full filled polygon in semi-transparent / dim color
    rasterizer.fillPolygonScanline(m_polygonVertices, 0xFF2A4365);

    // Draw polygon edges using Bresenham lines
    size_t n = m_polygonVertices.size();
    for (size_t i = 0; i < n; ++i) {
        size_t next = (i + 1) % n;
        rasterizer.drawLineBresenham(m_polygonVertices[i].x(), m_polygonVertices[i].y(),
                                    m_polygonVertices[next].x(), m_polygonVertices[next].y(), 0xFF00E5FF);
        rasterizer.fillCircleMidpoint(m_polygonVertices[i].x(), m_polygonVertices[i].y(), 3, 0xFFFFFFFF);
    }

    // Show active animated scanline
    int curY = m_activeScanlineY;
    rasterizer.drawLineBresenham(100, curY, 700, curY, 0xFFFF0055);

    // Find and highlight intersection nodes on this scanline
    std::vector<int> nodes;
    for (size_t i = 0; i < n; ++i) {
        size_t next = (i + 1) % n;
        int y0 = m_polygonVertices[i].y();
        int y1 = m_polygonVertices[next].y();
        int x0 = m_polygonVertices[i].x();
        int x1 = m_polygonVertices[next].x();
        if ((y0 < curY && y1 >= curY) || (y1 < curY && y0 >= curY)) {
            int ix = x0 + static_cast<int>(std::round((static_cast<double>(curY - y0) / (y1 - y0)) * (x1 - x0)));
            nodes.push_back(ix);
            rasterizer.fillCircleMidpoint(ix, curY, 4, 0xFFFFFF00); // Yellow intersection dot
        }
    }
    std::sort(nodes.begin(), nodes.end());

    // Highlight the active span being filled in bright green
    for (size_t i = 0; i + 1 < nodes.size(); i += 2) {
        rasterizer.drawLineBresenham(nodes[i], curY, nodes[i + 1], curY, 0xFF00FF00);
    }

    char scanInfo[128];
    std::snprintf(scanInfo, sizeof(scanInfo), "Current Scanline Y = %d | Intersections Found = %zu (Parity Pairs = %zu)",
                  curY, nodes.size(), nodes.size() / 2);
    rasterizer.drawTextBitmap(10, 62, scanInfo, 0xFF00FFCC, 1);
}

void AlgorithmShowcase::renderSpriteBlitDemo(SoftwareRasterizer& rasterizer) {
    rasterizer.drawTextBitmap(10, 36, "5. SPRITE BITMAP BLITTING & NEAREST-NEIGHBOR RASTER SCALING", 0xFFFFFFFF, 1);
    rasterizer.drawTextBitmap(10, 48, "Direct pixel copy, Colorkey Transparency (0x00000000), Alpha Compositing.", 0xFF888888, 1);

    // Blit Dino at 1x, 2x, 3x scale
    rasterizer.drawTextBitmap(60, 80, "1X NATIVE", 0xFFFFFFFF, 1);
    rasterizer.blitSprite(SpriteData::dinoRun1(), 60, 100);

    rasterizer.drawTextBitmap(160, 80, "2X SCALED", 0xFFFFFFFF, 1);
    rasterizer.blitSpriteScaled(SpriteData::dinoRun2(), 160, 100, 2, 2);

    rasterizer.drawTextBitmap(300, 80, "3X SCALED", 0xFFFFFFFF, 1);
    rasterizer.blitSpriteScaled(SpriteData::dinoDuck1(), 300, 120, 3, 3);

    // Alpha Blending Showcase
    rasterizer.drawTextBitmap(540, 80, "ALPHA BLENDING (50%)", 0xFFFFFFFF, 1);
    // Draw background stripes
    for (int y = 100; y < 190; y += 10) {
        rasterizer.fillRectScanline(540, y, 160, 5, 0xFF00E5FF);
    }
    rasterizer.blitSpriteScaled(SpriteData::cactusBig2(), 560, 100, 2, 2, false);
}

void AlgorithmShowcase::renderXiaolinWuDemo(SoftwareRasterizer& rasterizer) {
    rasterizer.drawTextBitmap(10, 36, "6. XIAOLIN WU'S ANTI-ALIASED LINE ALGORITHM (SUB-PIXEL WEIGHTING)", 0xFFFFFFFF, 1);
    rasterizer.drawTextBitmap(10, 48, "Top: Bresenham Line (Jagged / Aliased)  vs  Bottom: Xiaolin Wu (Anti-Aliased)", 0xFF888888, 1);

    int dx = std::abs(m_lineP1.x() - m_lineP0.x());
    int dy = std::abs(m_lineP1.y() - m_lineP0.y());
    float slope = (dx != 0) ? static_cast<float>(m_lineP1.y() - m_lineP0.y()) / (m_lineP1.x() - m_lineP0.x()) : 9999.0f;

    char info[128];
    std::snprintf(info, sizeof(info), "P0=(%d,%d) P1=(%d,%d) | slope=%.3f | Fractional alpha weighting: (1-fpart) and fpart",
                  m_lineP0.x(), m_lineP0.y(), m_lineP1.x(), m_lineP1.y(), slope);
    rasterizer.drawTextBitmap(10, 62, info, 0xFF00FFCC, 1);

    // Separator grid line
    rasterizer.drawDottedLineBresenham(30, 165, 620, 165, 0xFF2F3542, 4);

    // 1. Top half: Bresenham Line (Aliased)
    rasterizer.drawTextBitmap(15, 82, "[ALIASED - BRESENHAM'S INTEGER LINE]", 0xFFFF6666, 1);
    rasterizer.drawLineBresenham(m_lineP0.x(), m_lineP0.y() - 40, m_lineP1.x(), m_lineP1.y() - 40, 0xFFFF3333);

    // 2. Bottom half: Xiaolin Wu Line (Anti-Aliased)
    rasterizer.drawTextBitmap(15, 178, "[ANTI-ALIASED - XIAOLIN WU'S WEIGHTED LINE]", 0xFF00FFCC, 1);
    rasterizer.drawLineXiaolinWu(m_lineP0.x(), m_lineP0.y() + 50, m_lineP1.x(), m_lineP1.y() + 50, 0xFF00FFCC);

    // Draw handles on Xiaolin Wu line
    rasterizer.fillCircleMidpoint(m_lineP0.x(), m_lineP0.y() + 50, 6, 0xFF00E5FF);
    rasterizer.drawCircleMidpoint(m_lineP0.x(), m_lineP0.y() + 50, 7, 0xFFFFFFFF);
    rasterizer.drawTextBitmap(m_lineP0.x() + 9, m_lineP0.y() + 46, "P0", 0xFF00E5FF, 1);

    rasterizer.fillCircleMidpoint(m_lineP1.x(), m_lineP1.y() + 50, 6, 0xFFFF0077);
    rasterizer.drawCircleMidpoint(m_lineP1.x(), m_lineP1.y() + 50, 7, 0xFFFFFFFF);
    rasterizer.drawTextBitmap(m_lineP1.x() + 9, m_lineP1.y() + 46, "P1", 0xFFFF0077, 1);

    // Zoom comparison box on the right
    int zoomX = 640;
    int zoomY = 75;
    int zoomSize = 8;
    int cellW = 14;

    rasterizer.drawRectBresenham(zoomX - 2, zoomY - 2, zoomSize * cellW + 4, zoomSize * cellW + 4, 0xFF555555);
    rasterizer.drawTextBitmap(zoomX, zoomY - 12, "PIXEL GRID INSPECTOR", 0xFFE0E0E0, 1);

    // Sample from the middle of Xiaolin Wu line
    int midX = (m_lineP0.x() + m_lineP1.x()) / 2;
    int midY = (m_lineP0.y() + m_lineP1.y()) / 2 + 50;

    for (int gy = 0; gy < zoomSize; ++gy) {
        for (int gx = 0; gx < zoomSize; ++gx) {
            int sx = midX - zoomSize / 2 + gx;
            int sy = midY - zoomSize / 2 + gy;
            uint32_t sampleCol = rasterizer.getPixel(sx, sy);
            rasterizer.fillRectScanline(zoomX + gx * cellW, zoomY + gy * cellW, cellW - 1, cellW - 1, sampleCol);
        }
    }
}

void AlgorithmShowcase::renderAffineRotationDemo(SoftwareRasterizer& rasterizer) {
    rasterizer.drawTextBitmap(10, 36, "7. 2D AFFINE TRANSFORMATION: SOFTWARE RASTER ROTATION (INVERSE MAPPING)", 0xFFFFFFFF, 1);
    rasterizer.drawTextBitmap(10, 48, "Inverse Mapping: [x_src, y_src]^T = R(-theta) * [x_dest - cx, y_dest - cy]^T + [cx, cy]^T", 0xFF888888, 1);

    char info[128];
    float rad = m_rotationAngle * 3.14159265f / 180.0f;
    std::snprintf(info, sizeof(info), "Rotation Angle = %.1f deg (%.3f rad) | Cos(theta)=%.3f, Sin(theta)=%.3f | Drag mouse to rotate!",
                  m_rotationAngle, rad, std::cos(rad), std::sin(rad));
    rasterizer.drawTextBitmap(10, 62, info, 0xFF00FFCC, 1);

    // Draw coordinate axes at center
    int cx1 = 180;
    int cy1 = 175;
    rasterizer.drawDottedLineBresenham(cx1 - 70, cy1, cx1 + 70, cy1, 0xFF3A3F4B, 3);
    rasterizer.drawDottedLineBresenham(cx1, cy1 - 70, cx1, cy1 + 70, 0xFF3A3F4B, 3);

    rasterizer.drawTextBitmap(cx1 - 50, cy1 - 65, "RUNNING DINO ROTATED", 0xFFFFFFFF, 1);
    rasterizer.blitSpriteRotated(SpriteData::dinoRun1(), cx1, cy1, m_rotationAngle);

    // Draw 2nd sprite: Pterodactyl flying at tilt
    int cx2 = 390;
    int cy2 = 175;
    rasterizer.drawDottedLineBresenham(cx2 - 70, cy2, cx2 + 70, cy2, 0xFF3A3F4B, 3);
    rasterizer.drawDottedLineBresenham(cx2, cy2 - 70, cx2, cy2 + 70, 0xFF3A3F4B, 3);

    rasterizer.drawTextBitmap(cx2 - 50, cy2 - 65, "PTERODACTYL ROTATED", 0xFFFFFFFF, 1);
    rasterizer.blitSpriteRotated(SpriteData::pteroWingUp(), cx2, cy2, -m_rotationAngle);

    // Mathematical Matrix Formula Panel on Right
    int panX = 540;
    int panY = 85;
    int panW = 240;
    int panH = 185;

    for (int y = panY; y < panY + panH; ++y) {
        for (int x = panX; x < panX + panW; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xEE14161C);
        }
    }
    rasterizer.drawRectBresenham(panX, panY, panW, panH, 0xFF00E5FF);

    rasterizer.drawTextBitmap(panX + 8, panY + 10, "INVERSE ROTATION MATRIX", 0xFF00E5FF, 1);
    rasterizer.drawTextBitmap(panX + 8, panY + 28, "|  cos(-th)  -sin(-th) |", 0xFFE0E0E0, 1);
    rasterizer.drawTextBitmap(panX + 8, panY + 42, "|  sin(-th)   cos(-th) |", 0xFFE0E0E0, 1);

    rasterizer.drawTextBitmap(panX + 8, panY + 65, "PREVENTS HOLES & GAPS:", 0xFFFFD700, 1);
    rasterizer.drawTextBitmap(panX + 8, panY + 80, "Forward mapping creates", 0xFF999999, 1);
    rasterizer.drawTextBitmap(panX + 8, panY + 92, "moiré holes due to rounding.", 0xFF999999, 1);
    rasterizer.drawTextBitmap(panX + 8, panY + 110, "Inverse mapping iterates", 0xFF33FF33, 1);
    rasterizer.drawTextBitmap(panX + 8, panY + 122, "each dest pixel and samples", 0xFF33FF33, 1);
    rasterizer.drawTextBitmap(panX + 8, panY + 134, "the source buffer!", 0xFF33FF33, 1);
    rasterizer.drawTextBitmap(panX + 8, panY + 155, "NO VECTOR PIPELINES!", 0xFFFF0077, 1);
}
