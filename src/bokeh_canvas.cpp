#include "bokeh_canvas.h"

#include "mesh.h"

void bokeh_mousebuttoncb(GLFWwindow *window, int button, int action, int mods) {
  BokehCanvas *canvas = (BokehCanvas*) glfwGetWindowUserPointer(window);

  switch (button) {
    case GLFW_MOUSE_BUTTON_1:
      if (action == GLFW_PRESS) {
        canvas->_mouse.buttons |= MOUSE_BUTTON_LEFT;
      } else {
        canvas->_mouse.buttons &= ~MOUSE_BUTTON_LEFT;
      }
      break;

    case GLFW_MOUSE_BUTTON_2:
      if (action == GLFW_PRESS) {
        canvas->_mouse.buttons |= MOUSE_BUTTON_RIGHT;
      } else {
        canvas->_mouse.buttons &= ~MOUSE_BUTTON_RIGHT;
      }
      break;

    case GLFW_MOUSE_BUTTON_3:
      if (action == GLFW_PRESS) {
        canvas->_mouse.buttons |= MOUSE_BUTTON_MIDDLE;
      } else {
        canvas->_mouse.buttons &= ~MOUSE_BUTTON_MIDDLE;
      }
      break;

    default:
      break;
  }
}

void bokeh_cursorposcb(GLFWwindow *window, double x, double y) {
  BokehCanvas *canvas = (BokehCanvas*) glfwGetWindowUserPointer(window);
  double prevx = canvas->_mouse.x, prevy = canvas->_mouse.y;

  uint32_t buttons = canvas->_mouse.buttons;
  if (buttons & MOUSE_BUTTON_LEFT) {
    canvas->_camera.rotate(prevx - x, prevy - y);
  }

  if (buttons & MOUSE_BUTTON_MIDDLE) {
    canvas->_camera.truck(prevx - x, y - prevy);
  }

  if (buttons & MOUSE_BUTTON_RIGHT) {
    canvas->_camera.dolly(prevy - y);
  }

  canvas->_mouse.x = x;
  canvas->_mouse.y = y;
}

void bokeh_keyboardcb(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_R && action == GLFW_PRESS) {
    BokehCanvas *canvas = (BokehCanvas*) glfwGetWindowUserPointer(window);
    canvas->randomize_buns();
  }
}

BokehCanvas::BokehCanvas(int width, int height, const BokehCanvasConf &conf)
  : Canvas(width, height, "Bokeh"),
    _camera(conf.cam_pos, conf.cam_poi, glm::vec3(0.0, 0.0, 1.0))
{
  GLFWwindow *window = this->window();
  glfwSetWindowUserPointer(window, (void*) this);

  glfwSetMouseButtonCallback(window, bokeh_mousebuttoncb);
  glfwSetCursorPosCallback(window, bokeh_cursorposcb);
  glfwSetKeyCallback(window, bokeh_keyboardcb);

  /*
  float size = 2.0;
  glm::vec3 o(conf.cam_poi);
  glm::vec3 x(conf.cam_poi + size*glm::vec3(1.0, 0.0, 0.0));
  glm::vec3 y(conf.cam_poi + size*glm::vec3(0.0, 1.0, 0.0));
  glm::vec3 z(conf.cam_poi + size*glm::vec3(0.0, 0.0, 1.0));
  glm::vec3 nx(conf.cam_poi - size*glm::vec3(1.0, 0.0, 0.0));
  glm::vec3 ny(conf.cam_poi - size*glm::vec3(0.0, 1.0, 0.0));
  glm::vec3 nz(conf.cam_poi - size*glm::vec3(0.0, 0.0, 1.0));
  glm::vec4 r(1.0, 0.0, 0.0, 1.0);
  glm::vec4 g(0.0, 1.0, 0.0, 1.0);
  glm::vec4 b(0.0, 0.0, 1.0, 1.0);

  _dbviz.add_line(o, x, r);
  _dbviz.add_line(o, y, g);
  _dbviz.add_line(o, z, b);

  _dbviz.add_line(o, nx, r, b);
  _dbviz.add_line(o, ny, g, r);
  _dbviz.add_line(o, nz, b, g);

  _dbviz.add_line(x, y, r, g);
  _dbviz.add_line(y, z, g, b);
  _dbviz.add_line(z, x, b, r);

  _dbviz.add_line(nx, ny, r);
  _dbviz.add_line(ny, nz, g);
  _dbviz.add_line(nz, nx, b);
  */

  _bg_color = conf.bg_color;

  MeshInstance bun(add_mesh_from_obj("bunny_mesh", conf.objfile.c_str()));

  for (unsigned i = 0; i < 16; ++i) {
    _meshes.push_back(MeshInstance(bun));
    _meshes[i].translate(0.75f*randvec());
    _meshes[i].rotate(2*PI*randf(), randvec());
    _meshes[i].set_scale(glm::vec3(10.0));
  }

  glEnable(GL_CULL_FACE);
}

void BokehCanvas::update() {
  glClearColor(_bg_color.r, _bg_color.g, _bg_color.b, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 view, proj;
  _camera.get_view_projection(view, proj);

  for (unsigned i = 0; i < _meshes.size(); ++i) {
    _meshes[i].set_viewmat(view);
    _meshes[i].set_projmat(proj);
    _meshes[i].draw();
  }

  _dbviz.set_viewmat(view);
  _dbviz.set_projmat(proj);
  _dbviz.draw();

  glfwSwapBuffers(this->window());
}

void BokehCanvas::randomize_buns() {
  for (unsigned i = 0; i < _meshes.size(); ++i) {
    _meshes[i].set_translate(0.75f*randvec());
    _meshes[i].set_rotate(2*PI*randf(), randvec());
  }
}
