#include "sim.h"
#include <stdlib.h>
#include <math.h>

static float frand()
{
    return (float)rand() / (float)RAND_MAX;
}

static Vec2 v_add(Vec2 a, Vec2 b) { return (Vec2){a.x + b.x, a.y + b.y}; }
static Vec2 v_sub(Vec2 a, Vec2 b) { return (Vec2){a.x - b.x, a.y - b.y}; }
static Vec2 v_scale(Vec2 a, float s) { return (Vec2){a.x * s, a.y * s}; }
static float v_len2(Vec2 a) { return a.x*a.x + a.y*a.y; }
static float v_len(Vec2 a) { return sqrtf(v_len2(a)); }

void sim_init(Simulation* sim, size_t count, float width, float height)
{
    sim->count = count;
    sim->width = width;
    sim->height = height;
    sim->damping = 0.999f;
    sim->gravity = 400.0f; // px/s^2 downward
    sim->enable_gravity = true;
    sim->enable_collisions = true;
    sim->enable_attractor = false;
    sim->attractor = (Vec2){width * 0.5f, height * 0.5f};

    sim->particles = (Particle*)calloc(count, sizeof(Particle));

    for (size_t i = 0; i < count; ++i) {
        float r = 2.0f + frand() * 4.0f;
        float x = 20.0f + frand() * (width - 40.0f);
        float y = 20.0f + frand() * (height - 40.0f);
        sim->particles[i].pos = (Vec2){x, y};
        // small random initial velocity encoded via prev
        float vx = (frand() - 0.5f) * 50.0f;
        float vy = (frand() - 0.5f) * 50.0f;
        sim->particles[i].prev = (Vec2){x - vx * 0.016f, y - vy * 0.016f};
        sim->particles[i].acc = (Vec2){0, 0};
        sim->particles[i].radius = r;
        sim->particles[i].mass = r * r; // area proxy
        sim->particles[i].r = 0.7f + 0.3f * frand();
        sim->particles[i].g = 0.7f + 0.3f * frand();
        sim->particles[i].b = 0.7f + 0.3f * frand();
    }
}

void sim_reset(Simulation* sim)
{
    if (!sim || !sim->particles) return;
    size_t count = sim->count;
    float width = sim->width, height = sim->height;
    for (size_t i = 0; i < count; ++i) {
        float r = 2.0f + ((float)rand()/RAND_MAX) * 4.0f;
        float x = 20.0f + ((float)rand()/RAND_MAX) * (width - 40.0f);
        float y = 20.0f + ((float)rand()/RAND_MAX) * (height - 40.0f);
        sim->particles[i].pos = (Vec2){x, y};
        float vx = (((float)rand()/RAND_MAX) - 0.5f) * 50.0f;
        float vy = (((float)rand()/RAND_MAX) - 0.5f) * 50.0f;
        sim->particles[i].prev = (Vec2){x - vx * 0.016f, y - vy * 0.016f};
        sim->particles[i].acc = (Vec2){0, 0};
        sim->particles[i].radius = r;
        sim->particles[i].mass = r * r;
    }
}

void sim_free(Simulation* sim)
{
    if (sim && sim->particles) {
        free(sim->particles);
        sim->particles = NULL;
    }
}

static void apply_global_forces(Simulation* sim)
{
    for (size_t i = 0; i < sim->count; ++i) {
        Vec2 a = {0, 0};
        if (sim->enable_gravity) {
            a.y -= sim->gravity; // gravity pulls towards bottom of the screen
        }
        if (sim->enable_attractor) {
            Vec2 dir = v_sub(sim->attractor, sim->particles[i].pos);
            float d2 = v_len2(dir) + 5.0f; // less softening -> stronger near cursor
            float invd = 1.0f / sqrtf(d2);
            dir = v_scale(dir, invd);
            float strength = 300000.0f / d2; // much stronger inverse-square attraction
            a = v_add(a, v_scale(dir, strength));
        }
        sim->particles[i].acc = a;
    }
}

static void integrate_verlet(Simulation* sim, float dt)
{
    const float dt2 = dt * dt;
    for (size_t i = 0; i < sim->count; ++i) {
        Particle* p = &sim->particles[i];
        Vec2 pos = p->pos;
        Vec2 vel = v_sub(p->pos, p->prev);
        vel = v_scale(vel, sim->damping);
        p->pos = v_add(v_add(pos, vel), v_scale(p->acc, dt2));
        p->prev = pos;
        p->acc = (Vec2){0,0};
    }
}

static void satisfy_bounds(Simulation* sim)
{
    for (size_t i = 0; i < sim->count; ++i) {
        Particle* p = &sim->particles[i];
        float r = p->radius;
        if (p->pos.x < r) p->pos.x = r;
        if (p->pos.x > sim->width - r) p->pos.x = sim->width - r;
        if (p->pos.y < r) p->pos.y = r;
        if (p->pos.y > sim->height - r) p->pos.y = sim->height - r;
    }
}

static void collide_pairs(Simulation* sim)
{
    if (!sim->enable_collisions) return;
    size_t n = sim->count;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            Particle* a = &sim->particles[i];
            Particle* b = &sim->particles[j];
            Vec2 d = v_sub(b->pos, a->pos);
            float dist2 = v_len2(d);
            float r = a->radius + b->radius;
            if (dist2 > 0.0f && dist2 < r * r) {
                float dist = sqrtf(dist2);
                float overlap = r - dist;
                Vec2 nrm = v_scale(d, 1.0f / (dist + 1e-6f));
                float im_a = 1.0f / a->mass;
                float im_b = 1.0f / b->mass;
                float sum_im = im_a + im_b;
                float move_a = overlap * (im_a / sum_im);
                float move_b = overlap * (im_b / sum_im);
                a->pos = v_sub(a->pos, v_scale(nrm, move_a));
                b->pos = v_add(b->pos, v_scale(nrm, move_b));
            }
        }
    }
}

void sim_step(Simulation* sim, float dt, int solver_iterations)
{
    apply_global_forces(sim);
    integrate_verlet(sim, dt);
    for (int i = 0; i < solver_iterations; ++i) {
        collide_pairs(sim);
        satisfy_bounds(sim);
    }
}
