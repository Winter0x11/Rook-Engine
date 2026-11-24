#include "raylib.h"
#include "raymath.h"
#include "rlgl.h" // Raylib's friendly OpenGL helper (lets us push/pop matrices easily)
#include <cmath>
#include <vector>

#include "gui.hpp"

//
// Hey! Welcome to your C++ 3D starter engine.
//
// We are using Raylib because it takes care of all the annoying OpenGL plumbing
// (window creation, GLSL shader compilation, buffer pointers) so you can focus
// on what actually matters: 3D math, camera controls, and physics!
//

// A simple struct describing each block in our 5-cube tower
struct Block {
    Vector3 position; // Where the center of the block is in 3D world space
    Vector3 size;     // Width (X), Height (Y), Depth (Z)
    float rotationY;  // Rotation around the vertical Y axis (in degrees)
    Color color;      // Warm amber/terracotta color for the block
};

int main() {
    // 1. Open the game window
    const int screenWidth = 1280;
    const int screenHeight = 720;
    SetConfigFlags(FLAG_MSAA_4X_HINT); // 4x anti-aliasing for smooth edges
    InitWindow(screenWidth, screenHeight, "Point Orbit Engine (Raylib)");
    SetTargetFPS(60); // Lock simulation to a smooth 60 FPS

    // 2. Set up our Orbit Camera parameters (matching the screenshot)
    Vector3 orbitPoint = { 0.0f, 4.225f, 0.0f }; // Center point the camera stares at
    float radius = 13.598f;                      // How far the camera is from the center
    float height = 10.600f;                      // Camera height above ground
    float frequency = 0.300f;                   // How fast the camera spins when playing
    float azimuth = 45.0f;                       // Current camera angle in degrees
    bool play = true;                            // True = auto spin camera, False = paused
    bool showGrid = true;                        // Toggle the 3D grid floor on/off

    // Directional light angles (controls the dynamic shadow on the ground)
    float lightAzimuth = 124.335f;
    float lightElevation = 52.488f;

    // Raylib's 3D Camera structure
    Camera3D camera{};
    camera.target = orbitPoint;
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f }; // Y is up in 3D space
    camera.fovy = 45.0f;                     // Field of view in degrees
    camera.projection = CAMERA_PERSPECTIVE;

    // 3. Define the 5 stacked blocks in the tower
    // Notice how each block sits neatly on top of the previous one with a slight twist!
    std::vector<Block> tower = {
        { { 0.0f, 0.85f, 0.0f }, { 2.6f, 1.7f, 2.6f },   0.0f, Color{ 175, 110,  55, 255 } },
        { { 0.0f, 2.55f, 0.0f }, { 2.6f, 1.7f, 2.6f },  18.0f, Color{ 165, 105,  50, 255 } },
        { { 0.0f, 4.25f, 0.0f }, { 2.6f, 1.7f, 2.6f }, -14.0f, Color{ 180, 115,  60, 255 } },
        { { 0.0f, 5.95f, 0.0f }, { 2.6f, 1.7f, 2.6f },  25.0f, Color{ 160, 100,  45, 255 } },
        { { 0.0f, 7.65f, 0.0f }, { 2.6f, 1.7f, 2.6f },  -6.0f, Color{ 185, 120,  65, 255 } }
    };

    // Beautiful teal floor color from your screenshot
    Color tealFloor = { 15, 100, 110, 255 };

    // Track mouse dragging for manual camera rotation
    bool isDraggingMouse = false;
    float lastMouseX = 0.0f;

    // -------------------------------------------------------------
    // Main Game / Engine Loop (Runs 60 times every second)
    // -------------------------------------------------------------
    while (!WindowShouldClose()) {
        float dt = GetFrameTime(); // Seconds since the last frame (e.g. ~0.016s at 60fps)

        // --- Step A: Update Camera Position ---
        if (play) {
            // Spin the camera around automatically
            azimuth += frequency * 360.0f * dt;
            if (azimuth >= 360.0f) azimuth -= 360.0f;
        }

        // Allow manual mouse dragging when the user isn't clicking a GUI button
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !gui::IsMouseBusy()) {
            isDraggingMouse = true;
            lastMouseX = mouse.x;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isDraggingMouse = false;
        }
        if (isDraggingMouse && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            float dx = mouse.x - lastMouseX;
            lastMouseX = mouse.x;
            azimuth -= dx * 0.4f; // Rotate based on mouse drag speed
        }

        // Convert our spherical angles (azimuth, radius, height) into a 3D (X, Y, Z) point!
        float rad = azimuth * DEG2RAD;
        camera.position.x = orbitPoint.x + radius * std::sin(rad);
        camera.position.y = height;
        camera.position.z = orbitPoint.z + radius * std::cos(rad);
        camera.target = orbitPoint;

        // Calculate the light direction vector from the GUI azimuth/elevation angles
        float lAzRad = lightAzimuth * DEG2RAD;
        float lElRad = lightElevation * DEG2RAD;
        Vector3 lightDir = {
            std::cos(lElRad) * std::sin(lAzRad),
            std::sin(lElRad),
            std::cos(lElRad) * std::cos(lAzRad)
        };
        // Normalize so its length is exactly 1.0
        lightDir = Vector3Normalize(lightDir);

        // --- Step B: Start Drawing ---
        BeginDrawing();
            // Clear screen to dark charcoal/slate
            ClearBackground(Color{ 8, 12, 18, 255 });

            // ---------------------------------------------------------
            // 3D Rendering Pass
            // ---------------------------------------------------------
            BeginMode3D(camera);

                // 1. Draw the Teal Ground Floor
                DrawPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector2{ 50.0f, 50.0f }, tealFloor);

                // 2. Draw Dynamic Ground Shadows
                // Super intuitive shadow: if light comes from the right, shadow falls to the left!
                // The higher up the block is (block.position.y), the further the shadow stretches.
                Color shadowColor = Color{ 5, 45, 50, 190 }; // Dark silhouette on the teal floor

                for (const auto& block : tower) {
                    rlPushMatrix();
                        float shadowX = block.position.x - lightDir.x * (block.position.y * 0.6f);
                        float shadowZ = block.position.z - lightDir.z * (block.position.y * 0.6f);

                        rlTranslatef(shadowX, 0.002f, shadowZ);
                        rlRotatef(block.rotationY, 0.0f, 1.0f, 0.0f);

                        // Draw flattened shadow box right on the floor
                        DrawCube(Vector3{ 0.0f, 0.0f, 0.0f }, block.size.x, 0.005f, block.size.z, shadowColor);
                    rlPopMatrix();
                }

                // 3. Draw the 5 Stacked Blocks
                for (const auto& block : tower) {
                    rlPushMatrix();
                        rlTranslatef(block.position.x, block.position.y, block.position.z);
                        rlRotatef(block.rotationY, 0.0f, 1.0f, 0.0f);

                        // Draw the solid block
                        DrawCube(Vector3{ 0.0f, 0.0f, 0.0f }, block.size.x, block.size.y, block.size.z, block.color);

                        // Draw slightly darker outlines for that clean architectural look
                        DrawCubeWires(Vector3{ 0.0f, 0.0f, 0.0f }, block.size.x, block.size.y, block.size.z, Color{ 70, 40, 20, 180 });
                    rlPopMatrix();
                }

                // 4. Draw the 3D Grid floor (if enabled)
                if (showGrid) {
                    DrawGrid(25, 1.0f);
                }

            EndMode3D();

            // ---------------------------------------------------------
            // 2D Immediate-Mode GUI Pass (Drawn on top of 3D)
            // ---------------------------------------------------------
            gui::Begin("Point orbit GUI", 15.0f, 15.0f, 240.0f);
                gui::Checkbox("Play", &play);
                gui::Checkbox("Show grid", &showGrid);
                gui::SliderFloat("radius", &radius, 1.0f, 35.0f);
                gui::SliderFloat("frequency", &frequency, 0.0f, 2.0f);
                gui::SliderFloat("height", &height, -5.0f, 30.0f);
                gui::DragFloat3("orbit point", &orbitPoint.x, &orbitPoint.y, &orbitPoint.z, -20.0f, 20.0f);
                gui::SliderFloat("Direction...", &lightAzimuth, 0.0f, 360.0f);
                gui::SliderFloat("Direction...", &lightElevation, 10.0f, 85.0f);
            gui::End();

        EndDrawing();
    }

    // 4. Cleanup when window closes
    CloseWindow();
    return 0;
}
