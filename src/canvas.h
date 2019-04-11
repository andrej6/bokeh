// Window and context management.
#ifndef CANVAS_H_
#define CANVAS_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdint>

// A top-level window and OpenGL context manager class.
//
// Most methods are static and operate on the active Canvas. A Canvas
// can be made active with the `make_active()` method, after which it
// will be accessible from anywhere in the program via static methods.
//
// This class is designed to be subclassed to handle application-specific
// behavior. The protected virual function `update()` is called on each
// update event, and should be overridden in subclasses to contain their
// own custom behavior. The implementation of `update()` in the base
// Canvas class does nothing.
//
// In order for a Canvas to begin updates, one of `run()` or
// `run_wait_events()` must be called. The former continually polls for
// new events and updates the Canvas. The latter blocks waiting for new
// events before updating. `run()` is preferred for real-time applications.
//
// A subclass's `update()` function may call the protected method
// `stop_updates()` to halt updates on the current Canvas. The halt will
// take effect after the next event or poll for events. Afterwards, the
// Canvas will need to be made active and run for updates to resume.
class Canvas {
  public:
    typedef uint32_t canvas_id;
    static const canvas_id NONE;

    Canvas(int width, int height, const char *title);

    // Can't be copied, but can be moved
    Canvas(const Canvas&) = delete;
    Canvas &operator=(const Canvas&) = delete;
    Canvas(Canvas&&);
    Canvas &operator=(Canvas&&);

    virtual ~Canvas();

    // Make this Canvas the active Canvas.
    void make_active() const;

    canvas_id id() const { return _id; }

    // Return a pointer to the active Canvas, or NULL if there is none.
    static Canvas *active() { return _s_active; }

    template <typename C>
      static C *active_as() { return static_cast<C*>(_s_active); }

    // Return the ID of the currently active canvas, or Canvas::NONE if there
    // is none. Canvas ID's are unique and persist when a Canvas is moved.
    static canvas_id active_id() {
      if (_s_active) {
        return _s_active->_id;
      } else {
        return NONE;
      }
    }

    // Begin updates on the active Canvas, polling (rather than waiting) for
    // events before each update.
    static void run() {
      run_with_event_fn(glfwPollEvents);
    }

    // Begin updates on the active Canvas, waiting (rather than polling) for
    // events before each update.
    static void run_wait_events() {
      run_with_event_fn(glfwWaitEvents);
    }

    // Return the aspect ratio (width / height) of the active Canvas's
    // framebuffer.
    static double aspect();

    static int width();
    static int height();

  protected:
    // Get a pointer to this Canvas's GLFW window object.
    GLFWwindow *window() { return _window; }

    // Halt updates on the current Canvas.
    void stop_updates() { _continue_updates = false; }

    // Action performed on each update of the Canvas.
    virtual void update() {}

  private:
    // Functions to lazily initialize (and terminate) GLFW and GLEW.
    static void lazy_init_glfw();
    static void lazy_init_glew();
    static void terminate_gl();

    static canvas_id _s_next_id;
    static unsigned _s_num_canvases;
    static Canvas *_s_active;
    static int _s_gl_initialized;

    // Free OpenGL and GLFW resources held by the Canvas.
    void destroy_resources();

    // Start the update loop, calling the given function to wait or poll for
    // events before each update.
    static void run_with_event_fn(void (*event_fn)());

    canvas_id _id;
    unsigned _width, _height;
    bool _moved;
    bool _continue_updates;

    GLFWwindow *_window;
};

#endif /* CANVAS_H_ */
