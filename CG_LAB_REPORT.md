# 📄 Computer Graphics Laboratory Report & Viva Defense Guide

**Project Title:** Implementation of Chrome Dino Game using a Pure Software Raster Graphics Engine in Qt/C++  
**Subject:** Computer Graphics Laboratory  
**Evaluation Standard:** 100/100 Distinction Demonstration  

---

## 1. Objective & Scope

The objective of this laboratory assignment is to design, implement, and demonstrate a complete 2D side-scrolling platformer ("Chrome Dino") exclusively using **Raster Graphics** primitives, strictly omitting high-level vector graphics libraries, vector paths (`QPainterPath`, SVG), or OpenGL vector pipelines.

---

## 2. Theoretical Distinction: Raster vs. Vector Graphics

| Feature | Vector Graphics | Pure Raster Graphics (Implemented Here) |
|---|---|---|
| **Representation** | Mathematical formulas, parametric equations, control points | 2D rectangular matrix of discrete color values (Pixels) |
| **Storage Structure** | Command lists, vertex arrays, spline control points | Framebuffer (`uint32_t pixels[W * H]`) |
| **Rasterization** | Offloaded to hardware/graphics driver or vector library | Custom software algorithms running directly on the CPU |
| **Scaling** | Resolution-independent (reevaluated curves) | Discrete pixel resampling (Nearest-Neighbor, Bilinear) |
| **Compliance** | **Violates Lab Rubric** | **100% Compliant with Syllabus Requirements** |

---

## 3. Mathematical Derivations of Implemented Algorithms

### 3.1. Bresenham's Integer Line Drawing Algorithm
- **Location in Code:** `src/graphics/SoftwareRasterizer.cpp` (`drawLineBresenham`)
- **Usage:** Ground horizon line, terrain pebbles, star sparkle glints, collision bounding boxes, wireframe line showcase.
- **Mathematical Principle:**
  Given two endpoints $(x_0, y_0)$ and $(x_1, y_1)$:
  $$\Delta x = |x_1 - x_0|, \quad \Delta y = |y_1 - y_0|$$
  Direction signs:
  $$s_x = \text{sgn}(x_1 - x_0), \quad s_y = \text{sgn}(y_1 - y_0)$$
  Decision variable is initialized without floating-point division:
  $$\text{err} = \Delta x - \Delta y$$
  At each iterative step:
  $$e_2 = 2 \cdot \text{err}$$
  $$\text{if } e_2 > -\Delta y: \quad \text{err} \leftarrow \text{err} - \Delta y, \quad x \leftarrow x + s_x$$
  $$\text{if } e_2 < \Delta x: \quad \text{err} \leftarrow \text{err} + \Delta x, \quad y \leftarrow y + s_y$$
- **Advantages:** Eliminates floating-point division ($m = \Delta y / \Delta x$) and multiplication entirely, executing purely through bitwise shifts and integer additions.

---

### 3.2. Midpoint Circle Algorithm (Bresenham's Circle)
- **Location in Code:** `src/graphics/SoftwareRasterizer.cpp` (`drawCircleMidpoint`, `fillCircleMidpoint`)
- **Usage:** Moon, Sun, celestial bodies, dust puff particles, Dino eye, interactive sandbox.
- **Mathematical Principle:**
  Circle function:
  $$F(x, y) = x^2 + y^2 - R^2$$
  Starting at $(0, R)$, initial decision parameter:
  $$P_0 = F\left(1, R - \frac{1}{2}\right) = 1^2 + \left(R - \frac{1}{2}\right)^2 - R^2 = \frac{5}{4} - R \approx 1 - R \quad (\text{for integer math})$$
  Recurrence relation:
  - If $P_k < 0$: Midpoint is inside circle $\implies$ select pixel $E = (x_k + 1, y_k)$:
    $$P_{k+1} = P_k + 2x_{k+1} + 1 = P_k + 2x_k + 3$$
  - If $P_k \ge 0$: Midpoint is outside circle $\implies$ select pixel $SE = (x_k + 1, y_k - 1)$:
    $$P_{k+1} = P_k + 2x_k - 2y_k + 5$$
  - **8-Way Symmetry:** Any calculated point $(x, y)$ in the first octant is simultaneously plotted across all 8 octants:
    $$(\pm x, \pm y) \quad \text{and} \quad (\pm y, \pm x)$$
- **Proof in Sandbox Mode (`F2`):** Mode 2 colors each of the 8 octants in a different color to visibly prove 8-way symmetry to the examiner!

---

