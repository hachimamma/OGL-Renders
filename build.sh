#!/bin/bash

echo "What do you want to render?"
echo "1) Black Hole"
echo "2) Fractals"
echo ""
read -p "Choose (1 or 2): " choice

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

        echo "WASD - Pan Around"
        echo "Q/E - Zoom In/Zoom Out"
        echo "SPACE - Toggle Mandelbrot-Julia"
        echo "Mousewheel - Zoom In/Zoom Out"
        echo "R - Reset position"
        echo "ESC - Exit"

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
echo "Press ESC to exit the program"
echo ""
./$exe

cd ../..