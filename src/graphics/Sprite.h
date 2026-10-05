#ifndef SPRITE_H
#define SPRITE_H

#include <vector>
#include <string>
#include <cstdint>

/**
 * @file Sprite.h
 * @brief 2D Bitmap Sprite representation for Raster Blitting.
 * 
 * In raster graphics, sprites are 2D pixel grids (bitmaps) blitted directly
 * into the main screen buffer. This class stores the sprite bitmap data
 * and provides pixel mask queries for pixel-perfect collision detection.
 */
class Sprite {
public:
    Sprite(int width = 0, int height = 0);
    Sprite(int width, int height, const std::vector<uint32_t>& pixels, uint32_t transparentKey = 0x00000000);

    int width() const { return m_width; }
    int height() const { return m_height; }
    uint32_t transparentKey() const { return m_transparentKey; }
    const std::vector<uint32_t>& pixels() const { return m_pixels; }

    uint32_t getPixel(int x, int y) const {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            return m_pixels[y * m_width + x];
        }
        return m_transparentKey;
    }

    void setPixel(int x, int y, uint32_t color) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_pixels[y * m_width + x] = color;
        }
    }

    // Returns true if the pixel is solid (non-transparent)
    bool isSolid(int x, int y) const {
        uint32_t c = getPixel(x, y);
        uint8_t a = (c >> 24) & 0xFF;
        return a > 0 && c != m_transparentKey;
    }

    // Factory method: parses an ASCII pattern into a pixel-art bitmap
    static Sprite fromAscii(int width, int height, const std::string& ascii,
                            char solidChar = '#', uint32_t solidColor = 0xFF535353,
                            uint32_t transparentColor = 0x00000000);

    // Creates a horizontally flipped clone of this sprite
    Sprite flippedHorizontal() const;

    // Creates an inverted color version of this sprite (for Day/Night mode)
    Sprite invertedColors() const;

private:
    int m_width;
    int m_height;
    uint32_t m_transparentKey;
    std::vector<uint32_t> m_pixels;
};

#endif // SPRITE_H
