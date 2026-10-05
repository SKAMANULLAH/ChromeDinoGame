#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

#include "../graphics/SoftwareRasterizer.h"
#include <string>
#include <vector>
#include <unordered_set>

class RetroAudio;

struct Achievement {
    std::string id;
    std::string title;
    std::string description;
    bool unlocked{false};
};

class Achievements {
public:
    Achievements();

    void reset();
    void update(float dt);
    void render(SoftwareRasterizer& rasterizer);

    void trigger(const std::string& id, RetroAudio* audio = nullptr);

    bool isUnlocked(const std::string& id) const {
        return m_unlockedIds.find(id) != m_unlockedIds.end();
    }

private:
    std::vector<Achievement> m_achievements;
    std::unordered_set<std::string> m_unlockedIds;

    // Toast popup
    std::string m_activePopupTitle;
    float m_popupTimer{0.0f};
    float m_popupSlideX{300.0f}; // Slide offset

    void showPopup(const std::string& title);
};

#endif // ACHIEVEMENTS_H
