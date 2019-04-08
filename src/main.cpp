#include <iostream>

#include "canvas.h"
#include "bokeh_canvas.h"

int main(int argc, char **argv) {
  BokehCanvas canvas(640, 480, { glm::vec3(1.0, 4.5, 3.0), glm::vec3(0.0) });
  canvas.make_active();

  Canvas::run();

  return 0;
}
