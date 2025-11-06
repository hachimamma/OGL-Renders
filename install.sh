#!/usr/bin/env bash
set -e

echo "=== Setting up OGL-Renders (Linux/macOS) ==="

if ! command -v cmake &> /dev/null; then
  echo "Installing CMake..."
  sudo apt-get update
  sudo apt-get install -y cmake
fi

echo "Installing OpenGL dependencies..."
sudo apt-get install -y g++ gcc libglfw3-dev libglew-dev libglm-dev \
  libx11-dev libxi-dev libxxf86vm-dev libxrandr-dev libxinerama-dev libxcursor-dev

echo "=== Building Project ==="
chmod +x build.sh
./build.sh

echo "Setup complete! You can now run your program from the build directory."
