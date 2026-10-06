# Module 2: Dino Physics Kinematics & Collision Detection

**Author / Maintainer:** [Friend 1 Name / GitHub Username]  
**Primary Engine Source:** `src/game/Dino.h`, `src/game/Dino.cpp`

## 1. Discrete Euler Kinematics

The player motion is calculated at discrete 16ms time-steps:

- Vertical Velocity: v_y(t + dt) = v_y(t) + g \* dt
- Vertical Position: y(t + dt) = y(t) + v_y(t + dt) \* dt
- Jump impulse: -820 px/s (upward)
- Gravity: +2400 px/s^2 (downward)
- Fast-duck drop gravity: +4200 px/s^2

## 2. Hitbox Architecture: AABB

- Standing Dino Box: 44 x 47 px
- Ducking Dino Box: 59 x 30 px (lowers profile beneath airborne Pterodactyls)
- AABB overlap test validates horizontal and vertical axis intersections simultaneously.


Here is the complete breakdown of all the **gameplay features, power-ups, cosmetics, and atmospheric systems** built into your game:

---

# 1. Power-Ups (Spawns Every ~16 Seconds)
Power-ups float at jump-height across the screen with a gentle sine-wave bobbing motion ($y = y_0 + 6 \sin(4t)$). You must time your jump to collect them:

| Power-Up | Visual Icon | In-Game Effect | Implementation Detail |
| :--- | :--- | :--- | :--- |
| **Bone Helmet (Shield)** | 🛡️ Cyan Glow Circle | **Grants 1-hit protection.** Absorbs a fatal obstacle collision. | Dino wears a visible skull helmet. When hit, the shield shatters with screen shake and spark particles, destroying the obstacle and keeping you alive! |
| **Bullet Time (Clock)** | ⏱️ Golden Glow Circle | **Slow-motion for 5.0 seconds.** | Scales the simulation time step ($\Delta t_{\text{eff}} = 0.45 \cdot \Delta t$). Obstacles and physics move in slow motion, giving you time to dodge tight obstacles. |
| **Double Jump (Feather)** | 🪶 Magenta Glow Circle | **Grants 3 mid-air jump charges.** | Allows the Dino to jump again while airborne. Perfect for escaping mistimed jumps or flying Pterodactyls. |

---

# 2. Score Milestone Cosmetics (Automatic Unlocks)
As your score increases during a run, the Dino dynamically equips retro accessories:

* **Score $\ge$ 500:** **Pixel Sunglasses 😎**  
  The Dino automatically wears cool shades on its eyes.
* **Score $\ge$ 1000:** **Party Hat 🥳**  
  A colorful party cone hat is rendered on top of the Dino's head.
* **Score $\ge$ 2000:** **Golden Dino Skin ✨**  
  The dinosaur's entire sprite turns into metallic reflective gold.

---

# 3. Dynamic Weather System (`Weather.cpp`)
You can cycle the weather manually or let it evolve naturally during the run:

1. **Clear Day / Night:** Standard desert atmosphere.
2. **Rain:** Individual raindrops are drawn as diagonal sloped lines using Bresenham's algorithm. When drops hit the ground baseline, they spawn miniature expanding circular splash ripples.
3. **Snow:** Flakes drift gently downward with independent horizontal sinusoidal sway.
4. **Thunderstorm:** The sky darkens, accompanied by procedurally generated branching lightning bolts that flash the screen white, followed by a procedural low-frequency rumble sound.

---

# 4. Day / Night Astronomical Cycle (`Environment.cpp`)
* **Trigger:** Night mode activates cyclically every 700 points (lasting from score 500 to 700).
* **Color Blending:** The background smoothly transitions between Day Gray (`#F7F7F7`) and Night Charcoal (`#202124`) using linear RGB interpolation.
* **24 Twinkling Stars:** Scattered across the sky, pulsating in brightness via sinusoidal phase math ($I = 0.5 + 0.5 \sin(\omega t)$).
* **Shooting Stars / Meteors:** Randomly streak across the night sky at high speed with fading particle trails.
* **Auto-Color Inversion:** All sprites (Dino, cacti, score font) automatically invert their pixels so they contrast against the dark background.

---

# 5. Particle Effects System (`ParticleSystem.cpp`)
* **Running Footstep Dust:** Small puffs of smoke puff behind the Dino's alternating left and right feet every $160\text{ ms}$ while sprinting.
* **Jump & Landing Dust:** Burst of ground dust whenever the Dino jumps or lands.
* **Collision Sparks:** An explosive radial spray of high-velocity sparks when hitting an obstacle or shattering a shield.

---

# 6. Pro-Level Platformer Controls & Kinematics (`Dino.cpp`)
Your Dino includes kinematics features found in professional games like *Celeste*:

* **Jump Buffering ($120\text{ ms}$ window):** If you press Jump slightly *before* landing, the game remembers the input and jumps on the exact frame your feet touch the ground. No missed jumps!
* **Coyote Time ($80\text{ ms}$ window):** If you walk off an edge, you still have an 80ms grace window to press Jump.
* **Fast-Fall / Fast-Duck:** Pressing the `DOWN` arrow key while airborne multiplies gravity to $4200\text{ px/s}^2$, slamming the Dino downward for fast landings.
* **Airborne Pitch / Tilt:** As the Dino jumps upward, it tilts slightly backward; as it falls downward, it pitches forward, rendered using real-time sprite rotation (`blitSpriteRotated`).

---

# 7. Obstacle Variety & Combo Hazards
* **Cacti:** Procedurally generated in 5 distinct types (Small Single, Small Double, Small Triple, Big Single, Big Double).
* **Pterodactyls (3 Elevation Tiers):**
  * *High altitude:* Flies safely above the Dino; you can run under it without jumping.
  * *Mid altitude:* Requires you to **Duck** (`DOWN` arrow) to slide underneath.
  * *Low altitude:* Requires you to **Jump** over it!
* **Jump $\to$ Duck Combo (Score $\ge 450$ or Turbo mode):** The engine spawns a small cactus immediately followed by a low Pterodactyl just $175\text{ px}$ behind, requiring a quick **Jump $\to$ Duck** reflex combination.

---

# 8. Visual Filters & Retro CRT Palettes (Press `P` to cycle)
1. **Classic Monochrome:** Original Chrome desert grayscale.
2. **Game Boy DMG-01:** 4-shade nostalgic pea-green LCD display.
3. **Cyberpunk Synthwave:** Neon pink, cyan, and deep purple.
4. **1-Bit Bayer Dithering:** $4 \times 4$ ordered threshold matrix for a vintage Mac / newspaper print look.
5. **Vintage Amber CRT:** Warm orange/amber phosphor glow with scanlines.
6. **Vignette Flashlight Mode:** A dark horror-style spotlight centered on the Dino's head.

---

# 9. UI, Mobile Touch & Settings Modal
* **Virtual Touch Controls:** On-screen Duck pad (bottom-left) and Jump pad (bottom-right) for touchscreen laptops or mobile devices.
* **In-Game Settings Modal:** Click the `[SET]` button in the top-right to toggle weather, palettes, audio mute, CRT scanlines, anti-aliasing, and Turbo Mode without leaving the game.
* **Local High Score Leaderboard:** Automatically tracks and saves your top 5 highest runs across sessions.