#include "SoftwareRasterizer.h"
#include "Font5x7.h"
#include <cmath>
#include <algorithm>

SoftwareRasterizer::SoftwareRasterizer(Framebuffer& framebuffer)
    : m_fb(framebuffer)
{
}

void SoftwareRasterizer::setPixel(int x, int y, uint32_t color) {
    uint8_t a = (color >> 24) & 0xFF;
    if (a == 255) {
        m_fb.setPixel(x, y, color);
    } else if (a > 0) {
        m_fb.setPixelBlend(x, y, color);
    }
    m_telemetry.totalPixels++;
}

/**
 * @brief Bresenham's Integer Line Algorithm.
 * 
 * Textbook derivation:
 * Given endpoints (x0, y0) and (x1, y1), slope m = dy / dx.
 * To avoid floating point math, Bresenham defines an integer decision error:
 *   err = dx - dy
 * At each step, based on e2 = 2 * err, we decide whether to step along x, y, or both.
 * This guarantees exact integer calculation for all 8 octants.
 */
void SoftwareRasterizer::drawLineBresenham(int x0, int y0, int x1, int y1, uint32_t color) {
    m_telemetry.bresenhamLines++;
    if (m_inspectorHighlight) color = 0xFFFF3333; // Bright Red in inspector mode

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        setPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void SoftwareRasterizer::drawDottedLineBresenham(int x0, int y0, int x1, int y1, uint32_t color, int dotPeriod) {
    m_telemetry.bresenhamLines++;
    if (m_inspectorHighlight) color = 0xFFFF6666;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int step = 0;

    while (true) {
        if ((step % dotPeriod) < (dotPeriod / 2)) {
            setPixel(x0, y0, color);
        }
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
        step++;
    }
}

void SoftwareRasterizer::drawThickLineBresenham(int x0, int y0, int x1, int y1, int thickness, uint32_t color) {
    if (thickness <= 1) {
        drawLineBresenham(x0, y0, x1, y1, color);
        return;
    }
    int half = thickness / 2;
    for (int off = -half; off <= half; ++off) {
        if (std::abs(x1 - x0) >= std::abs(y1 - y0)) {
            drawLineBresenham(x0, y0 + off, x1, y1 + off, color);
        } else {
            drawLineBresenham(x0 + off, y0, x1 + off, y1, color);
        }
    }
}

/**
 * @brief Xiaolin Wu's Anti-Aliased Line Algorithm.
 * 
 * Draws smooth lines with sub-pixel intensity weighting.
 * Decomposes coordinates into integer and fractional parts (ipart, fpart),
 * plotting pairs of adjacent pixels with complementary alpha blending weights (1 - fpart, fpart).
 */
void SoftwareRasterizer::drawLineXiaolinWu(int x0, int y0, int x1, int y1, uint32_t color) {
    if (x0 == x1 && y0 == y1) {
        setPixel(x0, y0, color);
        return;
    }

    m_telemetry.xiaolinWuLines++;
    if (m_inspectorHighlight) color = 0xFFFF00CC; // Highlight in magenta

    uint8_t ca, cr, cg, cb;
    Framebuffer::unpackARGB(color, ca, cr, cg, cb);

    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    float dx = static_cast<float>(x1 - x0);
    float dy = static_cast<float>(y1 - y0);
    float gradient = (dx == 0.0f) ? 1.0f : (dy / dx);

    auto plotWu = [&](int x, int y, float brightness) {
        if (brightness <= 0.0f) return;
        uint8_t finalA = static_cast<uint8_t>(std::clamp(ca * brightness, 0.0f, 255.0f));
        uint32_t blendedCol = Framebuffer::packARGB(finalA, cr, cg, cb);
        if (steep) {
            m_fb.setPixelBlend(y, x, blendedCol);
        } else {
            m_fb.setPixelBlend(x, y, blendedCol);
        }
        m_telemetry.totalPixels++;
    };

    // First endpoint
    float xend = std::round(static_cast<float>(x0));
    float yend = y0 + gradient * (xend - x0);
    float xgap = 1.0f - (static_cast<float>(x0) + 0.5f - std::floor(static_cast<float>(x0) + 0.5f));
    int xpxl1 = static_cast<int>(xend);
    int ypxl1 = static_cast<int>(std::floor(yend));
    plotWu(xpxl1, ypxl1, (1.0f - (yend - std::floor(yend))) * xgap);
    plotWu(xpxl1, ypxl1 + 1, (yend - std::floor(yend)) * xgap);
    float intery = yend + gradient;

    // Second endpoint
    xend = std::round(static_cast<float>(x1));
    yend = y1 + gradient * (xend - x1);
    xgap = static_cast<float>(x1) + 0.5f - std::floor(static_cast<float>(x1) + 0.5f);
    int xpxl2 = static_cast<int>(xend);
    int ypxl2 = static_cast<int>(std::floor(yend));
    plotWu(xpxl2, ypxl2, (1.0f - (yend - std::floor(yend))) * xgap);
    plotWu(xpxl2, ypxl2 + 1, (yend - std::floor(yend)) * xgap);

    // Main loop
    for (int x = xpxl1 + 1; x < xpxl2; ++x) {
        int ipart = static_cast<int>(std::floor(intery));
        float fpart = intery - ipart;
        plotWu(x, ipart, 1.0f - fpart);
        plotWu(x, ipart + 1, fpart);
        intery += gradient;
    }
}

void SoftwareRasterizer::drawLineAuto(int x0, int y0, int x1, int y1, uint32_t color) {
    if (m_useAntiAliasing) {
        drawLineXiaolinWu(x0, y0, x1, y1, color);
    } else {
        drawLineBresenham(x0, y0, x1, y1, color);
    }
}

/**
 * @brief Midpoint Circle Algorithm (Bresenham's Circle).
 * 
 * Derivation:
 * Evaluates circle implicit function F(x, y) = x^2 + y^2 - R^2.
 * Starts at top pixel (0, R).
 * Initial decision parameter: P_0 = 1 - R.
 * If P_k < 0, midpoint is inside: Choose E (x+1, y), P_{k+1} = P_k + 2x + 3.
 * If P_k >= 0, midpoint is outside: Choose SE (x+1, y-1), P_{k+1} = P_k + 2x - 2y + 5.
 * Exploits 8-way symmetry to plot all 8 octants simultaneously.
 */
void SoftwareRasterizer::drawCircleMidpoint(int xc, int yc, int r, uint32_t color) {
    if (r <= 0) {
        setPixel(xc, yc, color);
        return;
    }
    m_telemetry.midpointCircles++;
    if (m_inspectorHighlight) color = 0xFF22EE22; // Green in inspector mode

    int x = 0;
    int y = r;
    int d = 1 - r; // Initial decision parameter

    auto plot8 = [&](int px, int py) {
        setPixel(xc + px, yc + py, color);
        setPixel(xc - px, yc + py, color);
        setPixel(xc + px, yc - py, color);
        setPixel(xc - px, yc - py, color);
        setPixel(xc + py, yc + px, color);
        setPixel(xc - py, yc + px, color);
        setPixel(xc + py, yc - px, color);
        setPixel(xc - py, yc - px, color);
    };

    plot8(x, y);

    while (x < y) {
        x++;
        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
        plot8(x, y);
    }
}

/**
 * @brief Filled Circle using Midpoint Scanlines.
 * 
 * At each step of the Midpoint Circle calculation, horizontal spans are drawn
 * between symmetric x-coordinates on rows (yc + y), (yc - y), (yc + x), and (yc - x).
 */
void SoftwareRasterizer::fillCircleMidpoint(int xc, int yc, int r, uint32_t color) {
    if (r <= 0) {
        setPixel(xc, yc, color);
        return;
    }
    m_telemetry.midpointCircles++;
    if (m_inspectorHighlight) color = 0xFF00DD77;

    int x = 0;
    int y = r;
    int d = 1 - r;

    drawHorizontalSpan(xc - r, xc + r, yc, color);

    while (x < y) {
        x++;
        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
        drawHorizontalSpan(xc - x, xc + x, yc + y, color);
        drawHorizontalSpan(xc - x, xc + x, yc - y, color);
        drawHorizontalSpan(xc - y, xc + y, yc + x, color);
        drawHorizontalSpan(xc - y, xc + y, yc - x, color);
    }
}

/**
 * @brief Midpoint Ellipse Algorithm.
 * 
 * Ellipse equation: b^2 x^2 + a^2 y^2 - a^2 b^2 = 0.
 * Region 1: |slope| < 1, moves primarily along x.
 * Decision variable: p1 = b^2 - a^2 b + 0.25 a^2.
 * Region 2: |slope| >= 1, moves primarily along y.
 * Decision variable: p2 = b^2 (x + 0.5)^2 + a^2 (y - 1)^2 - a^2 b^2.
 * Exploits 4-way symmetry.
 */
void SoftwareRasterizer::drawEllipseMidpoint(int xc, int yc, int rx, int ry, uint32_t color) {
    if (rx <= 0 || ry <= 0) return;
    m_telemetry.midpointEllipses++;
    if (m_inspectorHighlight) color = 0xFF33CCFF; // Cyan

    long long a = rx;
    long long b = ry;
    long long a2 = a * a;
    long long b2 = b * b;

    long long x = 0;
    long long y = b;

    auto plot4 = [&](int px, int py) {
        setPixel(xc + px, yc + py, color);
        setPixel(xc - px, yc + py, color);
        setPixel(xc + px, yc - py, color);
        setPixel(xc - px, yc - py, color);
    };

    plot4(x, y);

    // Region 1
    double p1 = b2 - (a2 * b) + (0.25 * a2);
    long long dx = 2 * b2 * x;
    long long dy = 2 * a2 * y;

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
        plot4(x, y);
    }

    // Region 2
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
        plot4(x, y);
    }
}

