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
  double prevx, prevy;
  if (canvas->_mouse.inited) {
    prevx = canvas->_mouse.x;
    prevy = canvas->_mouse.y;
  } else {
    prevx = x;
    prevy = y;
    canvas->_mouse.inited = true;
  }

  uint32_t buttons = canvas->_mouse.buttons;
  Camera *cam = canvas->_scene.camera();
  if (buttons & MOUSE_BUTTON_LEFT) {
    cam->rotate(prevx - x, prevy - y);
  }

  if (buttons & MOUSE_BUTTON_MIDDLE) {
    cam->truck(prevx - x, y - prevy);
  }

  if (buttons & MOUSE_BUTTON_RIGHT) {
    cam->dolly(prevy - y);
  }

  canvas->_mouse.x = x;
  canvas->_mouse.y = y;
}

void bokeh_keyboardcb(GLFWwindow *window, int key, int scancode, int action, int mods) {
  BokehCanvas *canvas = (BokehCanvas*) glfwGetWindowUserPointer(window);
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_A:
        canvas->_draw_axes = !canvas->_draw_axes;
        break;

      case GLFW_KEY_D:
        canvas->_dbviz.toggle_depth_test();
        break;

      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(canvas->window(), GLFW_TRUE);
        break;

      case GLFW_KEY_T:
        canvas->_scene.visualize_raytree(canvas->_mouse.x, canvas->_mouse.y);
        break;
    }
  }
}

BokehCanvas::BokehCanvas(int width, int height, const BokehCanvasConf &conf)
  : Canvas(width, height, "Bokeh"),
    _scene(Scene::from_scn(conf.scnfile.c_str())),
    _draw_axes(false)
{
  GLFWwindow *window = this->window();
  glfwSetWindowUserPointer(window, (void*) this);

  glfwSetMouseButtonCallback(window, bokeh_mousebuttoncb);
  glfwSetCursorPosCallback(window, bokeh_cursorposcb);
  glfwSetKeyCallback(window, bokeh_keyboardcb);

  _dbviz.add_line(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec4(1, 0, 0, 1));
  _dbviz.add_line(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec4(0, 1, 0, 1));
  _dbviz.add_line(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec4(0, 0, 1, 1));
}

void BokehCanvas::update() {
  glm::vec3 bg_color = _scene.bg_color();
  glClearColor(bg_color.r, bg_color.g, bg_color.b, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  _scene.draw();

  glm::mat4 view, proj;
  _scene.camera()->get_view_projection(view, proj);

  if (_draw_axes) {
    _dbviz.set_viewmat(view);
    _dbviz.set_projmat(proj);
    _dbviz.draw();
  }

  glfwSwapBuffers(this->window());
}
