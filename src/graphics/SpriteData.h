#ifndef SPRITEDATA_H
#define SPRITEDATA_H

#include "Sprite.h"

/**
 * @file SpriteData.h
 * @brief Pre-rasterized Bitmaps for Dino, Obstacles, and Environment.
 */
class SpriteData {
public:
    static void initialize();

    // Dino sprites
    static const Sprite& dinoIdle();
    static const Sprite& dinoRun1();
    static const Sprite& dinoRun2();
    static const Sprite& dinoDuck1();
    static const Sprite& dinoDuck2();
    static const Sprite& dinoDead();
    static const Sprite& dinoBlink();
    static const Sprite& sunglasses();
    static const Sprite& partyHat();
    static const Sprite& dinoHelmet();

    // Power-Up item icons
    static const Sprite& powerupShield();
    static const Sprite& powerupClock();
    static const Sprite& powerupFeather();

    // Cactus sprites
    static const Sprite& cactusSmall1();
    static const Sprite& cactusSmall2();
    static const Sprite& cactusSmall3();
    static const Sprite& cactusBig1();
    static const Sprite& cactusBig2();

    // Pterodactyl flying enemy
    static const Sprite& pteroWingUp();
    static const Sprite& pteroWingDown();

    // Scenery & Environment
    static const Sprite& cloud();
    static const Sprite& moonFull();
    static const Sprite& moonCrescent();
    static const Sprite& star();
    static const Sprite& restartIcon();

private:
    static bool s_initialized;
    static Sprite s_dinoIdle;
    static Sprite s_dinoRun1;
    static Sprite s_dinoRun2;
    static Sprite s_dinoDuck1;
    static Sprite s_dinoDuck2;
    static Sprite s_dinoDead;
    static Sprite s_dinoBlink;
    static Sprite s_sunglasses;
    static Sprite s_partyHat;
    static Sprite s_dinoHelmet;
    static Sprite s_powerupShield;
    static Sprite s_powerupClock;
    static Sprite s_powerupFeather;

    static Sprite s_cactusSmall1;
    static Sprite s_cactusSmall2;
    static Sprite s_cactusSmall3;
    static Sprite s_cactusBig1;
    static Sprite s_cactusBig2;

    static Sprite s_pteroWingUp;
    static Sprite s_pteroWingDown;

    static Sprite s_cloud;
    static Sprite s_moonFull;
    static Sprite s_moonCrescent;
    static Sprite s_star;
    static Sprite s_restartIcon;
};

#endif // SPRITEDATA_H
