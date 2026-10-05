# Chrome Dino (Mini Qt Version — Milestone 1)

A clean, beginner-friendly 2D Chrome Dino game built strictly using standard **Qt 6 C++** (`QWidget`, `QPainter`, `QTimer`).

Designed specifically for **Milestone 1 / Preliminary Progress Demonstration**.

---

## 👥 4-Member Group Role Breakdown

When presenting to your professor ("Sir"), each member can take ownership of one distinct section:

| Member | Assigned Role | Functions / Code | Viva Defense Answer (What to say to Sir) |
| :--- | :--- | :--- | :--- |
| **Member 1** | **Qt App & Game Loop Architecture** | `main.cpp`, `gameLoop()`, `QTimer` | *"Sir, I configured the Qt application lifecycle and the 60 FPS non-blocking timer (`QTimer::timeout` at 16ms), which drives the game loop and triggers window repaints."* |
| **Member 2** | **Kinematics & Jump Physics** | `updatePhysics()`, `keyPressEvent()` | *"Sir, I implemented discrete Euler kinematics ($v = v_0 + gt, y = y + v$). Pressing Space or Up applies negative jump velocity, and gravity gradually brings the dinosaur back to the ground."* |
| **Member 3** | **Procedural Obstacles & Collision** | `updateObstacles()`, `checkCollisions()` | *"Sir, I wrote the obstacle spawner using random intervals and dimensions. For collision detection, we check intersection between the dino's hitbox and the cactus bounding box using `QRectF::intersects()`."* |
| **Member 4** | **Graphics & HUD Rendering** | `paintEvent()`, `QPainter` | *"Sir, I implemented the 2D rendering pipeline using `QPainter`. I draw the ground line, the dinosaur silhouette, animated running legs, procedural cacti, score counter, and the Game Over banner."* |

---

## 🚀 How to Run

### Option 1: Open in Qt Creator
* Open Qt Creator $\rightarrow$ Open File or Project $\rightarrow$ Select `mini/ChromeDinoMini.pro`.
* Click **Run (Ctrl + R)**.

### Option 2: 1-Click Batch File (From Windows Explorer)
* Double-click `RUN_MINI.bat` to launch instantly.
* If modifying code, double-click `build_mini.bat` to recompile in 3 seconds.

### Option 3: Portable ZIP (For Lab / Other PCs)
* Extract `ChromeDinoMini_Portable.zip` from your Desktop and double-click `ChromeDinoMini.exe`. No Qt installation required.

---

## 🎮 Controls
* **SPACE** or **UP ARROW**: Jump
* **SPACE** (after Game Over): Restart Game
