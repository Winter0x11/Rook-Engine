pub use glam::Vec2;

/// Fixed simulation timestep: 60 Hz (1/60th second per tick).
pub const FIXED_DT: f32 = 1.0 / 60.0;

/// Identifier for a body in the World, represented as a simple array index.
///
/// In data-oriented C style, handles are preferred over pointers or references
/// to avoid memory fragmentation and lifetime issues.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct BodyId(pub u32);

/// 2D physics simulation world.
///
/// Bodies are stored in parallel `Vec`s (Structure-of-Arrays).
/// All arrays have matching lengths, and `BodyId(i)` maps to index `i`.
pub struct World {
    pub positions: Vec<Vec2>,
    pub velocities: Vec<Vec2>,
    pub inv_masses: Vec<f32>, // 1.0 / mass; 0.0 means static (infinite mass)
    pub radii: Vec<f32>,

    pub gravity: Vec2,
    pub restitution: f32, // Bounciness e in [0.0, 1.0]
    pub width: f32,       // Screen width (right wall)
    pub height: f32,      // Screen height (ground plane)
    pub accumulator: f32, // Accumulated delta time for fixed 60 Hz steps
}

impl World {
    /// Creates a new physics world.
    ///
    /// - `gravity`: acceleration vector (e.g. `(0.0, 980.0)` for screen-down gravity).
    /// - `width`: horizontal boundary (left wall is at 0, right wall is at `width`).
    /// - `height`: vertical boundary (ceiling is at 0, ground plane is at `height`).
    pub fn new(gravity: Vec2, width: f32, height: f32) -> Self {
        Self {
            positions: Vec::new(),
            velocities: Vec::new(),
            inv_masses: Vec::new(),
            radii: Vec::new(),
            gravity,
            restitution: 0.75,
            width,
            height,
            accumulator: 0.0,
        }
    }

    /// Spawns a new circular particle into the world.
    ///
    /// If mass is 0.0 or negative, inverse mass is set to 0.0 (static body).
    /// Storing inverse mass simplifies F = m*a -> a = F * inv_mass, avoiding
    /// divisions by zero for static obstacles.
    pub fn add_body(&mut self, position: Vec2, velocity: Vec2, mass: f32, radius: f32) -> BodyId {
        let id = BodyId(self.positions.len() as u32);
        let inv_mass = if mass > 0.0 { 1.0 / mass } else { 0.0 };
        self.positions.push(position);
        self.velocities.push(velocity);
        self.inv_masses.push(inv_mass);
        self.radii.push(radius);
        id
    }

    /// Returns the number of bodies in the world.
    pub fn body_count(&self) -> usize {
        self.positions.len()
    }

    /// Clears all bodies from the world.
    pub fn clear(&mut self) {
        self.positions.clear();
        self.velocities.clear();
        self.inv_masses.clear();
        self.radii.clear();
        self.accumulator = 0.0;
    }

    /// Semi-implicit Euler integration:
    ///
    /// 1. v(t + dt) = v(t) + g * dt
    /// 2. x(t + dt) = x(t) + v(t + dt) * dt
    ///
    /// Updating velocity first and using the new velocity for position makes
    /// the integrator "symplectic". Unlike explicit Euler, it does not add
    /// artificial energy into the system and preserves oscillations stably.
    pub fn integrate(&mut self, dt: f32) {
        for i in 0..self.positions.len() {
            if self.inv_masses[i] == 0.0 {
                continue; // Static bodies do not move
            }
            self.velocities[i] += self.gravity * dt;
            self.positions[i] += self.velocities[i] * dt;
        }
    }

