#ifndef UTIL_H_
#define UTIL_H_

#include <iostream>
#include <string>
#include <vector>

#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#define EPSILON 0.00075f
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
static inline glm::vec3 rand_vec() {
  float x = 2*randf() - 1.0,
        y = 2*randf() - 1.0,
        z = 2*randf() - 1.0;
  return glm::vec3(x, y, z);
}

// Construct a unit vector with angle theta in the xy plane and angle phi off
// of the positive z-axis.
static inline glm::vec3 unit_vec_from_angles(float theta, float phi) {
  return glm::vec3(cos(theta)*sin(phi), sin(theta)*sin(phi), cos(phi));
}

// Generate a uniform random vector on the unit sphere centered at the origin.
static inline glm::vec3 rand_unit_vec() {
  return unit_vec_from_angles(2*PI*randf(), PI*randf());
}

// Generate uniform random barycentric coordinates (three non-negative numbers
// that sum to 1).
static inline glm::vec3 rand_barycentric() {
  double r1 = randf();
  double sqrt_r2 = sqrt(randf());

  return glm::vec3(float(1 - sqrt_r2), float(sqrt_r2*(1.0 - r1)), float(r1*sqrt_r2));
}

#define VEC3_DIR   0
#define VEC3_POINT 1

// Apply 4x4 homogeneous matrix to a 3-vector, and get a homogenized 3-vector of
// the result. The third argument `type` should be one of `VEC3_POINT` or
// `VEC3_DIR`; the former indicates that the full affine transformation, including
// translation, should be applied, while the latter indicates that only the
// linear parts of the transformation should be applied.
glm::vec3 apply_homog(const glm::mat4 &mat, const glm::vec3 &vec, int type = VEC3_POINT);

void barycentric_coords(const glm::vec3 &point,
    const glm::vec3 &va, const glm::vec3 &vb, const glm::vec3 &vc,
    float &alpha, float &beta, float &gamma);

// Convert degrees to radians.
static inline double deg_to_rad(double d) {
  return d * PI / 180.0;
}

// Convert radians to degrees.
static inline double rad_to_deg(double r) {
  return r * 180.0 / PI;
}

// Split a string into substrings delimited by the characters in `delims`. If `multi` is
// true, split() will treat any sequence of consecutive characters in `delims` as a single
// delimiter, and not produce an empty string for each consecutive pair.
std::vector<std::string> split(
    const std::string &str,
    const char *delims = " \t\n",
    bool multi=true);

// Strip a string of leading and trailing characters, by default whitespace.
std::string strip(const std::string &str, const char *trailing = " \t\n");

// Return the leading directory part of the given path. If there is no leading
// directory, outputs ".".
std::string dirname(const std::string &path);

glm::vec3 parse_vec3(const std::vector<std::string> &tokens, unsigned start_idx);

float parse_float(const std::vector<std::string> &tokens, unsigned idx);

int parse_int(const std::vector<std::string> &tokens, unsigned idx);

#endif /* UTIL_H_ */
