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
