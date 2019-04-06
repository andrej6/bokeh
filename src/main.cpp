#include <iostream>

#include "canvas.h"

int main(int argc, char **argv) {
  Canvas canvas(640, 360, "Bokeh");
  canvas.make_active();

  Canvas::run();

  return 0;
}
