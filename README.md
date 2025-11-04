# OpenGL Renders

This repository showcases a collection of visually engaging and mathematically inspired OpenGL renderings, written entirely in **C++17**.  
Each module demonstrates a unique concept or effect, such as black holes, fractals, and wave simulations, using low-level rendering techniques.

The goal of this project is to explore advanced rendering ideas, simulation logic, and efficient use of modern OpenGL — purely for creative experimentation.

---

## Features

- Modular CMake-based project structure  
- Cross-platform (Linux and Windows)  
- Real-time rendering using OpenGL  
- Examples include:
  - Black Hole Simulation  
  - Fractal Visualization  
  - Wave Simulation  

---

## Build and Run

### 1. Quick Start (Recommended)
Simply run:
```bash
bash build.sh
```

This script handles configuration, compilation, and execution automatically.

### 2. Manual Method

If you prefer running commands manually:
```bash
chmod +x build.sh
./build.sh
```

Development Notes

    Language Standard: C++17

    Build System: CMake

    Dependencies:

        GLFW

        GLEW

        GLM

All dependencies are automatically installed when using the provided CI workflow or vcpkg.
Contributing

Contributions, corrections, and new render ideas are welcome.
You can open a pull request, report an issue, or suggest a new rendering concept.
License

This project is open-source and available under the MIT License.

If you find the simulations interesting or useful, consider leaving a star to support the project.