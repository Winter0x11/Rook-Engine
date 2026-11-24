#pragma once

#include "raylib.h"
#include <cstdio>
#include <algorithm>

//
// This is a friendly, lightweight Immediate-Mode GUI built purely with Raylib.
//
// In immediate-mode GUI (like Dear ImGui):
// - We don't store widgets in memory or build complex UI trees.
// - Every single frame, we simply say: "draw a slider here, and if the user dragged it, update this float!"
// - It runs in 2D space right after the 3D scene finishes drawing.
//

namespace gui {

// Tracks if the user's mouse is currently interacting with our GUI,
// so we don't accidentally orbit the 3D camera while dragging a slider!
inline bool isMouseCaptured = false;

// Keeps track of the current layout position for widgets as we stack them vertically
inline float panelX = 15.0f;
inline float panelY = 15.0f;
inline float panelW = 240.0f;
inline float cursorY = 0.0f;
inline bool isCollapsed = false;

// Which slider is currently being dragged (pointer to the float being modified)
inline void* activeWidget = nullptr;

// Call this at the start of your 2D draw pass
inline void Begin(const char* title, float x = 15.0f, float y = 15.0f, float width = 240.0f) {
    panelX = x;
    panelY = y;
    panelW = width;
    cursorY = panelY;
    isMouseCaptured = false;

    Vector2 mouse = GetMousePosition();

    // Check if the user is hovering over the title bar
    float titleHeight = 22.0f;
    Rectangle titleRect = { panelX, panelY, panelW, titleHeight };
    bool hoverTitle = CheckCollisionPointRec(mouse, titleRect);

    if (hoverTitle) {
        isMouseCaptured = true;
        // Clicking the title collapses or expands the window
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isCollapsed = !isCollapsed;
        }
    }

    // Draw the dark blue title bar banner
    DrawRectangleRec(titleRect, Color{ 35, 55, 90, 240 });
    DrawRectangleLinesEx(titleRect, 1.0f, Color{ 60, 95, 150, 255 });

    // Little drop-down arrow indicator
    const char* arrow = isCollapsed ? ">" : "v";
    DrawText(TextFormat("%s %s", arrow, title), (int)panelX + 8, (int)panelY + 5, 12, RAYWHITE);

    cursorY += titleHeight + 4.0f;

    // Reset active dragged widget when the mouse button is released
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        activeWidget = nullptr;
    }
}

// A simple toggle checkbox
inline bool Checkbox(const char* label, bool* value) {
    if (isCollapsed) return false;

    Vector2 mouse = GetMousePosition();
    float itemH = 18.0f;
    float boxSize = 12.0f;
    float boxX = panelX + 8.0f;
    float boxY = cursorY + 2.0f;

    Rectangle rowRect = { panelX, cursorY, panelW, itemH };
    bool hovered = CheckCollisionPointRec(mouse, rowRect);

    if (hovered) {
        isMouseCaptured = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *value = !(*value); // Toggle the boolean
        }
    }

    // Draw the checkbox square
    DrawRectangle((int)boxX, (int)boxY, (int)boxSize, (int)boxSize, Color{ 25, 35, 50, 255 });
    DrawRectangleLines((int)boxX, (int)boxY, (int)boxSize, (int)boxSize, Color{ 70, 95, 135, 255 });

    // If checked, draw a bright blue inner square
    if (*value) {
        DrawRectangle((int)boxX + 2, (int)boxY + 2, (int)boxSize - 4, (int)boxSize - 4, Color{ 80, 160, 240, 255 });
    }

    // Label text next to the checkbox
    DrawText(label, (int)boxX + (int)boxSize + 8, (int)cursorY + 2, 12, Color{ 220, 225, 235, 255 });

    cursorY += itemH;
    return *value;
}

