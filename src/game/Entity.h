#ifndef ENTITY_H
#define ENTITY_H

#include "../graphics/Sprite.h"
#include <algorithm>

/**
 * @file Entity.h
 * @brief Base geometry, AABB bounding box, and Pixel-Perfect collision detection.
 */
struct AABB {
    int x{0};
    int y{0};
    int w{0};
    int h{0};

    bool intersects(const AABB& o) const {
        return (x < o.x + o.w &&
                x + w > o.x &&
                y < o.y + o.h &&
                y + h > o.y);
    }

    AABB intersection(const AABB& o) const {
        int ix1 = std::max(x, o.x);
        int iy1 = std::max(y, o.y);
        int ix2 = std::min(x + w, o.x + o.w);
        int iy2 = std::min(y + h, o.y + o.h);
        if (ix2 > ix1 && iy2 > iy1) {
            return {ix1, iy1, ix2 - ix1, iy2 - iy1};
        }
        return {0, 0, 0, 0};
    }
};

/**
 * @brief Checks pixel-perfect collision between two raster sprites.
 * 
 * If a collision occurs, sets collisionPoint to the first colliding pixel in world coordinates.
 */
inline bool checkPixelCollision(const Sprite& spriteA, int ax, int ay,
                                const Sprite& spriteB, int bx, int by,
                                int& hitX, int& hitY)
{
    AABB boxA{ax, ay, spriteA.width(), spriteA.height()};
    AABB boxB{bx, by, spriteB.width(), spriteB.height()};
    AABB overlap = boxA.intersection(boxB);

    if (overlap.w <= 0 || overlap.h <= 0) return false;

    for (int y = overlap.y; y < overlap.y + overlap.h; ++y) {
        for (int x = overlap.x; x < overlap.x + overlap.w; ++x) {
            if (spriteA.isSolid(x - ax, y - ay) && spriteB.isSolid(x - bx, y - by)) {
                hitX = x;
                hitY = y;
                return true;
            }
        }
    }
    return false;
}

#endif // ENTITY_H
