#include "bokeh_canvas.h"

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

BokehCanvas::BokehCanvas(int width, int height, const BokehCanvasConf &conf)
  : Canvas(width, height, "Bokeh"),
    _camera(conf.cam_pos, conf.cam_poi, glm::vec3(0.0, 0.0, 1.0))
{
  GLFWwindow *window = this->window();
  glfwSetWindowUserPointer(window, (void*) this);

  glfwSetMouseButtonCallback(window, bokeh_mousebuttoncb);
  glfwSetCursorPosCallback(window, bokeh_cursorposcb);

  _dbviz.add_line(conf.cam_poi, conf.cam_poi + glm::vec3(1.0, 0.0, 0.0),
      glm::vec4(1.0, 0.0, 0.0, 1.0));
  _dbviz.add_line(conf.cam_poi, conf.cam_poi + glm::vec3(0.0, 1.0, 0.0),
      glm::vec4(0.0, 1.0, 0.0, 1.0));
  _dbviz.add_line(conf.cam_poi, conf.cam_poi + glm::vec3(0.0, 0.0, 1.0),
      glm::vec4(0.0, 0.0, 1.0, 1.0));
}

void BokehCanvas::update() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 view, proj;
  _camera.get_view_projection(view, proj);
  _dbviz.set_viewmat(view);
  _dbviz.set_projmat(proj);

  _dbviz.draw();

  glfwSwapBuffers(this->window());
}
