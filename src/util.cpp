#include "util.h"

#include <random>
#include <string>

#include <cstdlib>
#include <ctime>

const double PI = acos(-1);
std::ostream *gl_error_stream = &std::cerr;

const char *gl_error_to_string(GLenum err) {
  switch (err) {
    case GL_NO_ERROR:
      return "NO_ERROR";
    case GL_INVALID_ENUM:
      return "INVALID_ENUM";
    case GL_INVALID_VALUE:
      return "INVALID_VALUE";
    case GL_INVALID_OPERATION:
      return "INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
      return "OUT_OF_MEMORY";
    case GL_STACK_UNDERFLOW:
      return "STACK_UNDERFLOW";
    case GL_STACK_OVERFLOW:
      return "STACK_OVERFLOW";
    default:
      return "OTHER GL ERROR";
  }
}

void handle_gl_error(const std::string &msg, bool warn) {
  GLenum error;
  bool err = false;
  while ((error = glGetError()) != GL_NO_ERROR) {
    err = true;
    if (!msg.empty()) {
      *gl_error_stream << '[' << msg << "] ";
    }

    *gl_error_stream << "GL_ERROR: " << gl_error_to_string(error) << std::endl;
  }

  if (!warn && err) {
    exit(-1);
  }
}

void handle_shader_error(GLuint shader, bool warn) {
  GLint compile_status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
  if (compile_status == GL_FALSE) {
    GLsizei log_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
    char *log = new char[log_len];
    glGetShaderInfoLog(shader, log_len, NULL, log);
    *gl_error_stream << "SHADER COMPILE ERROR ======"
                  << log
                  <<  "\n===========================" << std::endl;
  }

  if (!(warn || compile_status == GL_TRUE)) {
    exit(-1);
  }
}

void handle_program_error(GLuint program, bool warn) {
  GLint link_status;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  if (link_status == GL_FALSE) {
    GLsizei log_len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
    char *log = new char[log_len];
    glGetProgramInfoLog(program, log_len, NULL, log);
    *gl_error_stream << "SHADER COMPILE ERROR ======"
                  << log
                  <<  "\n===========================" << std::endl;
  }

  if (!(warn || link_status == GL_TRUE)) {
    exit(-1);
  }
}

static std::mt19937 rand_engine(time(NULL));
static std::uniform_real_distribution<double> rand_distr(0.0, 1.0);

double randf() {
  return rand_distr(rand_engine);
}

uint32_t randi() {
  return rand_engine();
}
