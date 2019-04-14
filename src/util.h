#ifndef UTIL_H_
#define UTIL_H_

#include <iostream>
#include <string>
#include <vector>

#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

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
void handle_gl_error(const char *msg, bool warn = false);

// Writes a message to the error stream upon any shader compilation errors, and
// prints the error log. If `warn` is false, terminates the application if any
// errors were detected.
void handle_shader_error(const char *msg, GLuint shader, bool warn = false);

// Writes a message to the error stream upon and program linking errors, and
// prints the error log. If `warn` is false, terminates the application if any
// errors were detected.
void handle_program_error(const char *msg, GLuint program, bool warn = false);

// Generate a random floating-point value in the range 0 to 1.
double randf();

// Generate a random integer.
uint32_t randi();

// Generate a random vector in the [-1, 1] cube centered at the origin.
static inline glm::vec3 randvec() {
  float x = 2*randf() - 1.0,
        y = 2*randf() - 1.0,
        z = 2*randf() - 1.0;
  return glm::vec3(x, y, z);
}

// Convert degrees to radians.
static inline double deg_to_rad(double d) {
  return d * PI / 180.0;
}

// Convert radians to degrees.
static inline double rad_to_deg(double r) {
  return r * 180.0 / PI;
}

void barycentric_coords(const glm::vec3 &point,
    const glm::vec3 &va, const glm::vec3 &vb, const glm::vec3 &vc,
    float &alpha, float &beta, float &gamma);

// Split a string into substrings delimited by the characters in `delims`. If `multi` is
// true, split() will treat any sequence of consecutive characters in `delims` as a single
// delimiter, and not produce an empty string for each consecutive pair.
std::vector<std::string> split(
    const std::string &str,
    const char *delims = " \t\n",
    bool multi=true);

// Strip a string of leading and trailing whitespace.
std::string strip(const std::string &str);

#endif /* UTIL_H_ */
