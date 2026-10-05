# 🦖 Chrome Dino: Pure Software Raster Graphics Engine

[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/)
[![Qt 6](https://img.shields.io/badge/Qt-6.x-green.svg)](https://www.qt.io/)
[![Pure Software Rasterizer](https://img.shields.io/badge/Graphics-Zero_GPU_Pure_CPU-purple.svg)]()
[![Multi-Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20Android%20%7C%20Web-orange.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> **Computer Graphics Laboratory Project**  
> **Core Constraint:** Pure Software Raster Graphics only — **Zero GPU Shaders, Zero Vector Graphics APIs** (`QPainterPath`, SVG, and OpenGL vector pipelines strictly prohibited).  
> **Architecture:** Every pixel on the $800 \times 300$ screen is computed from first mathematical principles directly in system RAM using fundamental Computer Graphics algorithms.

---

## 🚀 Instant Download & How to Run (Choose Your Platform)

| Platform | Executable / Package | How to Run |
| :--- | :--- | :--- |
| **Windows** | [`ChromeDino.exe`](ChromeDino.exe) | **Double-click to play.** Fully self-contained; requires zero external DLLs, folders, or extraction. |
| **Android Mobile** | [`ChromeDino.apk`](ChromeDino.apk) | **Install directly on phone.** Signed v2/v3 APK. Transfer via WhatsApp or USB and tap to install. |
| **Linux** | [`ChromeDino.run`](ChromeDino.run) | **Single-file executable.** Run `chmod +x ChromeDino.run && ./ChromeDino.run`. |
| **Web Browser** | [`web/index.html`](web/index.html) | **Zero-install instant play.** Double-click `web/index.html` to run in Chrome, Edge, or Firefox. |

---

## 📖 Step-by-Step Running Guides

### 1. Windows Desktop

#### Option A: 1-Click Standalone Executable (No setup needed)
1. Double-click `ChromeDino.exe`.
2. On first launch, the embedded runtime automatically unpacks silently into `%LOCALAPPDATA%\ChromeDinoApp` in $< 0.4\text{ s}$.
3. Subsequent launches start instantly in $< 50\text{ ms}$.

#### Option B: Build From Source (CMake + MinGW / MSVC)
Prerequisites: Qt 6 (Core, Gui, Widgets, Multimedia) and CMake.
```bat
build.bat
```
Or manually via CMake:
```bat
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
.\ChromeDinoRaster.exe
```

---

### 2. Android Mobile

1. **Transfer the APK to your phone:**
   - Send `ChromeDino.apk` to yourself on **WhatsApp Web** as a **Document**, OR connect phone via USB and copy to `Downloads`.
2. **Install on Phone:**
   - Tap `ChromeDino.apk` on your phone $\rightarrow$ select **Install** (enable *"Install unknown apps"* if prompted).
3. **Features on Mobile:**
   - On-screen touch thumb pads for Jump & Duck.
   - Dedicated `[SET]` button coordinate isolation (will not cause accidental jumps when toggling algorithms).
   - Sticky immersive edge-to-edge full-screen display with haptic feedback.

*(To rebuild the APK from source, run `BUILD_ANDROID_APK.bat`).*

---

### 3. Linux Desktop

#### Option A: Single-File Self-Extracting Runner
In your Linux terminal:
```bash
chmod +x ChromeDino.run
./ChromeDino.run
```
The script unpacks itself into `~/.local/share/ChromeDino` and launches the game automatically.

#### Option B: Native 1-Click Build Script
```bash
chmod +x build_and_run_linux.sh
./build_and_run_linux.sh
```
*(Automatically detects Debian/Ubuntu `apt`, Fedora `dnf`, or Arch `pacman`, installs prerequisites, compiles with CMake, and executes).*

---

### 4. Web Browser (Any OS, Zero Installation)
Double-click `web/index.html` or open it with your browser:
```bash
# Optional: run local server
python -m http.server 8080 --directory web
```
Navigate to `http://localhost:8080`. Fully identical physics, pure software raster algorithms, and retro audio synthesis.

---

## 🧠 The 7 Fundamental Computer Graphics Algorithms Implemented

All rendering occurs in `src/graphics/SoftwareRasterizer.cpp` writing directly into a 1D linear array in system RAM (`std::vector<uint32_t>` in `src/graphics/Framebuffer.cpp`).

| # | Algorithm | Implementation File | Mathematical Formula & Characteristics |
| :--- | :--- | :--- | :--- |
| **1** | **Bresenham's Line** | `SoftwareRasterizer.cpp:31` | Integer decision error $err = dx - dy$; handles all 8 octants with zero division or floats. |
| **2** | **Midpoint Circle** | `SoftwareRasterizer.cpp:195` | Evaluates $F(x, y) = x^2 + y^2 - R^2 = 0$; initial $d = 1 - R$; plots all 8 octants simultaneously. |
| **3** | **Midpoint Ellipse** | `SoftwareRasterizer.cpp:277` | 4-way symmetry; divides curve into Region 1 ($|m| < 1$) and Region 2 ($|m| \ge 1$). |
| **4** | **Scanline Polygon Fill** | `SoftwareRasterizer.cpp:410` | Computes edge-scanline intersections, sorts nodes horizontally, fills interior spans via Even-Odd Parity. |
| **5** | **2D Affine Rotation** | `SoftwareRasterizer.cpp:573` | **Inverse Mapping** $\mathbf{x} = \mathbf{R}(-\theta)\mathbf{x}'$ to guarantee solid rotated sprites without gaps or holes. |
| **6** | **Xiaolin Wu Anti-Aliasing** | `SoftwareRasterizer.cpp:109` | Decomposes intercept into integer/fractional parts; plots adjacent pixels with $(1-f, f)$ intensity weights. |
| **7** | **Bitmap Sprite Blitter** | `SoftwareRasterizer.cpp:458` | Alpha compositing $C_{\text{out}} = \frac{C_{\text{src}}A_{\text{src}} + C_{\text{dst}}(255 - A_{\text{src}})}{255}$ with colorkey transparency. |

---

## 🔬 Interactive CG Algorithm Lab & Telemetry (`F2` / `TAB`)

Press **`TAB`** or **`F2`** at any time to freeze the game and open the **Live Algorithm Showcase**:
* **Mode 1 — Bresenham Line:** Drag line endpoints with your mouse to observe slope transitions ($m > 1$, $m < 1$, negative slopes) and decision variable changes in real-time.
* **Mode 2 — Midpoint Circle:** Drag radius handles; inspect 8 octants rendered in 8 distinct colors.
* **Mode 3 — Midpoint Ellipse:** Resize $R_x$ and $R_y$ handles to watch Region 1 and Region 2 boundary updates.
* **Mode 4 — Scanline Polygon Fill:** Step-by-step visual scanline sweeping through concave star geometry.
* **Mode 5 — Sprite Blit & Transparency:** Inspect colorkey transparency masking and alpha blending.
* **Mode 6 — Anti-Aliased Xiaolin Wu:** Side-by-side comparison with Bresenham under a live **12&times;12 discrete pixel magnification grid**.
* **Mode 7 — 2D Affine Rotation:** Live rotating sprite displaying active transformation matrix $\mathbf{R}(\theta)$.

---

## 🎮 Game Controls & Hotkeys

| Key | Action |
| :--- | :--- |
| **Space** / **Up Arrow** / **W** | Jump (variable height: tap for short hop, hold for high leap) |
| **Down Arrow** / **S** | Duck (lowers hitbox height from $47\text{ px} \rightarrow 30\text{ px}$) / Fast-Fall in mid-air |
| **TAB** / **F2** | Toggle CG Algorithm Showcase Lab |
| **F1** | Toggle Real-Time Telemetry HUD & 12&times;12 Pixel Grid Inspector |
| **A** | Toggle Xiaolin Wu Anti-Aliasing on ground and lines |
| **B** | Toggle Procedural 8-bit Chiptune Background Music |
| **N** | Toggle Day / Night Cycle manually |
| **Esc** | Pause Game / Return to Menu |

---

## 📚 Viva Defense & Academic Documentation

Two comprehensive master guides are provided for examination and viva defense:

1. **[`ChromeDino_LineByLine_Defense_Guide.pdf`](ChromeDino_LineByLine_Defense_Guide.pdf)** (13 Pages):
   - **Line-by-line C++ code dissection** with plain-English explanation tables for every algorithm and physics function.
   - Exact 20-second spoken answers for tricky examiner questions (e.g. *"Why is $dy$ negative in Bresenham?"*, *"Why $1-r$ instead of $\frac{5}{4}-r$?"*).
   - Complete memory layout, Euler integration, and 2-tier collision pipeline analysis.
2. **[`ChromeDino_Viva_Defense_Guide.pdf`](ChromeDino_Viva_Defense_Guide.pdf)** (10 Pages):
   - High-Level Design (HLD) architecture diagrams and algorithm derivations.
   - Top 10 viva defense questions with examiner score rubrics.
3. **[`CG_LAB_REPORT.md`](CG_LAB_REPORT.md)**:
   - Full academic laboratory report formatted with theory, mathematical derivations, algorithm pseudo-code, and complexity analysis.

---

## 📁 Repository Structure

```
Dino/
├── src/                               # C++ Pure Software Raster Engine Source
│   ├── graphics/                      # 7 Core CG Algorithms, Framebuffer, Sprites, Font
│   │   ├── Framebuffer.h / .cpp       # Flat 1D RAM buffer (ARGB32), alpha blending
│   │   ├── SoftwareRasterizer.h/.cpp  # Bresenham, Midpoint Circle/Ellipse, Scanline, Wu
│   │   ├── Sprite.h / .cpp            # 2D Bitmaps, colorkey transparency, affine rotation
│   │   ├── SpriteData.h / .cpp        # Pixel-art sprite bit tables (Dino, Cacti, Birds)
│   │   └── Font5x7.h                  # Discrete 5x7 bitmap font rasterizer
│   ├── game/                          # Game Mechanics, Physics & Collision Engine
│   │   ├── Dino.h / .cpp              # Euler physics, Coyote time, Jump buffering, Ducking
│   │   ├── DinoGame.h / .cpp          # Master game loop, 2-tier AABB + bitmask collision
│   │   ├── Obstacle.h / .cpp          # Procedural obstacles (Cacti, animated Pterodactyls)
│   │   ├── Environment.h / .cpp       # Parallax clouds, stars, day/night color interpolation
│   │   ├── Weather.h / .cpp           # Rain, snow, sandstorm particle vector integration
│   │   ├── ParticleSystem.h / .cpp    # Footstep dust, collision sparks, speed trails
│   │   ├── PowerUp.h / .cpp           # Shield armor, Bullet-time slow-mo, Double jump
│   │   └── Achievements.h / .cpp      # Arcade milestones and cosmetics
│   ├── lab/                           # Interactive CG Algorithm Laboratory
│   │   ├── AlgorithmShowcase.h/.cpp   # 7 interactive algorithm sandboxes with handles
│   │   └── LabInspector.h / .cpp      # Live telemetry HUD, 12x12 discrete pixel loupe
│   ├── audio/                         # Procedural 8-bit Sound Engine
│   │   └── RetroAudio.h / .cpp        # Mathematical square-wave synthesizer (zero WAVs)
│   ├── MainWindow.h / .cpp            # Zero-copy Qt presentation, letterbox aspect ratio
│   └── main.cpp                       # Application entry point
├── web/                               # Pure Web / Browser Engine
│   ├── index.html                     # Zero-install browser player (100% feature parity)
│   └── manifest.json                  # Progressive Web App (PWA) manifest
├── android-apk/                       # Native Android Project
│   ├── AndroidManifest.xml            # Full-screen sticky immersive permissions
│   ├── src/.../MainActivity.java      # Hardware-accelerated WebView container
│   ├── res/                           # Icons and XML values
│   └── assets/index.html              # Embedded offline game bundle
├── standalone/                        # Standalone Windows Launcher Source
│   ├── main.cpp                       # Silent unpacker (%LOCALAPPDATA%\ChromeDinoApp)
│   ├── icon.rc                        # Windows PE resource icon script
│   └── icon.ico                       # Embedded 256x256 application icon
├── ChromeDino.exe                     # Standalone Windows Executable (36.4 MB)
├── ChromeDino.apk                     # Standalone Signed Android App (21 KB)
├── ChromeDino.run                     # Standalone Linux Self-Extracting Runner (62 KB)
├── ChromeDino_LineByLine_Defense_Guide.pdf # 13-Page Line-by-Line Code Dissection Manual
├── ChromeDino_Viva_Defense_Guide.pdf       # 10-Page HLD & Viva Defense Guide
├── CG_LAB_REPORT.md                   # Formal Computer Graphics Lab Report
├── CMakeLists.txt                     # Cross-platform CMake build configuration
├── build.bat                          # 1-click Windows compilation script
├── build_and_run_linux.sh             # 1-click Linux compilation script
├── BUILD_ANDROID_APK.bat              # 1-click Android APK build script
└── .gitignore                         # Git exclusion rules for clean repo maintenance
```

---

## 📄 License
This project is open-source under the [MIT License](LICENSE). Built for educational and academic computer graphics study.