void SoftwareRasterizer::fillEllipseMidpoint(int xc, int yc, int rx, int ry, uint32_t color) {
    if (rx <= 0 || ry <= 0) return;
    m_telemetry.midpointEllipses++;
    if (m_inspectorHighlight) color = 0xFF2299DD;

    long long a = rx;
    long long b = ry;
    long long a2 = a * a;
    long long b2 = b * b;

    long long x = 0;
    long long y = b;

    // Region 1
    double p1 = b2 - (a2 * b) + (0.25 * a2);
    long long dx = 2 * b2 * x;
    long long dy = 2 * a2 * y;

    while (dx < dy) {
        x++;
        dx += 2 * b2;
        if (p1 < 0) {
            p1 += dx + b2;
        } else {
            y--;
            dy -= 2 * a2;
            p1 += dx - dy + b2;
            drawHorizontalSpan(xc - x, xc + x, yc + (y + 1), color);
            drawHorizontalSpan(xc - x, xc + x, yc - (y + 1), color);
        }
    }

    // Region 2
    double p2 = (b2 * (x + 0.5) * (x + 0.5)) + (a2 * (y - 1) * (y - 1)) - (a2 * b2);
    while (y >= 0) {
        drawHorizontalSpan(xc - x, xc + x, yc + y, color);
        drawHorizontalSpan(xc - x, xc + x, yc - y, color);
        y--;
        dy -= 2 * a2;
        if (p2 > 0) {
            p2 += a2 - dy;
        } else {
            x++;
            dx += 2 * b2;
            p2 += dx - dy + a2;
        }
    }
}