### 3.3. Midpoint Ellipse Algorithm
- **Location in Code:** `src/graphics/SoftwareRasterizer.cpp` (`drawEllipseMidpoint`, `fillEllipseMidpoint`)
- **Usage:** Parallax clouds, crater depressions, interactive ellipse tester.
- **Mathematical Principle:**
  Ellipse equation:
  $$F(x, y) = r_y^2 x^2 + r_x^2 y^2 - r_x^2 r_y^2 = 0$$
  Divided into two distinct regions based on curve tangent slope:
  - **Region 1:** $\left|\frac{dy}{dx}\right| < 1 \iff 2 r_y^2 x \le 2 r_x^2 y$. Step along $x$.
    $$p_{1,0} = r_y^2 - r_x^2 r_y + \frac{1}{4} r_x^2$$
  - **Region 2:** $\left|\frac{dy}{dx}\right| \ge 1 \iff 2 r_y^2 x > 2 r_x^2 y$. Step along $y$.
    $$p_{2,0} = r_y^2 \left(x + \frac{1}{2}\right)^2 + r_x^2 (y - 1)^2 - r_x^2 r_y^2$$

---

### 3.4. Scanline Polygon Fill Algorithm
- **Location in Code:** `src/graphics/SoftwareRasterizer.cpp` (`fillPolygonScanline`)
- **Usage:** Distant terrain mountains, UI dialogue panels, multi-vertex polygons.
- **Algorithm Steps:**
  1. Find vertical extrema $[y_{\min}, y_{\max}]$ of the polygon vertices.
  2. For each scanline $y \in [y_{\min}, y_{\max}]$:
     - Compute edge intersections with $y$:
       $$x_{\text{intersect}} = x_i + \frac{y - y_i}{y_{i+1} - y_i} (x_{i+1} - x_i)$$
     - Sort intersection points along $x$ in ascending order.
     - Pairwise fill horizontal spans $[x_0, x_1], [x_2, x_3]$ applying the **even-odd parity rule**.

---

### 3.5. Sprite Bitmap Blitting & Alpha Compositing
- **Location in Code:** `src/graphics/SoftwareRasterizer.cpp` (`blitSprite`, `blitSpriteScaled`)
- **Colorkey Transparency:**
  Pixels matching key `0x00000000` are rejected during blitting, preserving background geometry.
- **Alpha Blending (Source-Over Operator):**
  $$C_{\text{out}} = \frac{C_{\text{src}} \cdot A_{\text{src}} + C_{\text{dst}} \cdot (255 - A_{\text{src}})}{255}$$
- **Nearest-Neighbor Resampling:**
  For arbitrary integer scale factor $S$:
  $$\text{Pixel}_{\text{dst}}(x, y) = \text{Pixel}_{\text{src}}\left(\left\lfloor \frac{x}{S} \right\rfloor, \left\lfloor \frac{y}{S} \right\rfloor\right)$$

---

### 3.6. Xiaolin Wu's Anti-Aliased Line Algorithm
- **Location in Code:** `src/graphics/SoftwareRasterizer.cpp` (`drawLineXiaolinWu`)
- **Usage:** Rain streaks, horizon smoothing, and interactive algorithm sandbox (Mode 6).
- **Mathematical Principle:**
  Unlike Bresenham's algorithm which rounds each coordinate to the nearest single discrete integer pixel (causing visible staircasing / jaggies), Xiaolin Wu's algorithm weights pixel intensity according to its **fractional distance** from the mathematical line:
  1. Determine driving axis ($dx > dy$ vs steep $dy > dx$).
  2. Compute line gradient:
     $$\text{gradient} = \frac{\Delta y}{\Delta x}$$
  3. At each discrete coordinate along the major axis, calculate the exact real-valued intersection coordinate:
     $$\text{intery} = y_0 + \text{gradient} \cdot (x - x_0)$$
  4. Decompose $\text{intery}$ into integer part and fractional part:
     $$\text{ipart} = \lfloor \text{intery} \rfloor, \quad \text{fpart} = \text{intery} - \text{ipart}$$
  5. Plot two vertically (or horizontally) adjacent pixels simultaneously with complementary alpha transparency weights:
     $$\text{Pixel}(x, \text{ipart}) \leftarrow \text{color} \times (1 - \text{fpart})$$
     $$\text{Pixel}(x, \text{ipart} + 1) \leftarrow \text{color} \times (\text{fpart})$$
- **Examiner Proof:** Press `A` in game to toggle between Bresenham and Xiaolin Wu in real-time, or open Sandbox Mode 6 (`F2`) to inspect side-by-side zoomed pixel matrices!

---

