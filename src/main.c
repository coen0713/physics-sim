#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "sim.h"
#include "shader.h"

static const int WIN_W = 1200;
static const int WIN_H = 800;
static const size_t NUM_PARTICLES = 2000;

static bool g_paused = false;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)scancode; (void)mods;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, 1);
        if (key == GLFW_KEY_SPACE) g_paused = !g_paused;
    }
}

static void set_blend()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(void)
{
    srand((unsigned)time(NULL));

    if (!glfwInit()) {
        fprintf(stderr, "Failed to init GLFW\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIN_W, WIN_H, "Verlet Particles", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW error: %s\n", glewGetErrorString(err));
        return 1;
    }

    Simulation sim = {0};
    sim_init(&sim, NUM_PARTICLES, (float)WIN_W, (float)WIN_H);

    GLuint prog = shader_compile_program("shaders/particle.vert", "shaders/particle.frag");
    if (!prog) {
        fprintf(stderr, "Failed to compile shaders\n");
        return 1;
    }

    GLint uResolution = glGetUniformLocation(prog, "uResolution");

    // VBO for positions (NDC), radii, colors
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    size_t stride = sizeof(float) * 6; // x,y, radius, r,g,b
    glBufferData(GL_ARRAY_BUFFER, sim.count * stride, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)0);
    glEnableVertexAttribArray(1); // radius
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)(sizeof(float)*2));
    glEnableVertexAttribArray(2); // color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)(sizeof(float)*3));

    set_blend();
    glEnable(GL_PROGRAM_POINT_SIZE);

    double last_time = glfwGetTime();
    double acc = 0.0;
    const double dt = 1.0 / 120.0; // fixed-step for stability

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        double frame = now - last_time;
        last_time = now;
        acc += frame;

        glfwPollEvents();

        // Inputs for toggles
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) sim.enable_gravity = true;
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) sim.enable_gravity = false;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) sim.enable_collisions = true;
        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) sim.enable_collisions = false;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) sim_reset(&sim);

        // Mouse as attractor
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double mx, my; glfwGetCursorPos(window, &mx, &my);
            sim.enable_attractor = true;
            sim.attractor.x = (float)mx;
            sim.attractor.y = (float)(WIN_H - my);
        } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            sim.enable_attractor = false;
        }

        while (acc >= dt) {
            if (!g_paused) sim_step(&sim, (float)dt, 2);
            acc -= dt;
        }

        glViewport(0, 0, WIN_W, WIN_H);
        glClearColor(0.06f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glUniform2f(uResolution, (float)WIN_W, (float)WIN_H);

        // Update buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        float* ptr = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        if (ptr) {
            for (size_t i = 0; i < sim.count; ++i) {
                // NDC conversion done in shader using uResolution; we store screen coords here
                ptr[0] = sim.particles[i].pos.x;
                ptr[1] = sim.particles[i].pos.y;
                ptr[2] = sim.particles[i].radius;
                ptr[3] = sim.particles[i].r;
                ptr[4] = sim.particles[i].g;
                ptr[5] = sim.particles[i].b;
                ptr += 6;
            }
            glUnmapBuffer(GL_ARRAY_BUFFER);
        }

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, (GLsizei)sim.count);

        glfwSwapBuffers(window);
    }

    sim_free(&sim);
    glDeleteProgram(prog);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