void SoftwareRasterizer::drawRectBresenham(int x, int y, int w, int h, uint32_t color) {
    if (w <= 0 || h <= 0) return;
    int x1 = x + w - 1;
    int y1 = y + h - 1;
    drawLineBresenham(x, y, x1, y, color);
    drawLineBresenham(x1, y, x1, y1, color);
    drawLineBresenham(x1, y1, x, y1, color);
    drawLineBresenham(x, y1, x, y, color);
}

void SoftwareRasterizer::fillRectScanline(int x, int y, int w, int h, uint32_t color) {
    if (w <= 0 || h <= 0) return;
    m_telemetry.scanlineFills++;
    if (m_inspectorHighlight) color = 0xFFFFBB33;

    int yStart = std::max(0, y);
    int yEnd = std::min(m_fb.height() - 1, y + h - 1);
    for (int curY = yStart; curY <= yEnd; ++curY) {
        drawHorizontalSpan(x, x + w - 1, curY, color);
    }
}

/**
 * @brief Standard Scanline Polygon Fill Algorithm.
 * 
 * Computes bounding rows, finds edge intersections with each scanline,
 * sorts intersection points along the scanline, and fills spans between pairs (parity test).
 */
void SoftwareRasterizer::fillPolygonScanline(const std::vector<QPoint>& vertices, uint32_t color) {
    if (vertices.size() < 3) return;
    m_telemetry.scanlineFills++;
    if (m_inspectorHighlight) color = 0xFFFF8800;

    int minY = m_fb.height();
    int maxY = 0;
    for (const auto& pt : vertices) {
        minY = std::min(minY, pt.y());
        maxY = std::max(maxY, pt.y());
    }
    minY = std::max(0, minY);
    maxY = std::min(m_fb.height() - 1, maxY);

    size_t n = vertices.size();
    std::vector<int> nodeX;
    nodeX.reserve(n);

    for (int y = minY; y <= maxY; ++y) {
        nodeX.clear();
        for (size_t i = 0; i < n; ++i) {
            size_t next = (i + 1) % n;
            int y0 = vertices[i].y();
            int y1 = vertices[next].y();
            int x0 = vertices[i].x();
            int x1 = vertices[next].x();

            if ((y0 < y && y1 >= y) || (y1 < y && y0 >= y)) {
                // Linear interpolation of edge intersection
                int intersectX = x0 + static_cast<int>(std::round((static_cast<double>(y - y0) / (y1 - y0)) * (x1 - x0)));
                nodeX.push_back(intersectX);
            }
        }

        std::sort(nodeX.begin(), nodeX.end());

        // Fill spans between pairs (parity fill)
        for (size_t i = 0; i + 1 < nodeX.size(); i += 2) {
            drawHorizontalSpan(nodeX[i], nodeX[i + 1], y, color);
        }
    }
}

