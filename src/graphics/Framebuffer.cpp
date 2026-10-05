#include "Framebuffer.h"

Framebuffer::Framebuffer(int width, int height)
    : m_width(width), m_height(height), m_pixels(width * height, 0xFFFFFFFF)
{
}

void Framebuffer::resize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    m_width = width;
    m_height = height;
    m_pixels.assign(width * height, 0xFFFFFFFF);
}

void Framebuffer::clear(uint32_t color) {
    std::fill(m_pixels.begin(), m_pixels.end(), color);
}

void Framebuffer::setPixelBlend(int x, int y, uint32_t srcColor) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;

    uint8_t sa = (srcColor >> 24) & 0xFF;
    if (sa == 0) return; // Completely transparent
    if (sa == 255) {    // Completely opaque
        m_pixels[y * m_width + x] = srcColor;
        m_pixelsWritten++;
        return;
    }

    uint32_t dstColor = m_pixels[y * m_width + x];
    uint8_t sr = (srcColor >> 16) & 0xFF;
    uint8_t sg = (srcColor >> 8)  & 0xFF;
    uint8_t sb = srcColor & 0xFF;

    uint8_t dr = (dstColor >> 16) & 0xFF;
    uint8_t dg = (dstColor >> 8)  & 0xFF;
    uint8_t db = dstColor & 0xFF;

    // Standard alpha compositing: out = (src * a + dst * (255 - a)) / 255
    uint32_t invA = 255 - sa;
    uint8_t outR = static_cast<uint8_t>((sr * sa + dr * invA) / 255);
    uint8_t outG = static_cast<uint8_t>((sg * sa + dg * invA) / 255);
    uint8_t outB = static_cast<uint8_t>((sb * sa + db * invA) / 255);

    m_pixels[y * m_width + x] = packARGB(255, outR, outG, outB);
    m_pixelsWritten++;
}

QImage Framebuffer::toQImage() const {
    // Construct QImage directly wrapping the raw contiguous pixel buffer.
    // Zero-copy wrapping for ultra-fast software raster presentation.
    return QImage(
        reinterpret_cast<const uchar*>(m_pixels.data()),
        m_width,
        m_height,
        m_width * sizeof(uint32_t),
        QImage::Format_ARGB32
    ).copy(); // Return an independent copy for thread/paint safety
}

void Framebuffer::applyScanlines(float intensity) {
    // Dim every second raster line (y % 2 == 1) by intensity factor
    float factor = 1.0f - std::clamp(intensity, 0.0f, 1.0f);
    for (int y = 1; y < m_height; y += 2) {
        int rowOffset = y * m_width;
        for (int x = 0; x < m_width; ++x) {
            uint32_t col = m_pixels[rowOffset + x];
            uint8_t a = (col >> 24) & 0xFF;
            uint8_t r = static_cast<uint8_t>(((col >> 16) & 0xFF) * factor);
            uint8_t g = static_cast<uint8_t>(((col >> 8)  & 0xFF) * factor);
            uint8_t b = static_cast<uint8_t>((col & 0xFF) * factor);
            m_pixels[rowOffset + x] = packARGB(a, r, g, b);
        }
    }
}

void Framebuffer::invertColors() {
    // Pure raster byte-wise inversion: R' = 255 - R, G' = 255 - G, B' = 255 - B
    for (size_t i = 0; i < m_pixels.size(); ++i) {
        uint32_t col = m_pixels[i];
        uint8_t a = (col >> 24) & 0xFF;
        uint8_t r = 255 - ((col >> 16) & 0xFF);
        uint8_t g = 255 - ((col >> 8) & 0xFF);
        uint8_t b = 255 - (col & 0xFF);
        m_pixels[i] = packARGB(a, r, g, b);
    }
}

