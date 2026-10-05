#include "LabInspector.h"
#include <cstdio>
#include <string>

void LabInspector::renderHUD(SoftwareRasterizer& rasterizer,
                             float fps, float frameTimeMs,
                             int score, int highScore,
                             const AABB& dinoBox,
                             const std::vector<AABB>& obstacleBoxes,
                             bool collisionDetected, int hitX, int hitY)
{
    if (!m_active) return;

    // 1. Draw collision boxes using Bresenham lines
    uint32_t dinoBoxCol = collisionDetected ? 0xFFFF0055 : 0xFF00FFCC;
    rasterizer.drawRectBresenham(dinoBox.x, dinoBox.y, dinoBox.w, dinoBox.h, dinoBoxCol);

    for (const auto& box : obstacleBoxes) {
        uint32_t obsBoxCol = collisionDetected ? 0xFFFF0000 : 0xFFFF00AA;
        rasterizer.drawRectBresenham(box.x, box.y, box.w, box.h, obsBoxCol);
    }

    // If collision happened, draw crosshairs at exact collision pixel
    if (collisionDetected && hitX >= 0 && hitY >= 0) {
        rasterizer.drawLineBresenham(hitX - 6, hitY, hitX + 6, hitY, 0xFFFFFFFF);
        rasterizer.drawLineBresenham(hitX, hitY - 6, hitX, hitY + 6, 0xFFFFFFFF);
        rasterizer.drawCircleMidpoint(hitX, hitY, 4, 0xFFFF0000);
    }

    // 2. Draw Semi-transparent HUD Panel on Top-Left
    int hudW = 270;
    int hudH = 134;
    int hudX = 8;
    int hudY = 8;

    // Scanline fill with dark alpha backing
    for (int y = hudY; y < hudY + hudH; ++y) {
        for (int x = hudX; x < hudX + hudW; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xCC15171C);
        }
    }
    // Panel border with Bresenham line
    rasterizer.drawRectBresenham(hudX, hudY, hudW, hudH, 0xFF00E5FF);

    // 3. Render telemetry strings using discrete 5x7 font
    const auto& tel = rasterizer.telemetry();
    char buf[128];

    rasterizer.drawTextBitmap(hudX + 6, hudY + 6, "=== CG LAB RASTER INSPECTOR ===", 0xFF00E5FF, 1);

    std::snprintf(buf, sizeof(buf), "FPS: %.1f  (%.2f ms) | SCALE: 1X NATIVE", fps, frameTimeMs);
    rasterizer.drawTextBitmap(hudX + 6, hudY + 18, buf, 0xFFFFFFFF, 1);

    std::snprintf(buf, sizeof(buf), "BRESENHAM LINES:     %u", tel.bresenhamLines);
    rasterizer.drawTextBitmap(hudX + 6, hudY + 30, buf, 0xFFFF6666, 1);

    std::snprintf(buf, sizeof(buf), "XIAOLIN WU (AA):     %u", tel.xiaolinWuLines);
    rasterizer.drawTextBitmap(hudX + 6, hudY + 42, buf, 0xFF00FFCC, 1);

    std::snprintf(buf, sizeof(buf), "MIDPOINT CIRCLES:    %u", tel.midpointCircles);
    rasterizer.drawTextBitmap(hudX + 6, hudY + 54, buf, 0xFF66FF66, 1);

    std::snprintf(buf, sizeof(buf), "MIDPOINT ELLIPSES:   %u", tel.midpointEllipses);
    rasterizer.drawTextBitmap(hudX + 6, hudY + 66, buf, 0xFF66CCFF, 1);

    std::snprintf(buf, sizeof(buf), "SCANLINE FILLS:      %u", tel.scanlineFills);
    rasterizer.drawTextBitmap(hudX + 6, hudY + 78, buf, 0xFFFFBB33, 1);

    std::snprintf(buf, sizeof(buf), "SPRITE BIT-BLITS:    %u", tel.spriteBlits);
    rasterizer.drawTextBitmap(hudX + 6, hudY + 90, buf, 0xFFFFCC00, 1);

    std::snprintf(buf, sizeof(buf), "ROTATED SPRITES:     %u", tel.rotatedSprites);
    rasterizer.drawTextBitmap(hudX + 6, hudY + 102, buf, 0xFFFF77CC, 1);

    std::snprintf(buf, sizeof(buf), "PIXELS RASTERIZED:   %llu", static_cast<unsigned long long>(tel.totalPixels));
    rasterizer.drawTextBitmap(hudX + 6, hudY + 114, buf, 0xFFE0E0E0, 1);

    // 4. Magnifier Tool (Bottom-Right)
    if (m_showMagnifier) {
        int inspectX = dinoBox.x + dinoBox.w / 2;
        int inspectY = dinoBox.y + dinoBox.h / 2;
        if (collisionDetected && hitX >= 0 && hitY >= 0) {
            inspectX = hitX;
            inspectY = hitY;
        }
        drawMagnifier(rasterizer, inspectX, inspectY, rasterizer.framebuffer().width() - 110, 8);
    }
}

void LabInspector::drawMagnifier(SoftwareRasterizer& rasterizer, int inspectX, int inspectY, int destX, int destY) {
    constexpr int GRID_SIZE = 12; // 12x12 pixels inspected
    constexpr int CELL_SIZE = 7;  // 7x7 screen pixels per framebuffer pixel
    int magW = GRID_SIZE * CELL_SIZE;
    int magH = GRID_SIZE * CELL_SIZE;

    // Draw Magnifier Background panel
    for (int y = destY; y < destY + magH + 20; ++y) {
        for (int x = destX - 4; x < destX + magW + 4; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xEE111318);
        }
    }
    rasterizer.drawRectBresenham(destX - 4, destY, magW + 8, magH + 20, 0xFFFF0077);
    rasterizer.drawTextBitmap(destX, destY + 3, "PIXEL ZOOM (12x12)", 0xFFFF0077, 1);

    int startInspectX = inspectX - GRID_SIZE / 2;
    int startInspectY = inspectY - GRID_SIZE / 2;
    int contentY = destY + 14;

    for (int gy = 0; gy < GRID_SIZE; ++gy) {
        for (int gx = 0; gx < GRID_SIZE; ++gx) {
            int srcX = startInspectX + gx;
            int srcY = startInspectY + gy;
            uint32_t col = rasterizer.getPixel(srcX, srcY);

            int px = destX + gx * CELL_SIZE;
            int py = contentY + gy * CELL_SIZE;

            // Fill magnified cell
            for (int dy = 0; dy < CELL_SIZE - 1; ++dy) {
                for (int dx = 0; dx < CELL_SIZE - 1; ++dx) {
                    rasterizer.framebuffer().setPixelUnsafe(px + dx, py + dy, col);
                }
            }
            // 1px discrete raster gridline
            for (int d = 0; d < CELL_SIZE; ++d) {
                rasterizer.framebuffer().setPixel(px + CELL_SIZE - 1, py + d, 0xFF3A3D45);
                rasterizer.framebuffer().setPixel(px + d, py + CELL_SIZE - 1, 0xFF3A3D45);
            }
        }
    }

    // Center crosshair indicating exact sample pixel
    int centerCellX = destX + (GRID_SIZE / 2) * CELL_SIZE;
    int centerCellY = contentY + (GRID_SIZE / 2) * CELL_SIZE;
    rasterizer.drawRectBresenham(centerCellX, centerCellY, CELL_SIZE, CELL_SIZE, 0xFF00FF00);
}
