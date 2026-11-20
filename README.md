# Ruin Engine

A 2D Physics Engine (Phase 1: Particle Dynamics)

A minimalist 2D physics engine starter project written in Rust with a data-oriented C mindset: plain structs, parallel `Vec` arrays, and integer index handles instead of pointers or complex object hierarchies.

## Folder Layout

```text
engine/
├── .github/
│   └── workflows/
│       └── ci.yml          # GitHub Actions CI (fmt, clippy, test)
├── core/                   # Physics engine library (depends only on glam)
│   ├── Cargo.toml
│   └── src/
│       └── lib.rs          # World struct, Euler integrator, collision solvers, unit tests
├── testbed/                # Interactive demo (depends on core and macroquad)
│   ├── Cargo.toml
│   └── src/
│       └── main.rs         # Window setup, mouse input, rendering loop, FPS counter
├── Cargo.toml              # Workspace configuration
└── README.md
```

## Running the Project

### Interactive Testbed
Run the demo window (left-click to spawn bouncing balls):
```bash
cargo run -p testbed
```

### Run Unit Tests
Verifies the semi-implicit Euler free-fall integrator against analytic formulas and validates ground collision response:
```bash
cargo test
```

### Run Linter & Formatter Check
```bash
cargo fmt --check
cargo clippy -- -D warnings
```

---

## Physics Implemented (Phase 1)

1. **Data-Oriented State (`World`)**:
   - Entities are identified by index `BodyId(u32)`.
   - Body properties (`positions`, `velocities`, `inv_masses`, `radii`) are kept in parallel `Vec`s (Structure-of-Arrays).
   - Storing inverse mass ($1/m$) makes static bodies ($1/m = 0$) seamless without special casing or dividing by zero.

2. **Semi-Implicit Euler Integrator**:
   - Updates velocity first: $v_{t+dt} = v_t + g \cdot dt$.
   - Updates position with the new velocity: $x_{t+dt} = x_t + v_{t+dt} \cdot dt$.
   - Symplectic properties conserve phase space, preventing the artificial energy gain of explicit Euler.

3. **Fixed Timestep (60 Hz)**:
   - Frame delta time accumulates in `world.accumulator`.
   - Steps discrete ticks at `FIXED_DT = 1/60`s for deterministic, stable behavior regardless of rendering framerates.

4. **Circle vs. Circle Collisions**:
   - **Position Projection**: Directly resolves geometric overlap along the collision normal proportional to inverse mass.
   - **Impulse Response**: Computes normal impulse $J = -\frac{(1 + e)(v_{\text{rel}} \cdot n)}{w_A + w_B}$ to exchange momentum.

5. **Screen Wall & Ground Plane Collisions**:
   - Clamps circles within boundaries ($x \in [0, \text{width}]$, $y \in [0, \text{height}]$).
   - Inverts normal velocity scaled by coefficient of restitution $e$ ($v_n' = -e \cdot v_n$). Zero friction keeps tangential velocity constant.

---

## Next Steps (Phases 2 & 3)

1. **Rigid Bodies with Rotation**:
   - Add orientation angle $\theta$ (`f32`) and angular velocity $\omega$ (`f32`).
   - Add moment of inertia $I$ and inverse inertia $1/I$ (for circle: $I = \frac{1}{2} m r^2$; for box: $I = \frac{1}{12} m (w^2 + h^2)$).
   - Integrate rotation: $\omega_{t+dt} = \omega_t + \tau \cdot \frac{1}{I} \cdot dt$, $\theta_{t+dt} = \theta_t + \omega_{t+dt} \cdot dt$.
   - Relative contact velocity includes rotational term: $v_p = v + \omega \times r$.

2. **Box Shapes (Oriented Bounding Boxes - OBB)**:
   - Store half-extents `Vec2` (half-width, half-height) per body.
   - Derive vertex positions using 2D rotation matrices based on current orientation $\theta$.

3. **SAT Collision (Separating Axis Theorem)**:
   - Test potential separating axes (the 2 face normals of box A + the 2 face normals of box B).
   - If projections overlap on all axes, a collision exists; the axis with minimum overlap gives the contact normal and penetration depth.
   - Clip polygon edges to find contact manifold points.

4. **Sequential Impulses (Velocity Constraint Solver)**:
   - Formulate contacts and joints as velocity constraints: $J v = 0$.
   - Iterate multiple solver loops per frame, clamping accumulated impulses to simulate solid non-penetration and resting stacks.
   - Add Coulomb friction along the contact tangent ($|J_{\text{tangent}}| \le \mu \cdot J_{\text{normal}}$).
