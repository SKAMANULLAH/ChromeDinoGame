# Module 3: Procedural Generation & Multi-Layer Parallax

**Author / Maintainer:** [Friend 2 Name / GitHub Username]  
**Primary Engine Source:** `src/game/Environment.h`, `src/game/Environment.cpp`

## 1. Multi-Layer Parallax Scrolling
Depth is rendered through differential layer speeds:
- Foreground ground line: 100% world velocity
- Midground clouds: 25% world velocity
- Background mountain peaks: 10% world velocity
- Objects wrap back to the right canvas edge when exiting the left edge.

## 2. Procedural Obstacle Spawning
- Dynamic speed scaling accelerates the world as the player's score increases.
- Minimum spawn distance enforced to guarantee every jump is physically possible.
- Procedural selection between small cacti, clustered cacti, tall cacti, and Pterodactyls.