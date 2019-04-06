#ifndef UTIL_H_
#define UTIL_H_

#include <iostream>

#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define EPSILON 0.00001f
extern const double PI;
extern std::ostream *gl_error_stream;

// Error reporting stream. By default, this is stderr, but this can be configured.
static inline std::ostream &glerr() { return *gl_error_stream; }
static inline void set_glerr(std::ostream &new_stream) { gl_error_stream = &new_stream; }

// Convert an OpenGL error enum to a human-readable string representation.
const char *gl_error_to_string(GLenum err);

// Writes a message to the error stream upon any errors. If `warn` is false,
// terminates the application if any errors were detected.
void handle_gl_error(bool warn = false);

// Writes a message to the error stream upon any shader compilation errors, and
// prints the error log. If `warn` is false, terminates the application if any
// errors were detected.
void handle_shader_error(GLuint shader, bool warn = false);

// Writes a message to the error stream upon and program linking errors, and
// prints the error log. If `warn` is false, terminates the application if any
// errors were detected.
void handle_program_error(GLuint program, bool warn = false);

// Generate a random floating-point value in the range 0 to 1.
double randf();

// Generate a random integer.
uint32_t randi();

// Convert degrees to radians.
static inline double deg_to_rad(double d) {
  return d * PI / 180.0;
}

// Convert radians to degrees.
static inline double rad_to_deg(double r) {
  return r * 180.0 / PI;
}

#endif /* UTIL_H_ */
