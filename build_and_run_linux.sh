#!/usr/bin/env bash
set -e

echo "=========================================================="
echo "  Chrome Dino: Pure Software Raster Engine (Linux Build)  "
echo "=========================================================="

# Check and install dependencies if on Debian/Ubuntu/Fedora/Arch
if ! command -v cmake &> /dev/null || ! command -v g++ &> /dev/null; then
    echo "[INFO] Installing build prerequisites..."
    if command -v apt-get &> /dev/null; then
        sudo apt-get update && sudo apt-get install -y cmake g++ qt6-base-dev qt6-multimedia-dev
    elif command -v dnf &> /dev/null; then
        sudo dnf install -y cmake gcc-c++ qt6-qtbase-devel qt6-qtmultimedia-devel
    elif command -v pacman &> /dev/null; then
        sudo pacman -S --noconfirm cmake gcc qt6-base qt6-multimedia
    fi
fi

mkdir -p build-linux
cd build-linux
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

echo ""
echo "=========================================================="
echo "  [SUCCESS] Build complete! Launching Chrome Dino...       "
echo "=========================================================="
./ChromeDinoRaster "$@"
