Here is a complete, first-principles explanation of how **Parallax Scrolling** and **Speed Scaling** are implemented in your C++ engine, followed by the **most critical features examiners will ask about during your viva/defense**.

---

# 1. How Parallax Scrolling is Implemented

### The Core Principle
In computer graphics, **Parallax Scrolling** creates an illusion of 3D depth in a flat 2D orthographic display. Objects that are perceived to be farther away appear to move slower across the viewer's retina than objects close to the camera.

Because your engine runs a pure 2D CPU software rasterizer without a 3D Z-buffer, depth is mathematically simulated by translating layers at **fractions of the world velocity**:

```
[Layer 0: Stars / Moon]        -> Speed Scale: 0.00x (Fixed celestial backdrop)
[Layer 1: Far Mountain Peaks]  -> Speed Scale: 0.06x (6% of world speed)
[Layer 2: Mid Desert Dunes]    -> Speed Scale: 0.18x (18% of world speed)
[Layer 3: Clouds]              -> Independent drift (20 to 30 px/s)
[Layer 4: Ground Line & Bumps] -> Speed Scale: 1.00x (100% of world speed)
[Layer 5: Dino & Obstacles]    -> Speed Scale: 1.00x (Active gameplay plane)
```

---

### Exact Implementation in the Code (`src/game/Environment.cpp`)

#### 1. Accumulating Parallax Offsets in `update()`:
```cpp
// Lines 101-103 in Environment.cpp
m_mountainScrollFar += gameSpeed * 0.06f * dt;  // 6% speed for distant mountains
m_mountainScrollMid += gameSpeed * 0.18f * dt;  // 18% speed for mid-distance dunes
m_groundScroll      += gameSpeed * dt;          // 100% speed for ground
```

#### 2. Endless Looping with Modulo Arithmetic:
To make the mountains infinite without generating endless geometry, the offset is wrapped using the modulus operator (`%`) by the width of a single mountain ridge segment ($240\text{ px}$ for far, $180\text{ px}$ for mid):
```cpp
// Lines 143-153 in Environment.cpp (Far Mountains)
int farOffset = static_cast<int>(m_mountainScrollFar) % 240;
for (int bx = -farOffset - 240; bx < m_width + 240; bx += 240) {
    std::vector<QPoint> farPoly = {
        QPoint(bx,       m_groundY),
        QPoint(bx + 70,  m_groundY - 55),
        QPoint(bx + 130, m_groundY - 70),
        QPoint(bx + 180, m_groundY - 45),
        QPoint(bx + 240, m_groundY)
    };
    rasterizer.fillPolygonScanline(farPoly, farMountainCol);
}
```
* **Graphics Algorithm Used:** Each mountain chunk is drawn using **Scanline Polygon Fill** (`fillPolygonScanline`), which calculates edge intersections and fills horizontal pixel spans directly into 1D RAM.

---

# 2. How Speed Increase is Implemented

### The Core Formula (`src/game/DinoGame.cpp`)
The world velocity $v(t)$ scales continuously as the player's score increases, clamped by a strict upper threshold:

$$v(t) = \min\left(v_{\max},\; v_{\text{base}} + \alpha \cdot \text{score}\right)$$

### Exact Code (`src/game/DinoGame.cpp`, lines 480–488):
```cpp
// Speed scaling
float maxSpeed  = m_turboMode ? 950.0f : 800.0f;
float baseSpeed = m_turboMode ? 520.0f : 340.0f;
m_speed = std::min(maxSpeed, baseSpeed + m_score * (m_turboMode ? 0.35f : 0.22f));

// Update score at 10 points per second
m_scoreAcc += 10.0f * effectiveDt;
m_score = static_cast<int>(m_scoreAcc);
```

| Parameter | Normal Mode | Turbo Mode | Meaning |
| :--- | :--- | :--- | :--- |
| **Base Speed ($v_{\text{base}}$)** | $340.0\text{ px/s}$ | $520.0\text{ px/s}$ | Starting velocity when run begins |
| **Speed Acceleration ($\alpha$)** | $+0.22\text{ px/s per pt}$ | $+0.35\text{ px/s per pt}$ | Rate at which speed ramps with score |
| **Terminal Velocity ($v_{\max}$)** | $800.0\text{ px/s}$ | $950.0\text{ px/s}$ | Prevents game from becoming physically impossible |

---

### How Speed Affects the Entire Engine:

1. **Obstacle Translation (`Obstacle.cpp`):**
   $$x_{n+1} = x_n - v_{\text{world}} \cdot \Delta t$$
2. **Ground & Parallax:** Passed directly into `m_env.update(dt, m_speed, m_score)`.
3. **Dynamic Difficulty Adjustment (DDA) on Spawning:**
   As speed increases, the time between obstacle spawns shrinks so obstacles don't feel too sparse:
   ```cpp
   // Line 334 in DinoGame.cpp
   float baseInterval = std::max(1.1f, 1.8f - (m_speed - 340.0f) * 0.0015f);
   m_nextSpawnInterval = baseInterval + jitter(rng);
   ```
4. **Bullet-Time Power-Up:**
   When the player collects the Bullet Time clock, the engine doesn't change `m_speed`; instead, it scales the time step:
   ```cpp
   effectiveDt = dt * 0.45f; // Slow motion at 45% speed
   ```
   All kinematics and translations smoothly slow down without altering physical gravity or velocity constants!

---

# 3. Key Features Examiners Love to Ask About