void Framebuffer::applyGameBoyPalette() {
    // Classic DMG-01 Game Boy 4-shade green palette
    static const uint32_t GB_PALETTE[4] = {
        0xFF0F380F, // Darkest green
        0xFF306230, // Dark green
        0xFF8BAC0F, // Light green
        0xFF9BBC0F  // Lightest green
    };

    for (size_t i = 0; i < m_pixels.size(); ++i) {
        uint32_t col = m_pixels[i];
        uint8_t a = (col >> 24) & 0xFF;
        uint8_t r = (col >> 16) & 0xFF;
        uint8_t g = (col >> 8)  & 0xFF;
        uint8_t b = col & 0xFF;

        int lum = (r * 299 + g * 587 + b * 114) / 1000;
        int idx = std::clamp(lum / 64, 0, 3);
        uint32_t mapped = GB_PALETTE[idx];
        m_pixels[i] = (mapped & 0x00FFFFFF) | (static_cast<uint32_t>(a) << 24);
    }
}

void Framebuffer::applyCyberpunkPalette() {
    // Synthwave / Cyberpunk neon palette
    for (size_t i = 0; i < m_pixels.size(); ++i) {
        uint32_t col = m_pixels[i];
        uint8_t a = (col >> 24) & 0xFF;
        uint8_t r = (col >> 16) & 0xFF;
        uint8_t g = (col >> 8)  & 0xFF;
        uint8_t b = col & 0xFF;

        int lum = (r * 299 + g * 587 + b * 114) / 1000;
        uint32_t mapped;
        if (lum < 50) {
            mapped = 0xFF0D0221; // Deep synthwave void
        } else if (lum < 130) {
            mapped = 0xFFFF007F; // Hot magenta
        } else if (lum < 200) {
            mapped = 0xFF00F0FF; // Electric neon cyan
        } else {
            mapped = 0xFFFFE600; // Cyber gold
        }
        m_pixels[i] = (mapped & 0x00FFFFFF) | (static_cast<uint32_t>(a) << 24);
    }
}

void Framebuffer::applyBayerDithering() {
    // Textbook 4x4 Ordered Bayer Dithering Matrix
    static const int BAYER_4X4[4][4] = {
        {  0,  8,  2, 10 },
        { 12,  4, 14,  6 },
        {  3, 11,  1,  9 },
        { 15,  7, 13,  5 }
    };

    for (int y = 0; y < m_height; ++y) {
        int row = y * m_width;
        int by = y % 4;
        for (int x = 0; x < m_width; ++x) {
            uint32_t col = m_pixels[row + x];
            uint8_t r = (col >> 16) & 0xFF;
            uint8_t g = (col >> 8)  & 0xFF;
            uint8_t b = col & 0xFF;
            int lum = (r * 299 + g * 587 + b * 114) / 1000;

            int threshold = (BAYER_4X4[by][x % 4] * 255) / 16;
            uint32_t dithered = (lum > threshold) ? 0xFFFFFFFF : 0xFF000000;
            m_pixels[row + x] = dithered;
        }
    }
}

void Framebuffer::applyAmberPalette() {
    // Vintage Monochrome Amber CRT Monitor Phosphor
    for (size_t i = 0; i < m_pixels.size(); ++i) {
        uint32_t col = m_pixels[i];
        uint8_t a = (col >> 24) & 0xFF;
        uint8_t r = (col >> 16) & 0xFF;
        uint8_t g = (col >> 8)  & 0xFF;
        uint8_t b = col & 0xFF;

        int lum = (r * 299 + g * 587 + b * 114) / 1000;
        uint8_t outR = static_cast<uint8_t>(lum);
        uint8_t outG = static_cast<uint8_t>(lum * 0.72f);
        uint8_t outB = static_cast<uint8_t>(lum * 0.08f);
        m_pixels[i] = packARGB(a, outR, outG, outB);
    }
}

void Framebuffer::applyScreenShake(int dx, int dy, uint32_t clearColor) {
    if (dx == 0 && dy == 0) return;

    std::vector<uint32_t> temp = m_pixels;
    std::fill(m_pixels.begin(), m_pixels.end(), clearColor);

    for (int y = 0; y < m_height; ++y) {
        int targetY = y + dy;
        if (targetY < 0 || targetY >= m_height) continue;

        for (int x = 0; x < m_width; ++x) {
            int targetX = x + dx;
            if (targetX < 0 || targetX >= m_width) continue;

            m_pixels[targetY * m_width + targetX] = temp[y * m_width + x];
        }
    }
}
