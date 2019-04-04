#ifndef CANVAS_H_
#define CANVAS_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Canvas {
  public:
    Canvas(int width, int height, const char *title);

    // Can't be copied, can be moved
    Canvas(const Canvas&) = delete;
    Canvas &operator=(const Canvas&) = delete;
    Canvas(Canvas&&);
    Canvas &operator=(Canvas&&);

    ~Canvas();

    void make_active() const {
      _s_active = (Canvas*) this;
      glfwMakeContextCurrent(_window);
      glfwSwapInterval(1);
      lazy_init_glew();
    }

    static GLFWwindow *window() { return _s_active->_window; }

  private:
    static void lazy_init_glfw();
    static void lazy_init_glew();
    static void terminate_gl();

    static unsigned _s_num_canvases;
    static Canvas *_s_active;
    static int _s_gl_initialized;

    void destroy_resources();

    unsigned _width, _height;
    bool _moved;

    GLFWwindow *_window;
};

#endif /* CANVAS_H_ */
