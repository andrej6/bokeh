#ifndef UTIL_H_
#define UTIL_H_

#include <iostream>

#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define EPSILON 0.00001f
extern const double PI;
extern std::ostream *gl_error_stream;

static inline std::ostream &glerr() { return *gl_error_stream; }

const char *gl_error_to_string(GLenum err);
// Writes a message to the error stream upon any errors. If `warn` is false,
// terminates the program if any errors were detected.
void handle_gl_error(bool warn = false);

void handle_shader_error(GLuint shader, bool warn = false);

void handle_program_error(GLuint program, bool warn = false);

double randf();
uint32_t randi();

#endif /* UTIL_H_ */
