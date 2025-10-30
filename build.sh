#!/bin/bash

echo "Building Black Hole Renderer..."

cd blackhole || { echo "Error: blackhole/ directory not found!"; exit 1; }

if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build || { echo "Error: Cannot access build directory!"; exit 1; }

echo "Running CMake..."
cmake .. || { echo "CMake failed!"; exit 1; }

echo "Compiling..."
make || { echo "Build failed!"; exit 1; }

echo "Launching Black Hole Renderer..."
echo "Press ESC to exit the program"
echo ""
./blackhole

cd ../..