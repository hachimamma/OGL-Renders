# OpenGL Renders

This repository showcases a collection of visually striking and mathematically inspired **OpenGL renderings**, written entirely in **C++17**.
Each module demonstrates a unique concept or simulation - ranging from **black holes** to **fractals** and **wave dynamics**, using low-level, modern OpenGL techniques.

The goal of this project is to explore graphics programming and mathematical visualization through efficient, modular, and creative render implementations.

---

## Features

* Modular **CMake-based** build system
* Fully **cross-platform** (Linux & Windows)
* **Real-time** OpenGL simulations
* Organized by category (Blackhole, Fractals, Waves, etc.)
* Automatic dependency installation (via vcpkg or setup script)

---

## Available Simulations

* **Black Hole Simulation** - Light distortion near an event horizon
* **Fractal Visualization** - Mandelbrot and complex pattern generation
* **Wave Simulation** - Procedural surface and ripple animation

---

## Build and Run

### **1. Easiest Setup (Recommended)**

For a one-step setup and build process:

#### Linux / macOS

```bash
bash install.sh
```
or
```bash
chmod +x install.sh
./install.sh
```

#### Windows (PowerShell)

```powershell
.\install.ps1
```

Both scripts automatically install required dependencies and compile the project.

---

### **2. Quick Build (Manual)**

If you already have dependencies installed, simply run:

```bash
bash build.sh
```

Or manually:

```bash
chmod +x build.sh
./build.sh
```

---

## Development Notes

* **Language Standard:** C++17
* **Build System:** CMake
* **Dependencies:**

  * [GLFW](https://www.glfw.org/) — Window and input management
  * [GLEW](http://glew.sourceforge.net/) — OpenGL extension handling
  * [GLM](https://github.com/g-truc/glm) — Mathematics library for graphics

All dependencies are handled automatically through **vcpkg** on Windows or via **APT** on Linux/macOS.

---

## Contributing

Contributions, ideas, and corrections are always welcome!
You can:

* Open a **pull request**
* Report an **issue**
* Suggest a **new rendering concept**

---

## License

This project is open source and distributed under the **MIT License**.

---

### ⭐ If you find the simulations interesting or useful, consider starring the repository to support future development.