/**
 * @brief Sprite Blitting Engine.
 * 
 * Direct 2D bitmap memory transfer with transparency colorkeying and alpha blending.
 */
void SoftwareRasterizer::blitSprite(const Sprite& sprite, int destX, int destY, bool flipH, float alpha) {
    int sw = sprite.width();
    int sh = sprite.height();
    if (sw <= 0 || sh <= 0) return;

    m_telemetry.spriteBlits++;
    uint32_t key = sprite.transparentKey();
    uint8_t masterAlpha = static_cast<uint8_t>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f);

    for (int sy = 0; sy < sh; ++sy) {
        int dy = destY + sy;
        if (dy < 0 || dy >= m_fb.height()) continue;

        for (int sx = 0; sx < sw; ++sx) {
            int srcX = flipH ? (sw - 1 - sx) : sx;
            uint32_t pixel = sprite.getPixel(srcX, sy);

            if (pixel == key) continue;
            uint8_t pa = (pixel >> 24) & 0xFF;
            if (pa == 0) continue;

            int dx = destX + sx;
            if (dx < 0 || dx >= m_fb.width()) continue;

            if (m_inspectorHighlight) {
                m_fb.setPixel(dx, dy, 0xFFFFCC00); // Highlight blitted pixels in gold
                m_telemetry.totalPixels++;
                continue;
            }

            if (masterAlpha < 255) {
                uint8_t finalA = static_cast<uint8_t>((pa * masterAlpha) / 255);
                uint32_t blendedCol = (pixel & 0x00FFFFFF) | (static_cast<uint32_t>(finalA) << 24);
                m_fb.setPixelBlend(dx, dy, blendedCol);
            } else if (pa == 255) {
                m_fb.setPixelUnsafe(dx, dy, pixel);
            } else {
                m_fb.setPixelBlend(dx, dy, pixel);
            }
            m_telemetry.totalPixels++;
        }
    }
}

/**
 * @brief Nearest-Neighbor Scaled Sprite Blitting.
 */
void SoftwareRasterizer::blitSpriteScaled(const Sprite& sprite, int destX, int destY, int scaleX, int scaleY, bool flipH) {
    if (scaleX <= 0 || scaleY <= 0) return;
    int sw = sprite.width();
    int sh = sprite.height();
    if (sw <= 0 || sh <= 0) return;

    m_telemetry.spriteBlits++;
    uint32_t key = sprite.transparentKey();

    for (int sy = 0; sy < sh; ++sy) {
        for (int sx = 0; sx < sw; ++sx) {
            int srcX = flipH ? (sw - 1 - sx) : sx;
            uint32_t pixel = sprite.getPixel(srcX, sy);
            if (pixel == key) continue;

            // Draw scaleX x scaleY block of pixels (nearest-neighbor)
            int startX = destX + sx * scaleX;
            int startY = destY + sy * scaleY;
            fillRectScanline(startX, startY, scaleX, scaleY, pixel);
        }
    }
}

