#include "shader.h"
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

static char* read_file_all(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc((size_t)len + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, (size_t)len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

static GLuint compile_shader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(s, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error: %s\n", log);
    }
    return s;
}

unsigned int shader_compile_program(const char* vert_path, const char* frag_path)
{
    char* vsrc = read_file_all(vert_path);
    char* fsrc = read_file_all(frag_path);
    if (!vsrc || !fsrc) {
        fprintf(stderr, "Failed to read shader files: %s, %s\n", vert_path, frag_path);
        free(vsrc); free(fsrc);
        return 0;
    }
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vsrc);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fsrc);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        fprintf(stderr, "Program link error: %s\n", log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    free(vsrc);
    free(fsrc);
    return prog;
}
