#include "canvas.h"

#include <iostream>

#include <cstdlib>

#include "util.h"

#define _GLFW_INITED 0x1
#define _GLEW_INITED 0x2

const Canvas::canvas_id Canvas::NONE = 0;
Canvas::canvas_id Canvas::_s_next_id = 1;
unsigned Canvas::_s_num_canvases = 0;
Canvas *Canvas::_s_active = NULL;
int Canvas::_s_gl_initialized = 0;

void glfw_error_cb(int err, const char *msg) {
  glerr() << "Error " << err << " initializing GLFW: " << msg << std::endl;
  exit(-1);
}

Canvas::Canvas(int width, int height, const char *title) :
  _id(_s_next_id), _width(width), _height(height), _moved(false), _continue_updates(true), _vao(0)
{
  lazy_init_glfw();

  ++_s_num_canvases;
  ++_s_next_id;

  _window = glfwCreateWindow(_width, _height, title, NULL, NULL);
}

Canvas::Canvas(Canvas &&other) :
  _id(other._id), _width(other._width), _height(other._height), _moved(false),
  _continue_updates(true), _window(other._window), _vao(other._vao)
{
  other._moved = true;
  if (_s_active == &other) {
    make_active();
  }
}

Canvas &Canvas::operator=(Canvas &&other) {
  destroy_resources();
  --_s_num_canvases;
  if (_s_active == this) {
    _s_active = NULL;
    glfwMakeContextCurrent(NULL);
  }

  other._moved = true;
  _id = other._id;
  _width = other._width;
  _height = other._height;
  _window = other._window;
  _vao = other._vao;

  if (_s_active == &other) {
    make_active();
  }

  return *this;
}

Canvas::~Canvas() {
  if (!_moved) {
    destroy_resources();
    --_s_num_canvases;
    if (_s_active == this) {
      _s_active = NULL;
      glfwMakeContextCurrent(NULL);
    }

    if (_s_num_canvases == 0) {
      terminate_gl();
    }
  }
}

void Canvas::make_active() const {
  _s_active = (Canvas*) this;
  glfwMakeContextCurrent(_window);
  glfwSwapInterval(1);
  lazy_init_glew();

  if (_vao == 0) {
    glGenVertexArrays(1, (GLuint*) &_vao);
    handle_gl_error("[Canvas::make_active] Generating canvas VAO");
  }

  glBindVertexArray(_vao);
  handle_gl_error("[Canvas::make_active] Binding canvas VAO");
}

double Canvas::aspect() {
  int w, h;
  glfwGetFramebufferSize(_s_active->_window, &w, &h);
  return double(w) / h;
}

void Canvas::run_with_event_fn(void (*event_fn)()) {
  while (!glfwWindowShouldClose(_s_active->_window) && _s_active->_continue_updates) {
    event_fn();
    _s_active->update();
  }
  _s_active->_continue_updates = true;
}

void Canvas::lazy_init_glfw() {
  if ((_s_gl_initialized & _GLFW_INITED) == 0) {
    glfwSetErrorCallback(glfw_error_cb);
    glfwInit();

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    _s_gl_initialized |= _GLFW_INITED;
  }
}

void Canvas::lazy_init_glew() {
  if ((_s_gl_initialized & _GLEW_INITED) == 0) {
    if (glewInit() != GLEW_OK) {
      glerr() << "Error initializing GLEW" << std::endl;
      exit(-1);
    }

    _s_gl_initialized |= _GLEW_INITED;
  }
}

void Canvas::terminate_gl() {
  glfwTerminate();
  _s_gl_initialized &= ~_GLFW_INITED;
}

void Canvas::destroy_resources() {
  glDeleteVertexArrays(1, &_vao);
  glfwDestroyWindow(_window);
  _window = NULL;
}
