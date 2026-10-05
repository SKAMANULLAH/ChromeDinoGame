#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include <cstdint>
#include <algorithm>
#include <QImage>

/**
 * @file Framebuffer.h
 * @brief Software Raster Framebuffer for Computer Graphics Lab.
 * 
 * In pure raster graphics, the screen is represented as a contiguous 2D grid
 * of discrete picture elements (pixels), known as a Framebuffer.
 * 
 * Unlike vector graphics (which store geometric commands and mathematical equations),
 * the framebuffer stores the discrete color values directly in memory.
 */
class Framebuffer {
public:
    Framebuffer(int width = 800, int height = 300);
    ~Framebuffer() = default;

    void resize(int width, int height);
    void clear(uint32_t color = 0xFFFFFFFF); // Default white background

    // Direct pixel access with bounds checking
    inline void setPixel(int x, int y, uint32_t color) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_pixels[y * m_width + x] = color;
            m_pixelsWritten++;
        }
    }

    // Direct pixel access without bounds checking (for verified scanlines)
    inline void setPixelUnsafe(int x, int y, uint32_t color) {
        m_pixels[y * m_width + x] = color;
        m_pixelsWritten++;
    }

    inline uint32_t getPixel(int x, int y) const {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            return m_pixels[y * m_width + x];
        }
        return 0x00000000;
    }

    // Alpha blending (Source Over Destination)
    // Formula: C_out = (C_src * A_src + C_dst * (255 - A_src)) / 255
    void setPixelBlend(int x, int y, uint32_t srcColor);

    int width() const { return m_width; }
    int height() const { return m_height; }
    const uint32_t* data() const { return m_pixels.data(); }
    uint32_t* data() { return m_pixels.data(); }
    size_t pixelCount() const { return m_pixels.size(); }

    // Converts internal raw raster buffer to QImage for 1-blit presentation
    QImage toQImage() const;

    // Direct byte/pixel post-processing effects
    void applyScanlines(float intensity = 0.25f);
    void invertColors();
    void applyGameBoyPalette();
    void applyCyberpunkPalette();
    void applyBayerDithering();
    void applyAmberPalette();
    void applyScreenShake(int dx, int dy, uint32_t clearColor);

    // Telemetry for CG lab demonstration
    uint64_t pixelsWrittenCount() const { return m_pixelsWritten; }
    void resetTelemetry() { m_pixelsWritten = 0; }

    // Static color helpers (ARGB 32-bit format)
    static inline uint32_t packARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) << 8)  |
               (static_cast<uint32_t>(b));
    }

    static inline void unpackARGB(uint32_t color, uint8_t& a, uint8_t& r, uint8_t& g, uint8_t& b) {
        a = (color >> 24) & 0xFF;
        r = (color >> 16) & 0xFF;
        g = (color >> 8)  & 0xFF;
        b = color & 0xFF;
    }

private:
    int m_width;
    int m_height;
    std::vector<uint32_t> m_pixels;
    uint64_t m_pixelsWritten{0};
};

#endif // FRAMEBUFFER_H
