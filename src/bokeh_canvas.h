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
#include "raytracing.h"
#include "scene.h"

#define MOUSE_BUTTON_LEFT   0x1
#define MOUSE_BUTTON_RIGHT  0x2
#define MOUSE_BUTTON_MIDDLE 0x4

struct MouseInfo {
  MouseInfo() : inited(false), buttons(0) {}
  bool inited;
  double x, y;
  uint32_t buttons;
};

struct BokehCanvasConf {
  unsigned width, height;
  unsigned shadow_samples;
  unsigned antialias_samples;
  unsigned num_bounces;
  std::string scnfile;
};

class BokehCanvas : public Canvas {
  public:
    BokehCanvas(const BokehCanvasConf &conf);
    const MouseInfo &mouse() const { return _mouse; }

  protected:
    void update();

  private:
    friend void bokeh_mousebuttoncb(GLFWwindow*, int, int, int);
    friend void bokeh_cursorposcb(GLFWwindow*, double, double);
    friend void bokeh_keyboardcb(GLFWwindow*, int, int, int, int);

    void trace_ray(double x, double y);

    DebugViz _dbviz;
    Scene _scene;
    MouseInfo _mouse;
    bool _draw_axes;
    bool _draw_raytracing;

    RayTracing _raytracing;
};

#endif /* BOKEH_CANVAS_H_ */