    /// Boundary collisions with the static ground plane and screen walls.
    ///
    /// Left wall: x = 0
    /// Right wall: x = self.width
    /// Top wall: y = 0
    /// Ground plane: y = self.height
    ///
    /// When a circle penetrates a wall:
    /// 1. Position correction: clamp position to the boundary surface so it cannot sink.
    /// 2. Impulse response: reflect normal velocity scaled by restitution: `v_normal = -e * v_normal`.
    ///
    /// Friction is 0, so tangential velocity is untouched.
    pub fn resolve_wall_collisions(&mut self) {
        for i in 0..self.positions.len() {
            if self.inv_masses[i] == 0.0 {
                continue;
            }

            let r = self.radii[i];
            let mut pos = self.positions[i];
            let mut vel = self.velocities[i];

            // Left wall (x = 0)
            if pos.x - r < 0.0 {
                pos.x = r;
                if vel.x < 0.0 {
                    vel.x = -vel.x * self.restitution;
                }
            }
            // Right wall (x = width)
            if pos.x + r > self.width {
                pos.x = self.width - r;
                if vel.x > 0.0 {
                    vel.x = -vel.x * self.restitution;
                }
            }
            // Top wall (y = 0)
            if pos.y - r < 0.0 {
                pos.y = r;
                if vel.y < 0.0 {
                    vel.y = -vel.y * self.restitution;
                }
            }
            // Ground plane (y = height)
            if pos.y + r > self.height {
                pos.y = self.height - r;
                if vel.y > 0.0 {
                    vel.y = -vel.y * self.restitution;
                }
            }

            self.positions[i] = pos;
            self.velocities[i] = vel;
        }
    }

    /// Circle vs. circle collision detection and impulse response.
    ///
    /// Two circles A and B overlap if distance `d < r_A + r_B`.
    ///
    /// 1. Position correction:
    ///    Push circles apart along the normal `n = (p_B - p_A) / d` proportional to
    ///    their inverse masses:
    ///    - `p_A -= n * penetration * (w_A / (w_A + w_B))`
    ///    - `p_B += n * penetration * (w_B / (w_A + w_B))`
    ///
    /// 2. Impulse response:
    ///    Relative velocity along contact normal: `v_n = (v_B - v_A) . n`.
    ///    If `v_n < 0` (approaching), compute impulse magnitude `J` from Newton's restitution:
    ///    - `J = -(1 + e) * v_n / (w_A + w_B)`
    ///    - `v_A -= J * w_A * n`
    ///    - `v_B += J * w_B * n`
    pub fn resolve_circle_collisions(&mut self) {
        let count = self.positions.len();
        for i in 0..count {
            for j in (i + 1)..count {
                let w_a = self.inv_masses[i];
                let w_b = self.inv_masses[j];
                let w_sum = w_a + w_b;
                if w_sum == 0.0 {
                    continue; // Both bodies are static
                }

                let delta = self.positions[j] - self.positions[i];
                let dist_sq = delta.length_squared();
                let r_sum = self.radii[i] + self.radii[j];

                if dist_sq >= r_sum * r_sum {
                    continue; // No collision
                }

                let dist = dist_sq.sqrt();
                let (normal, penetration) = if dist > 1e-6 {
                    (delta / dist, r_sum - dist)
                } else {
                    (Vec2::new(0.0, -1.0), r_sum)
                };

                // Position correction: eliminate overlap
                self.positions[i] -= normal * (penetration * (w_a / w_sum));
                self.positions[j] += normal * (penetration * (w_b / w_sum));

                // Impulse response: exchange momentum along normal
                let rel_vel = self.velocities[j] - self.velocities[i];
                let sep_vel = rel_vel.dot(normal);

                if sep_vel < 0.0 {
                    let impulse_mag = -(1.0 + self.restitution) * sep_vel / w_sum;
                    let impulse = normal * impulse_mag;
                    self.velocities[i] -= impulse * w_a;
                    self.velocities[j] += impulse * w_b;
                }
            }
        }
    }

    /// Advances the simulation by one discrete timestep `dt`.
    ///
    /// Execution order:
    /// 1. Integrate forces and velocities (semi-implicit Euler)
    /// 2. Resolve circle-circle collisions
    /// 3. Resolve ground and screen wall collisions
    pub fn step(&mut self, dt: f32) {
        self.integrate(dt);
        self.resolve_circle_collisions();
        self.resolve_wall_collisions();
    }

