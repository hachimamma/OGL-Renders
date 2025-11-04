#!/bin/bash

echo "What do you want to render?"
echo "1) Black Hole"
echo "2) Fractals"
echo "3) Wave Simulation"
echo ""
read -p "Choose (1-3): " choice

case $choice in
1)
    dir="blackhole"
    exe="blackhole"
    title="Black Hole Renderer"
    ;;
2)
    dir="fractal-zoom"
    exe="fractal"
    title="Fractal Explorer"
    echo ""
    echo "Controls:"
    echo "WASD - Pan Around"
    echo "Q/E - Zoom In/Zoom Out"
    echo "SPACE - Toggle Mandelbrot-Julia"
    echo "Mousewheel - Zoom In/Zoom Out"
    echo "Left Click - Jump to location"
    echo "R - Reset position"
    echo "ESC - Exit"
    echo ""
    ;;
3)
    dir="waves"
    exe="waves"
    title="Wave Simulation"
    echo ""
    echo "Controls:"
    echo "Click & Drag - Create ripples"
    echo "SPACE - Random splash"
    echo "R - Clear waves"
    echo "ESC - Exit"
    echo ""
    ;;
*)
    echo "Invalid choice!"
    exit 1
    ;;
esac

echo "Building $title..."
cd $dir || { echo "Error: $dir/ directory not found!"; exit 1; }

if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build || { echo "Error: Cannot access build directory!"; exit 1; }

echo "Running CMake..."
cmake .. || { echo "CMake failed!"; exit 1; }

echo "Compiling..."
make || { echo "Build failed!"; exit 1; }

echo "Launching $title..."
./$exe

cd ../..