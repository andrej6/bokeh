#ifndef BOKEH_CANVAS_H_
#define BOKEH_CANVAS_H_

#include <string>
#include <vector>

#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "canvas.h"
#include "debug_viz.h"
#include "mesh.h"

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
  glm::vec3 bg_color;
  std::string objfile;
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
    friend void bokeh_keyboardcb(GLFWwindow*, int, int, int, int);

    void randomize_buns();

    glm::vec3 _bg_color;
    PerspectiveCamera _camera;
    DebugViz _dbviz;
    MouseInfo _mouse;
    std::vector<MeshInstance> _meshes;
};

#endif /* BOKEH_CANVAS_H_ */
