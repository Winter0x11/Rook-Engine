# Ruin Engine (C++ 3D with Raylib)

A beginner-friendly 3D graphics & physics engine testbed written in C++ using **Raylib**.

Raylib is the C++ equivalent of `macroquad`: clean, lightweight, and human-readable. It handles windowing and 3D drawing so you can focus on learning C++, 3D vector math, and physics simulation without getting lost in raw OpenGL shader boilerplate.

## Features
- **Spherical Orbit Camera**: Orbit around an arbitrary 3D target point with radius, height, azimuth, and rotation frequency.
- **Dynamic Planar Ground Shadows**: Real-time shadows cast onto the teal floor based on directional light angles.
- **Stacked 3D Cubes**: 5 stacked boxes with slight rotational twists.
- **Teal Floor & 3D Grid**: Anti-aliased 3D wireframe grid floor that can be toggled on/off.
- **Immediate-Mode GUI ("Point orbit GUI")**: Real-time sliders and checkboxes for radius, frequency, height, target coordinates, and light directions.

## Project Structure
```text
engine/
├── src/
│   ├── main.cpp     # 3D scene, orbit camera math, and game loop (with friendly comments)
│   └── gui.hpp      # Lightweight immediate-mode GUI ("Point orbit GUI")
├── Makefile         # Fast single-command compilation (make run)
├── CMakeLists.txt   # Standard CMake file for IDEs (CLion, VS Code)
└── README.md
```

## Running the Project

### Fast Build with Make (Recommended)
```bash
make
./engine_demo
```
Or directly:
```bash
make run
```

### Build with CMake
```bash
mkdir -p build && cd build
cmake ..
make
./engine_demo
```

## Interactive Controls
- **Left Click & Drag (in viewport)**: Manually orbit the camera around the tower.
- **GUI Controls (top-left panel)**:
  - **Play**: Toggle continuous orbital camera rotation.
  - **Show grid**: Toggle ground grid on/off.
  - **radius**: Camera distance from orbit target.
  - **frequency**: Rotation speed when auto-orbiting.
  - **height**: Camera elevation above ground.
  - **orbit point**: Coordinates (X, Y, Z) of the center target.
  - **Direction...**: Directional light azimuth and elevation angles (adjusts ground shadow).
- **Escape**: Close the window.

## Requirements
- C++17 compiler (`g++` or `clang++`)
- Raylib 6.0+ (`libraylib.so`)