    /// Updates the world using a fixed timestep accumulator (60 Hz).
    ///
    /// Frame times from rendering vary; fixed timesteps keep physics deterministic
    /// and numerically stable. We accumulate frame time and take discrete 60 Hz steps.
    /// `frame_time` is clamped to 0.25s to avoid a spiral of death if frames drop.
    pub fn update(&mut self, frame_time: f32) {
        let dt = frame_time.min(0.25);
        self.accumulator += dt;

        while self.accumulator >= FIXED_DT {
            self.step(FIXED_DT);
            self.accumulator -= FIXED_DT;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_free_fall_integrator() {
        // Test semi-implicit Euler integration against analytic equations.
        // For constant gravity g:
        //   Analytic continuous:
        //     v(t) = v0 + g * t
        //     y(t) = y0 + v0 * t + 0.5 * g * t^2
        //   Discrete semi-implicit Euler after k steps of dt:
        //     v_k = v0 + g * t
        //     y_k = y0 + 0.5 * g * t^2 + 0.5 * g * dt * t
        let g = 9.8f32;
        let mut world = World::new(Vec2::new(0.0, g), 1000.0, 1000.0);
        let start_pos = Vec2::new(500.0, 100.0);
        let id = world.add_body(start_pos, Vec2::ZERO, 1.0, 5.0);

        let dt = FIXED_DT;
        let steps = 60; // 1.0 second total simulation time
        for _ in 0..steps {
            world.step(dt);
        }

        let idx = id.0 as usize;
        let vy = world.velocities[idx].y;
        let delta_y = world.positions[idx].y - start_pos.y;

        // Velocity matches continuous analytic result: v = g * t = 9.8 m/s
        assert!((vy - 9.8).abs() < 1e-4, "vy: expected 9.8, got {}", vy);

        // Continuous analytic displacement: 0.5 * 9.8 * 1.0^2 = 4.9 m.
        // Discrete semi-implicit Euler displacement: 4.9 + 0.5 * 9.8 * (1/60) * 1.0 ≈ 4.98167 m.
        // Displacement matches continuous within ~2% tolerance:
        assert!(
            (delta_y - 4.9).abs() < 0.1,
            "delta_y: expected ~4.9, got {}",
            delta_y
        );

        // Matches exact discrete semi-implicit Euler formula:
        let expected_discrete_y = 4.9 + 0.5 * g * dt * 1.0;
        assert!(
            (delta_y - expected_discrete_y).abs() < 1e-4,
            "delta_y: expected {}, got {}",
            expected_discrete_y,
            delta_y
        );
    }

    #[test]
    fn test_ground_collision() {
        // Place a ball falling toward the ground plane at y = 100.0.
        let mut world = World::new(Vec2::ZERO, 500.0, 100.0);
        world.restitution = 0.8;

        let radius = 10.0;
        let initial_vy = 50.0;
        // Position at y = 85.0. Next step of dt = 0.2 would put it at y = 95.0.
        // Bottom edge would be at 95.0 + 10.0 = 105.0 > 100.0 (ground).
        let id = world.add_body(
            Vec2::new(100.0, 85.0),
            Vec2::new(0.0, initial_vy),
            1.0,
            radius,
        );

        world.step(0.2);

        let idx = id.0 as usize;
        let pos = world.positions[idx];
        let vel = world.velocities[idx];

        // 1. Position correction: ball cannot sink through ground (y <= ground - radius)
        assert!(
            (pos.y - (100.0 - radius)).abs() < 1e-4,
            "Ball penetrated ground! pos.y = {}, expected {}",
            pos.y,
            100.0 - radius
        );

        // 2. Velocity response: vy reversed and scaled by restitution: -0.8 * 50 = -40
        let expected_vy = -world.restitution * initial_vy;
        assert!(
            (vel.y - expected_vy).abs() < 1e-4,
            "vy: expected {}, got {}",
            expected_vy,
            vel.y
        );
    }
}