void SoftwareRasterizer::drawCharBitmap(int x, int y, char c, uint32_t color, int scale) {
    const uint8_t* glyph = Font5x7::getGlyph(c);
    for (int col = 0; col < Font5x7::GLYPH_WIDTH; ++col) {
        uint8_t bits = glyph[col];
        for (int row = 0; row < Font5x7::GLYPH_HEIGHT; ++row) {
            if ((bits >> row) & 1) {
                if (scale <= 1) {
                    setPixel(x + col, y + row, color);
                } else {
                    fillRectScanline(x + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
    }
}

void SoftwareRasterizer::drawTextBitmap(int x, int y, const std::string& text, uint32_t color, int scale) {
    int curX = x;
    int spacing = (Font5x7::GLYPH_WIDTH + 1) * scale;
    for (char c : text) {
        if (c == '\n') {
            y += (Font5x7::GLYPH_HEIGHT + 2) * scale;
            curX = x;
            continue;
        }
        drawCharBitmap(curX, y, c, color, scale);
        curX += spacing;
    }
}

void SoftwareRasterizer::applyScanlines(float intensity) {
    m_fb.applyScanlines(intensity);
}

void SoftwareRasterizer::applyInvert() {
    m_fb.invertColors();
}

/**
 * @brief 2D Affine Transformation (Software Raster Rotation via Inverse Mapping).
 * 
 * To avoid sampling holes and grid tears, we evaluate inverse mapping:
 * For each destination pixel in the bounding circle, calculate its corresponding
 * source coordinate in the original sprite matrix using rotation matrix R(-theta).
 */
void SoftwareRasterizer::blitSpriteRotated(const Sprite& sprite, int destCenterX, int destCenterY, float angleDegrees, float alpha) {
    int sw = sprite.width();
    int sh = sprite.height();
    if (sw <= 0 || sh <= 0) return;

    if (std::abs(angleDegrees) < 0.1f) {
        // Fast-path unrotated blit
        blitSprite(sprite, destCenterX - sw / 2, destCenterY - sh / 2, false, alpha);
        return;
    }

    m_telemetry.rotatedSprites++;
    float rad = -angleDegrees * (3.14159265f / 180.0f); // Inverted angle for inverse mapping
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);

    float srcCx = sw * 0.5f;
    float srcCy = sh * 0.5f;

    // Radius of bounding box
    int radius = static_cast<int>(std::ceil(std::hypot(sw * 0.5f, sh * 0.5f))) + 1;
    uint32_t key = sprite.transparentKey();
    uint8_t masterAlpha = static_cast<uint8_t>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f);

    for (int dy = -radius; dy <= radius; ++dy) {
        int py = destCenterY + dy;
        if (py < 0 || py >= m_fb.height()) continue;

        for (int dx = -radius; dx <= radius; ++dx) {
            int px = destCenterX + dx;
            if (px < 0 || px >= m_fb.width()) continue;

            // Inverse affine mapping: source = R(theta) * dest
            float srcX = dx * cosA - dy * sinA + srcCx;
            float srcY = dx * sinA + dy * cosA + srcCy;

            int sx = static_cast<int>(std::round(srcX));
            int sy = static_cast<int>(std::round(srcY));

            if (sx >= 0 && sx < sw && sy >= 0 && sy < sh) {
                uint32_t pixel = sprite.getPixel(sx, sy);
                if (pixel == key) continue;
                uint8_t pa = (pixel >> 24) & 0xFF;
                if (pa == 0) continue;

                if (masterAlpha < 255) {
                    uint8_t finalA = static_cast<uint8_t>((pa * masterAlpha) / 255);
                    uint32_t blendedCol = (pixel & 0x00FFFFFF) | (static_cast<uint32_t>(finalA) << 24);
                    m_fb.setPixelBlend(px, py, blendedCol);
                } else if (pa == 255) {
                    m_fb.setPixelUnsafe(px, py, pixel);
                } else {
                    m_fb.setPixelBlend(px, py, pixel);
                }
                m_telemetry.totalPixels++;
            }
        }
    }
}

void SoftwareRasterizer::applyRadialVignette(int cx, int cy, int innerR, int outerR, float maxDarkness) {
    if (innerR >= outerR) return;
    int w = m_fb.width();
    int h = m_fb.height();
    int innerR2 = innerR * innerR;
    int outerR2 = outerR * outerR;
    float invRange = 1.0f / static_cast<float>(outerR - innerR);
    float fullDarkKeep = 1.0f - maxDarkness;

    for (int y = 0; y < h; ++y) {
        int dy = y - cy;
        int dy2 = dy * dy;
        for (int x = 0; x < w; ++x) {
            int dx = x - cx;
            int d2 = dx * dx + dy2;

            if (d2 <= innerR2) {
                continue; // Center circle remains 100% illuminated - skip sqrt!
            }

            float keep;
            if (d2 >= outerR2) {
                keep = fullDarkKeep; // Full darkness exterior - skip sqrt!
            } else {
                float dist = std::sqrt(static_cast<float>(d2));
                float factor = (dist - static_cast<float>(innerR)) * invRange * maxDarkness;
                keep = 1.0f - factor;
            }

            uint32_t col = m_fb.getPixel(x, y);
            uint8_t a, r, g, b;
            Framebuffer::unpackARGB(col, a, r, g, b);
            r = static_cast<uint8_t>(r * keep);
            g = static_cast<uint8_t>(g * keep);
            b = static_cast<uint8_t>(b * keep);
            m_fb.setPixelUnsafe(x, y, Framebuffer::packARGB(a, r, g, b));
        }
    }
}