// A horizontal slider for tweaking float numbers (like radius, height, speed)
inline bool SliderFloat(const char* label, float* value, float minVal, float maxVal) {
    if (isCollapsed) return false;

    Vector2 mouse = GetMousePosition();
    float itemH = 19.0f;
    float sliderW = 125.0f;
    float sliderH = 14.0f;
    float sliderX = panelX + 8.0f;
    float sliderY = cursorY + 2.0f;

    Rectangle sliderRect = { sliderX, sliderY, sliderW, sliderH };
    bool hovered = CheckCollisionPointRec(mouse, sliderRect);

    void* id = (void*)value;
    if (hovered) {
        isMouseCaptured = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            activeWidget = id;
        }
    }

    // While dragging the slider, calculate the new value based on mouse X position
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && activeWidget == id) {
        isMouseCaptured = true;
        float percent = (mouse.x - sliderX) / sliderW;
        percent = std::max(0.0f, std::min(1.0f, percent));
        *value = minVal + percent * (maxVal - minVal);
    }

    // Draw slider background groove
    DrawRectangle((int)sliderX, (int)sliderY, (int)sliderW, (int)sliderH, Color{ 20, 25, 38, 255 });

    // Draw filled portion representing current value
    float fillPercent = std::max(0.0f, std::min(1.0f, (*value - minVal) / (maxVal - minVal)));
    DrawRectangle((int)sliderX, (int)sliderY, (int)(sliderW * fillPercent), (int)sliderH, Color{ 50, 90, 145, 255 });
    DrawRectangleLines((int)sliderX, (int)sliderY, (int)sliderW, (int)sliderH, Color{ 70, 95, 135, 255 });

    // Show the number value on the slider
    DrawText(TextFormat("%.3f", *value), (int)sliderX + 6, (int)sliderY + 2, 10, Color{ 240, 240, 245, 255 });

    // Label on the right
    DrawText(label, (int)sliderX + (int)sliderW + 8, (int)sliderY + 2, 11, Color{ 200, 205, 215, 255 });

    cursorY += itemH;
    return true;
}

// 3 side-by-side float drag inputs for a 3D coordinate (X, Y, Z)
inline bool DragFloat3(const char* label, float* x, float* y, float* z, float minVal = -30.0f, float maxVal = 30.0f) {
    if (isCollapsed) return false;

    Vector2 mouse = GetMousePosition();
    float itemH = 19.0f;
    float totalW = 125.0f;
    float fieldW = (totalW - 6.0f) / 3.0f;
    float fieldH = 14.0f;
    float startX = panelX + 8.0f;
    float startY = cursorY + 2.0f;

    float* vals[3] = { x, y, z };
    for (int i = 0; i < 3; ++i) {
        float fx = startX + i * (fieldW + 3.0f);
        Rectangle fieldRect = { fx, startY, fieldW, fieldH };
        bool hovered = CheckCollisionPointRec(mouse, fieldRect);

        void* id = (void*)(vals[i]);
        if (hovered) {
            isMouseCaptured = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                activeWidget = id;
            }
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && activeWidget == id) {
            isMouseCaptured = true;
            float percent = (mouse.x - fx) / fieldW;
            percent = std::max(0.0f, std::min(1.0f, percent));
            *vals[i] = minVal + percent * (maxVal - minVal);
        }

        DrawRectangle((int)fx, (int)startY, (int)fieldW, (int)fieldH, Color{ 20, 25, 38, 255 });
        DrawRectangleLines((int)fx, (int)startY, (int)fieldW, (int)fieldH, Color{ 70, 95, 135, 255 });
        DrawText(TextFormat("%.1f", *vals[i]), (int)fx + 3, (int)startY + 2, 10, Color{ 240, 240, 245, 255 });
    }

    DrawText(label, (int)startX + (int)totalW + 8, (int)startY + 2, 11, Color{ 200, 205, 215, 255 });

    cursorY += itemH;
    return true;
}

// Call this at the end of your GUI calls
inline void End() {
    if (!isCollapsed) {
        float bodyH = cursorY - panelY - 22.0f + 4.0f;
        Rectangle bodyRect = { panelX, panelY + 22.0f, panelW, bodyH };

        // Outline around the panel
        DrawRectangleLinesEx(bodyRect, 1.0f, Color{ 50, 75, 110, 200 });

        // Check if mouse is anywhere inside the whole panel so camera clicks don't leak through
        Rectangle wholePanel = { panelX, panelY, panelW, cursorY - panelY + 4.0f };
        if (CheckCollisionPointRec(GetMousePosition(), wholePanel)) {
            isMouseCaptured = true;
        }
    }
}

// Check if the GUI is using the mouse right now
inline bool IsMouseBusy() {
    return isMouseCaptured;
}

} // namespace gui
