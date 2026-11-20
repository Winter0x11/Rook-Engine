use core::{Vec2, World};
use macroquad::prelude::*;

fn window_conf() -> Conf {
    Conf {
        window_title: "Woooblyy Wabbbly wooooo".to_string(),
        window_width: 800,
        window_height: 600,
        ..Default::default()
    }
}

#[macroquad::main(window_conf)]
async fn main() {
    // Gravity: 980 pixels/s^2 downwards
    let mut world = World::new(Vec2::new(0.0, 980.0), screen_width(), screen_height());

    // Initial bouncing balls so the simulation is active immediately on launch
    for i in 0..5 {
        let x = 150.0 + i as f32 * 120.0;
        let y = 100.0 + (i % 2) as f32 * 50.0;
        let vx = (i as f32 - 2.0) * 80.0;
        let radius = 20.0;
        let mass = 1.0;
        world.add_body(Vec2::new(x, y), Vec2::new(vx, 0.0), mass, radius);
    }

    loop {
        // Keep screen bounds in sync with window dimensions
        world.width = screen_width();
        world.height = screen_height();

        // Click to spawn a ball at mouse position
        if is_mouse_button_pressed(MouseButton::Left) {
            let (mx, my) = mouse_position();
            // Give a slight initial horizontal nudge based on distance from screen center
            let nudge_vx = (mx - world.width * 0.5) * 0.5;
            world.add_body(Vec2::new(mx, my), Vec2::new(nudge_vx, -50), 1.0, 20.0);
        }

        // Advance simulation with fixed 60 Hz timesteps
        world.update(get_frame_time());

        // Render
        clear_background(Color::new(0.12, 0.12, 0.15, 1.0));

        // Draw ground plane indicator line
        draw_line(
            0.0,
            world.height - 1.0,
            world.width,
            world.height - 1.0,
            2.0,
            GRAY,
        );

        // Draw all circle bodies
        for i in 0..world.body_count() {
            let pos = world.positions[i];
            let radius = world.radii[i];

            // Main body circle
            draw_circle(pos.x, pos.y, radius, SKYBLUE);
            // Circle outline
            draw_circle_lines(pos.x, pos.y, radius, 2.0, WHITE);
        }

        // UI overlay: FPS counter and ball count
        let fps_text = format!("FPS: {}", get_fps());
        let count_text = format!("Balls: {}", world.body_count());
        draw_text(&fps_text, 20.0, 30.0, 24.0, WHITE);
        draw_text(&count_text, 20.0, 60.0, 20.0, LIGHTGRAY);

        next_frame().await;
    }
}
