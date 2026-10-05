#ifndef LABINSPECTOR_H
#define LABINSPECTOR_H

#include "../graphics/SoftwareRasterizer.h"
#include "../game/Entity.h"
#include <string>

/**
 * @file LabInspector.h
 * @brief Live Computer Graphics Lab Inspector & Telemetry HUD.
 * 
 * Demonstrates to the instructor that all rendering is 100% discrete raster,
 * with real-time algorithm counters, magnified pixel grid inspection,
 * and primitive color tagging.
 */
class LabInspector {
public:
    LabInspector() = default;

    void toggle() { m_active = !m_active; }
    void setActive(bool active) { m_active = active; }
    bool isActive() const { return m_active; }

    void toggleMagnifier() { m_showMagnifier = !m_showMagnifier; }
    bool isMagnifierActive() const { return m_showMagnifier; }

    void renderHUD(SoftwareRasterizer& rasterizer,
                   float fps, float frameTimeMs,
                   int score, int highScore,
                   const AABB& dinoBox,
                   const std::vector<AABB>& obstacleBoxes,
                   bool collisionDetected, int hitX, int hitY);

private:
    bool m_active{false};
    bool m_showMagnifier{true};

    void drawMagnifier(SoftwareRasterizer& rasterizer, int inspectX, int inspectY, int destX, int destY);
};

#endif // LABINSPECTOR_H
