#ifndef SIM_H
#define SIM_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    Vec2 pos;
    Vec2 prev;
    Vec2 acc;
    float radius;
    float mass;
    float r, g, b;
} Particle;

typedef struct {
    Particle* particles;
    size_t count;
    float width;
    float height;
    float damping;
    float gravity;
    bool enable_gravity;
    bool enable_collisions;
    bool enable_attractor;
    Vec2 attractor;
} Simulation;

void sim_init(Simulation* sim, size_t count, float width, float height);
void sim_reset(Simulation* sim);
void sim_free(Simulation* sim);

// Step simulation by fixed dt in seconds
void sim_step(Simulation* sim, float dt, int solver_iterations);

#ifdef __cplusplus
}
#endif

#endif // SIM_H
