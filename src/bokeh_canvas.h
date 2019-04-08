#ifndef BOKEH_CANVAS_H_
#define BOKEH_CANVAS_H_

#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "canvas.h"
#include "debug_viz.h"

#define MOUSE_BUTTON_LEFT   0x1
#define MOUSE_BUTTON_RIGHT  0x2
#define MOUSE_BUTTON_MIDDLE 0x4

struct MouseInfo {
  double x, y;
  uint32_t buttons;
};

struct BokehCanvasConf {
  glm::vec3 cam_pos;
  glm::vec3 cam_poi;
};

class BokehCanvas : public Canvas {
  public:
    BokehCanvas(int width, int height, const BokehCanvasConf &conf);
    const MouseInfo &mouse() const { return _mouse; }

  protected:
    void update();

  private:
    friend void bokeh_mousebuttoncb(GLFWwindow*, int, int, int);
    friend void bokeh_cursorposcb(GLFWwindow*, double, double);

    PerspectiveCamera _camera;
    DebugViz _dbviz;
    MouseInfo _mouse;
};

#endif /* BOKEH_CANVAS_H_ */
