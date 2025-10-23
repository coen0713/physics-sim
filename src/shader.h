#ifndef SHADER_H
#define SHADER_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned int shader_compile_program(const char* vert_path, const char* frag_path);

#ifdef __cplusplus
}
#endif

#endif // SHADER_H
