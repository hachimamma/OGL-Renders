#!/bin/bash

echo "What do you want to render?"
echo "1) Black Hole"
echo "2) Fractals"
echo "3) Wave Simulation"
echo ""
read -p "Choose (1-3): " choice

case $choice in
1)
    exe="blackhole"
    title="Blackhole Simulation"
    ;;
2)
    exe="fractal"
    title="Fractal Zooms"
    echo ""
    echo "Controls:"
    echo ""
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
    exe="waves"
    title="Waves"
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

if [ ! -d "build" ]; then
    echo "Creating root build directory..."
    mkdir build
fi

cd build || { echo "Error: Cannot access build directory!"; exit 1; }

echo "Running CMake..."
cmake .. || { echo "CMake failed!"; exit 1; }

echo "Compiling..."
make -j$(nproc) || { echo "Build failed!"; exit 1; }

echo "Launching $title..."
case $exe in
    blackhole)
        ./blackhole/blackhole
        ;;
    fractal)
        ./fractal-zoom/fractal
        ;;
    waves)
        ./waves/waves
        ;;
esac