If the teacher starts probing for first-principles computer graphics knowledge, here are the exact features in your engine and how to answer them:

---

### 🟢 Feature A: Pure Software Framebuffer (1D RAM Indexing)
* **What to tell the teacher:**  
  *"We do not use GPU shaders, OpenGL, DirectX, or Qt’s QPainter for drawing. We allocate a continuous 1D array of 32-bit unsigned integers in heap memory: `uint32_t m_pixels[width * height]`."*
* **The Index Formula:**
  $$\text{index} = y \times \text{width} + x$$
* **Color Packing:** Stored in native ARGB format:
  $$\text{Pixel} = (A \ll 24) \mid (R \ll 16) \mid (G \ll 8) \mid B$$
* **Alpha Blending:** Done in integer arithmetic without floating-point overhead:
  $$\text{Out} = \frac{\text{Src} \cdot \alpha + \text{Dst} \cdot (255 - \alpha)}{255}$$

---

### 🟢 Feature B: Multi-Phase Collision Detection (AABB + Compound Boxes + Pixel Mask)
Many student projects only use 1 bounding box, which causes unfair deaths when jumping over cacti. Your engine uses a **3-tier hierarchical collision pipeline**:

1. **Broadphase:** Quick Axis-Aligned Bounding Box (AABB) intersection test.
   $$\text{Overlap} \iff \max(x_1, x_2) < \min(x_1 + w_1, x_2 + w_2) \ \land \ \max(y_1, y_2) < \min(y_1 + h_1, y_2 + h_2)$$
2. **Midphase (Compound Hitboxes):** When ducking, the dinosaur’s hitbox changes from $44 \times 47\text{ px}$ to $59 \times 30\text{ px}$ to fit underneath flying Pterodactyls. It also tests 3 separate sub-boxes (Head, Body, Tail).
3. **Narrowphase (Pixel-Perfect Mask):** If boxes overlap, `checkPixelCollision()` loops over the overlapping region and compares the 1-bit sprite transparency masks. If and only if two opaque pixels collide, a hit is registered.

---

### 🟢 Feature C: Bresenham & Midpoint Algorithms (Integer Math Only)
* **Lines (`drawLineBresenham`):**
  Uses Bresenham's Line Algorithm with an integer decision accumulator ($D = 2\Delta y - \Delta x$). Zero floating-point multiplication or division is used in the inner loop.
* **Circles (`drawCircleMidpoint`, Stars & Moon):**
  Uses the **Midpoint Circle Algorithm** with 8-way octant symmetry ($(\pm x, \pm y)$ and $(\pm y, \pm x)$).
* **Anti-Aliased Lines:**
  Implemented using **Xiaolin Wu's Algorithm**, which splits line intensity across two adjacent boundary pixels using fractional distance weights for smooth edges.

---

### 🟢 Feature D: Procedural 8-Bit Audio Synthesis in RAM
* **What to tell the teacher:**  
  *"We do not load external `.wav` or `.mp3` files from disk. All audio is procedurally synthesized directly in RAM as 44.1 kHz, 16-bit mono PCM samples."*
* **Jump Sound:** A frequency-modulated square wave sweeping from $300\text{ Hz}$ to $650\text{ Hz}$ over $120\text{ ms}$:
  $$s(t) = A \cdot \operatorname{sgn}\left(\sin(2\pi f(t) t)\right)$$
* **Collision Sound:** A pseudo-random noise generator (LFSR) coupled with an exponential decay volume envelope:
  $$s(t) = \text{rand}(-1, 1) \cdot e^{-\lambda t}$$

---

### 🟢 Feature E: Retro Post-Processing Filters

* **Scanline CRT Emulation:**
  Iterates over every scanline in the framebuffer. Even lines ($y \pmod 2 == 0$) are left unmodified; odd lines ($y \pmod 2 == 1$) have their RGB channels attenuated by $25\%$ using fast bitwise shifts:
  $$\text{Color}_{\text{dim}} = (\text{Color} \gg 1) \ \& \ \text{0x7F7F7F}$$
* **Radial Vignette / Flashlight:**
  Computes the Euclidean distance from the Dino's head to every pixel:
  $$r = \sqrt{(x - x_{\text{dino}})^2 + (y - y_{\text{dino}})^2}$$
  Pixels outside the radius are progressively darkened towards pitch black.


| Feature | Where it is coded | Key Graphic/Math Concept |
| :--- | :--- | :--- |
| **Parallax Scrolling** | `Environment.cpp` | Differential translation speeds ($0.06\times, 0.18\times, 1.0\times$) + modulo wrapping. |
| **Speed Scaling** | `DinoGame.cpp` | Linear score scaling: $v(t) = \min(v_{\max}, v_{\text{base}} + \alpha \cdot \text{score})$. |
| **Rasterizer Core** | `SoftwareRasterizer.cpp` | 1D heap memory buffer, index $= y \cdot W + x$, ARGB 32-bit packing. |
| **Mountain Drawing** | `SoftwareRasterizer.cpp` | Scanline Polygon Fill with Edge Table (ET) & Active Edge Table (AET). |
| **Collision Engine** | `Dino.cpp`, `DinoGame.cpp` | Hierarchical: AABB $\to$ Compound Duck Sub-boxes $\to$ Pixel-mask bit test. |
| **8-Bit Audio** | `RetroAudio.cpp` | Real-time mathematical waveform synthesis (Square wave chirp, LFSR noise burst). |4