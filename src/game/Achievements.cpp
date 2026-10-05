#include "Achievements.h"
#include "../audio/RetroAudio.h"
#include <algorithm>

Achievements::Achievements() {
    m_achievements = {
        {"first_jump",   "FIRST LEAP",      "Perform your first jump"},
        {"night_owl",    "NIGHT OWL",        "Survive until midnight"},
        {"sunglasses",   "FASHION ICON",     "Reach 500 points & equip shades"},
        {"party_hat",    "PARTY ANIMAL",     "Reach 1000 points & wear party hat"},
        {"golden_dino",  "GOLDEN LEGEND",    "Ascend to 2000 points aura"},
        {"storm",        "STORM CHASER",     "Encounter a desert thunderstorm"},
        {"shield",       "BONE ARMOR",       "Equip protective bone helmet"},
        {"bullet_time",  "BULLET TIME",      "Activate slow-motion clock"},
        {"double_jump",  "AIR WALKER",       "Perform mid-air double jump"}
    };
    reset();
}

void Achievements::reset() {
    m_activePopupTitle.clear();
    m_popupTimer = 0.0f;
    m_popupSlideX = 300.0f;
}

void Achievements::trigger(const std::string& id, RetroAudio* audio) {
    if (m_unlockedIds.find(id) != m_unlockedIds.end()) return;

    for (auto& a : m_achievements) {
        if (a.id == id) {
            a.unlocked = true;
            m_unlockedIds.insert(id);
            showPopup(a.title);
            if (audio) {
                audio->playScoreMilestone();
            }
            break;
        }
    }
}

void Achievements::showPopup(const std::string& title) {
    m_activePopupTitle = title;
    m_popupTimer = 3.5f;
    m_popupSlideX = 260.0f;
}

void Achievements::update(float dt) {
    if (m_popupTimer > 0.0f) {
        m_popupTimer -= dt;
        // Slide in from right
        if (m_popupSlideX > 0.0f) {
            m_popupSlideX = std::max(0.0f, m_popupSlideX - 700.0f * dt);
        }
    } else {
        // Slide out to right
        if (m_popupSlideX < 260.0f) {
            m_popupSlideX += 700.0f * dt;
        }
    }
}

void Achievements::render(SoftwareRasterizer& rasterizer) {
    if (m_popupSlideX >= 250.0f || m_activePopupTitle.empty()) return;

    int pw = 210;
    int ph = 30;
    int px = rasterizer.framebuffer().width() - pw - 12 + static_cast<int>(m_popupSlideX);
    int py = 12;

    // Toast panel background
    for (int y = py; y < py + ph; ++y) {
        for (int x = px; x < px + pw; ++x) {
            rasterizer.framebuffer().setPixelBlend(x, y, 0xEE161820);
        }
    }
    rasterizer.drawRectBresenham(px, py, pw, ph, 0xFFFFD700);

    // Trophy icon / text
    rasterizer.drawTextBitmap(px + 8, py + 6, "ACHIEVEMENT UNLOCKED!", 0xFFFFD700, 1);
    rasterizer.drawTextBitmap(px + 8, py + 18, m_activePopupTitle, 0xFFFFFFFF, 1);
}
