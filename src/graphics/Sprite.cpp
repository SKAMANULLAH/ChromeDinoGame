#include "Sprite.h"
#include "Framebuffer.h"

Sprite::Sprite(int width, int height)
    : m_width(width), m_height(height), m_transparentKey(0x00000000),
      m_pixels(width * height, 0x00000000)
{
}

Sprite::Sprite(int width, int height, const std::vector<uint32_t>& pixels, uint32_t transparentKey)
    : m_width(width), m_height(height), m_transparentKey(transparentKey), m_pixels(pixels)
{
}

Sprite Sprite::fromAscii(int width, int height, const std::string& ascii,
                         char solidChar, uint32_t solidColor, uint32_t transparentColor)
{
    Sprite sp(width, height);
    sp.m_transparentKey = transparentColor;
    
    // Strip newlines/whitespace when reading characters or read coordinate by coordinate
    int curX = 0;
    int curY = 0;

    for (char ch : ascii) {
        if (ch == '\r') continue;
        if (ch == '\n') {
            curY++;
            curX = 0;
            if (curY >= height) break;
            continue;
        }
        if (curX < width && curY < height) {
            uint32_t col = (ch == solidChar) ? solidColor : transparentColor;
            sp.setPixel(curX, curY, col);
            curX++;
        }
    }
    return sp;
}

Sprite Sprite::flippedHorizontal() const {
    Sprite result(m_width, m_height);
    result.m_transparentKey = m_transparentKey;

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            result.setPixel(m_width - 1 - x, y, getPixel(x, y));
        }
    }
    return result;
}

Sprite Sprite::invertedColors() const {
    Sprite result(m_width, m_height);
    result.m_transparentKey = m_transparentKey;

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            uint32_t c = getPixel(x, y);
            if (c == m_transparentKey || ((c >> 24) & 0xFF) == 0) {
                result.setPixel(x, y, m_transparentKey);
            } else {
                uint8_t a, r, g, b;
                Framebuffer::unpackARGB(c, a, r, g, b);
                result.setPixel(x, y, Framebuffer::packARGB(a, 255 - r, 255 - g, 255 - b));
            }
        }
    }
    return result;
}
