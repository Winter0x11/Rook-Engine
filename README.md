# Rook Engine

A small 3D engine experiment built with **C++17** and **Raylib 6**.

Rook Engine is currently a learning-focused 3D rendering and interaction testbed. It explores the foundations of a real-time engine: 3D transforms, camera movement, scene data, drawing, input handling, and a lightweight immediate-mode GUI.

> **Current scope:** this repository is not yet a general-purpose physics engine. The scene currently renders a manually defined stack of cubes; rigid-body simulation, collision detection, and a physics solver have not been implemented.

## Current Features

- **3D scene rendering** — a five-block tower rendered with Raylib.
- **Orbit camera** — automatic rotation around a configurable target, plus mouse-drag camera control.
- **Interactive controls** — a small custom immediate-mode GUI implemented using Raylib primitives.
- **Camera parameters** — adjust orbit radius, rotation frequency, camera height, and target position at runtime.
- **Lighting direction controls** — change the direction used by the ground-shadow projection.
- **Projected ground shadows** — flattened cube silhouettes are drawn onto the floor. This is a simple visual approximation, not a shadow-mapping or physically based lighting system.
- **Toggleable 3D grid** — show or hide the ground reference grid.
- **CMake and Make builds** — two ways to compile the testbed.

## Preview Scene

The current demo contains a teal ground plane, a stack of five slightly rotated cubes, projected shadows, a 3D grid, and an on-screen control panel.

## Project Structure

```text
Rook-Engine/
├── src/
│   ├── main.cpp       # Scene setup, camera, rendering, input, main loop
│   └── gui.hpp        # Lightweight immediate-mode GUI controls
├── Makefile           # Make-based build and run targets
├── CMakeLists.txt     # CMake project configuration
├── .gitignore
└── README.md
```

## Requirements

- C++17 compiler (GCC or Clang)
- [Raylib 6.0 or later](https://www.raylib.com/)
- GNU Make (for the Makefile workflow)
- CMake 3.16 or later (optional, for the CMake workflow)

Raylib must be installed and discoverable by your compiler and linker.

## Build and Run

### Using Make

Build:

```bash
make
```

Run:

```bash
make run
```

Or run the executable directly:

```bash
./engine_demo
```

### Using CMake

From the repository root:

```bash
cmake -S . -B build
cmake --build build
./build/engine_demo
```

On Windows, run the generated executable from the build directory (for example, `build\Debug\engine_demo.exe` with a Visual Studio generator).

## Controls

- **Left-click and drag in the 3D viewport** — rotate the camera around the scene.
- **Play** — toggle automatic camera rotation.
- **Show grid** — toggle the ground grid.
- **radius** — change the camera's distance from the orbit target.
- **frequency** — change automatic orbit speed.
- **height** — change camera elevation.
- **orbit point** — adjust the camera's target coordinates (X, Y, Z).
- **Direction controls** — adjust the azimuth and elevation used for the projected ground shadows.
- **Escape** — close the application.

The GUI captures mouse interaction while the pointer is over its panel, preventing GUI clicks and drags from rotating the camera.

## Development Notes

The current implementation keeps the demo deliberately small:

- The scene's blocks are static data, not simulated rigid bodies.
- The camera and light direction are calculated from spherical angles.
- The GUI is custom code built on Raylib drawing and input functions; it does not depend on Dear ImGui.
- The ground shadows are hand-projected silhouettes and do not represent a general-purpose lighting or shadow system.

## Roadmap

Potential next steps as the project grows:

1. Separate rendering, camera, input, and scene code into focused modules.
2. Introduce reusable transform and scene-object types.
3. Add a real-time update architecture with a fixed-step simulation loop.
4. Implement particle and rigid-body dynamics.
5. Add collision detection and a constraint/impulse solver.
6. Explore a more capable renderer, materials, and proper shadow techniques.

These are future directions, not features currently available.

## License

No license file is currently present in the repository. Until a license is added, assume the project is not licensed for reuse or redistribution.
