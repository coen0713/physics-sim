# Verlet Particle Collision Simulation (C + OpenGL)

- **Build**: Linux, GCC, OpenGL 3.3, GLFW, GLEW
- **Run**: `make && make run`

## Features
- **Verlet integration** for stable, precise motion.
- **Particle-particle collisions** resolved via positional correction.
- **Boundary constraints** within the window.
- **Real-time rendering** with OpenGL point sprites.
- **Interactivity**:
  - **Space**: Pause/resume
  - **R**: Reset particles
  - **G/H**: Enable/disable gravity
  - **C/V**: Enable/disable collisions
  - **Mouse Left**: Hold to enable attractor at cursor

## Dependencies
- `glfw3`, `glew`, `opengl` headers and libs (Ubuntu/Debian):
```
sudo apt-get install libglfw3-dev libglew-dev mesa-common-dev
```

## Structure
- `src/sim.h`, `src/sim.c`: Physics and collisions.
- `src/main.c`: App entry, render loop, inputs.
- `src/shader.h`, `src/shader.c`: Shader loader/compiler.
- `shaders/particle.vert`, `shaders/particle.frag`: Shaders for particles.
- `Makefile`: Build targets.

## Notes
- Simulation space == window pixels. Radii are in pixels for intuitive sizing.
- Fixed timestep integration improves stability; rendering interpolates current state.