### 3.7. 2D Affine Transformation (Software Raster Rotation via Inverse Mapping)
- **Location in Code:** `src/graphics/SoftwareRasterizer.cpp` (`blitSpriteRotated`)
- **Usage:** Dino velocity-based aerodynamic tilting during jumps and falls, and interactive Sandbox Mode 7.
- **Mathematical Principle (Inverse Mapping vs Forward Mapping):**
  - **Forward Mapping Problem:** If we compute $(x_{\text{dst}}, y_{\text{dst}}) = R(\theta) \cdot (x_{\text{src}}, y_{\text{src}})$, integer rounding leaves unmapped "holes" and disconnected gaps in the destination buffer.
  - **Inverse Mapping Solution:** We loop through every pixel in the destination bounding box $(x_{\text{dst}}, y_{\text{dst}})$ and back-project to find its corresponding point in the source sprite:
    $$\begin{bmatrix} x_{\text{src}} \\ y_{\text{src}} \end{bmatrix} = \begin{bmatrix} \cos(-\theta) & -\sin(-\theta) \\ \sin(-\theta) & \cos(-\theta) \end{bmatrix} \begin{bmatrix} x_{\text{dst}} - c_x \\ y_{\text{dst}} - c_y \end{bmatrix} + \begin{bmatrix} c_{\text{src}, x} \\ c_{\text{src}, y} \end{bmatrix}$$
  - If $(x_{\text{src}}, y_{\text{src}})$ lies within sprite dimensions and is solid, we write to the framebuffer.
- **Examiner Proof:** Zero OpenGL or hardware affine matrices are used; pure integer/floating point inverse mapping in CPU RAM!

---

## 4. Viva Questions & Model Answers (Examiner Defense)

### Q1: "How can you prove you didn't just use Qt's vector painting functions?"
**Answer:**  
> "Sir, if you inspect `SoftwareRasterizer.cpp` and `MainWindow.cpp`, we do not instantiate `QPainterPath` or call `painter.drawLine()`. We maintain our own continuous block of heap memory: `std::vector<uint32_t> m_pixels` representing a 32-bit ARGB Framebuffer. Every line is rasterized into this buffer using our own Bresenham loop. At the end of each frame, `QPainter::drawImage` simply performs a single 1:1 memory copy of this pixel buffer to the window surface."

### Q2: "Can you show me your Bresenham line algorithm running in real time?"
**Answer:**  
> "Yes sir! Press **`F1`** in the game, and you will see the **CG Lab Inspector HUD**. It tracks the exact count of Bresenham lines drawn per frame and includes a 12x12 magnified pixel zoom. Furthermore, if you press **`F2`**, it opens the **Algorithm Sandbox** where you can click and drag line endpoints to see the step-by-step decision variable $err$, slope $m$, and all octants tested interactively!"

### Q3: "How does your Midpoint Circle algorithm handle 8-way symmetry?"
**Answer:**  
> "In `SoftwareRasterizer::drawCircleMidpoint`, we evaluate only the first octant where $x \le y$. In each step, we invoke the `plot8()` lambda which reflects the point across the axes and diagonals: $(x, y)$, $(y, x)$, $(-x, y)$, $(-y, x)$, $(-x, -y)$, $(-y, -x)$, $(x, -y)$, and $(y, -x)$. In our Algorithm Sandbox (`F2` Mode 2), each of these 8 octants is rendered in a different color so you can visually verify the 8-way symmetry in action!"

### Q4: "How does collision detection work without vector geometry?"
**Answer:**  
> "We implement a dual-phase system: First, a broadphase AABB bounding-box overlap test. If the bounding boxes intersect, we run a narrowphase **Pixel-Perfect Raster Bitmask Collision** (`checkPixelCollision` in `Entity.h`). It checks if both sprites contain a solid, non-transparent pixel at the exact same world coordinate. If a collision occurs, the Inspector crosshair marks the exact $(x, y)$ colliding pixel!"

### Q5: "How does Xiaolin Wu's Anti-Aliasing differ mathematically from Bresenham?"
**Answer:**  
> "Bresenham's algorithm treats rasterization as a binary decision problem: a pixel is either on or off, leading to staircasing (jaggies). Xiaolin Wu treats rasterization as an area-coverage / sub-pixel weighting problem: for any non-integer position, it decomposes the coordinate into integer part $\lfloor y \rfloor$ and fractional part $\text{fpart}$, and splits the pixel's energy between two neighbor pixels with weights $(1 - \text{fpart})$ and $\text{fpart}$. This creates visually smooth edges without increasing screen resolution."

### Q6: "Why must 2D Software Raster Rotation use Inverse Mapping instead of Forward Mapping?"
**Answer:**  
> "In forward mapping, multiplying discrete source coordinates $(x, y)$ by the rotation matrix produces floating-point coordinates that round to destination pixels. Because rotation stretches distances along diagonals, certain destination pixels will never be targeted, resulting in artificial black 'holes' and moiré artifacts. Inverse mapping iterates through every destination pixel and back-projects it to the source sprite, ensuring every destination pixel is filled without gaps."
