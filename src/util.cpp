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

void handle_gl_error(const char *msg, bool warn) {
  GLenum error;
  bool err = false;
  while ((error = glGetError()) != GL_NO_ERROR) {
    err = true;
    if (msg) {
      *gl_error_stream << '[' << msg << "] ";
    }

    *gl_error_stream << "GL_ERROR: " << gl_error_to_string(error) << std::endl;
  }

  if (!warn && err) {
    exit(-1);
  }
}

void handle_shader_error(const char *msg, GLuint shader, bool warn) {
  GLint compile_status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
  if (compile_status == GL_FALSE) {
    GLsizei log_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
    char *log = new char[log_len];
    glGetShaderInfoLog(shader, log_len, NULL, log);

    if (msg) {
      *gl_error_stream << '[' << msg << "]\n";
    }

    *gl_error_stream << "SHADER COMPILE ERROR ======\n"
                  << log
                  <<    "===========================" << std::endl;
  }

  if (!(warn || compile_status == GL_TRUE)) {
    exit(-1);
  }
}

void handle_program_error(const char *msg, GLuint program, bool warn) {
  GLint link_status;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  if (link_status == GL_FALSE) {
    GLsizei log_len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
    char *log = new char[log_len];
    glGetProgramInfoLog(program, log_len, NULL, log);

    if (msg) {
      *gl_error_stream << '[' << msg << "]\n";
    }

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

void barycentric_coords(
    const glm::vec3 &point,
    const glm::vec3 &va, const glm::vec3 &vb, const glm::vec3 &vc,
    float &alpha, float &beta, float &gamma)
{
  glm::vec3 n = glm::cross(vb - va, vc - va);
  float A = 0.5f*glm::length(n);
  n = glm::normalize(n);

  glm::vec3 abr = glm::cross(vb - va, point - va);
  glm::vec3 bcr = glm::cross(vc - vb, point - vb);
  glm::vec3 car = glm::cross(va - vc, point - vc);

  float sgn_abr = glm::dot(n, abr) < 0 ? -1.0 : 1.0;
  float sgn_bcr = glm::dot(n, bcr) < 0 ? -1.0 : 1.0;
  float sgn_car = glm::dot(n, car) < 0 ? -1.0 : 1.0;

  alpha = sgn_bcr * glm::length(bcr) / A;
  beta = sgn_car * glm::length(car) / A;
  gamma = sgn_abr * glm::length(abr) / A;
}

static inline bool char_in_str(char c, const char *str) {
  const char *cur = str;
  while (*cur != '\0') {
    if (c == *cur) {
      return true;
    }
    ++cur;
  }
  return false;
}

std::vector<std::string> split(const std::string &str, const char *delims, bool multi) {
  if (str.empty()) {
    return std::vector<std::string>();
  }

  std::vector<std::string> vec;
  std::string accum;
  bool delim = false;
  for (unsigned i = 0; i < str.size(); ++i) {
    if (!char_in_str(str[i], delims)) {
      if (multi && delim) {
        vec.push_back(accum);
        accum.clear();
        --i;
        delim = false;
      } else {
        accum += str[i];
      }

      continue;
    }

    if (!multi) {
      vec.push_back(accum);
      accum.clear();
    } else {
      delim = true;
    }
  }

  if (!accum.empty()) {
    vec.push_back(accum);
  }

  return vec;
}

std::string strip(const std::string &str, const char *trailing) {
  unsigned i, j;

  for (i = 0; i < str.size(); ++i) {
    char c = str[i];
    if (!char_in_str(c, trailing)) {
      break;
    }
  }

  for (j = str.size() - 1; j < str.size(); --j) {
    char c = str[j];
    if (!char_in_str(c, trailing)) {
      break;
    }
  }

  ++j;
  return str.substr(i, j-i);
}

std::string dirname(const std::string &path) {
  if (path.empty()) {
    return std::string();
  }

  unsigned end;
  for (end = path.size(); end >= 0; --end) {
    if (path[end-1] == '/' || path[end-1] == '\\') {
      break;
    }
  }

  if (end == 0) {
    if (path[0] == '/') {
      return std::string();
    } else {
      return std::string(".");
    }
  }

  return path.substr(0, end);
}

glm::vec3 parse_vec3(const std::vector<std::string> &tokens, unsigned start_idx) {
  if (tokens.size() - start_idx < 3) {
    glerr() << "ERROR: insufficient components for 3-vector" << std::endl;
    exit(-1);
  }

  glm::vec3 vec;
  for (unsigned i = 0; i < 3; ++i) {
    float f;
    if (sscanf(tokens[start_idx + i].c_str(), "%f", &f) != 1) {
      glerr() << "ERROR: invalid vector component" << std::endl;
      exit(-1);
    }
    vec[i] = f;
  }

  return vec;
}

glm::vec3 parse_color(const std::vector<std::string> &tokens, unsigned start_idx) {
  glm::vec3 c = parse_vec3(tokens, start_idx);
  if (c.r < 0.0 || c.r > 1.0
   || c.g < 0.0 || c.g > 1.0
   || c.b < 0.0 || c.b > 1.0)
  {
    glerr() << "ERROR: invalid color component" << std::endl;
    exit(-1);
  }

  return c;
}

float parse_float(const std::vector<std::string> &tokens, unsigned idx) {
  if (tokens.size() - idx < 1) {
    glerr() << "ERROR: no float found" << std::endl;
    exit(-1);
  }

  float f;
  if (sscanf(tokens[idx].c_str(), "%f", &f) != 1) {
    glerr() << "ERROR: invalid floating-point parameter" << std::endl;
    exit(-1);
  }

  return f;
}

int parse_int(const std::vector<std::string> &tokens, unsigned idx) {
  if (tokens.size() - idx < 1) {
    glerr() << "ERROR: no int found" << std::endl;
    exit(-1);
  }

  int d;
  if (sscanf(tokens[idx].c_str(), "%d", &d) != 1) {
    glerr() << "ERROR: invalid integer parameter" << std::endl;
    exit(-1);
  }

  return d;
}
