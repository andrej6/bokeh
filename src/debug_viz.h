// Debugging visualizations. A buncha lines.
#ifndef DEBUG_VIZ_H_
#define DEBUG_VIZ_H_

#include <vector>

#include <glm/glm.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "canvas.h"

struct DebugVizPoint {
  glm::vec3 pos;
  glm::vec4 col;
};

struct LineSegment {
  DebugVizPoint a, b;
};

struct DebugVizShaderData {
  DebugVizShaderData() : program(0), vpos_loc(0), vcol_loc(0), viewmat_loc(0), projmat_loc(0) {}

  GLuint program;

  GLuint vpos_loc;
  GLuint vcol_loc;
  GLuint viewmat_loc;
  GLuint projmat_loc;
};

// A handler for a debugging visualization. Manages lines to be drawn and
// can edit line width and toggle depth occlusion independent of other
// DebugViz objects.
class DebugViz {
  public:
    DebugViz();
    ~DebugViz();

    // Add a single-color line segment to this DebugViz.
    void add_line(const glm::vec3 &start, const glm::vec3 &end,
        const glm::vec4 &color);

    // Add a line segment to this DebugViz that interpolates two colors over
    // its length.
    void add_line(const glm::vec3 &start, const glm::vec3 &end,
        const glm::vec4 &start_color, const glm::vec4 &end_color);

    // Add a line segment by its direction and length, rather than its endpoints.
    void add_ray(const glm::vec3 &start, const glm::vec3 &dir, float len,
        const glm::vec4 &color);
    void add_ray(const glm::vec3 &start, const glm::vec3 &dir, float len,
        const glm::vec4 &start_color, const glm::vec4 &end_color);

    // Draw the contents of this DebugViz.
    void draw();

    // Clear this DebugViz.
    void clear();

    // Set and query the line width of this DebugViz.
    void set_line_width(float width) { _line_width = width; }
    float line_width() const { return _line_width; }

    // Toggle and query the state of depth occlusion on this DebugViz.
    void toggle_depth_test() { _depth_test = !_depth_test; }
    bool depth_test() const { return _depth_test; }

    void set_viewmat(const glm::mat4 &view);
    void set_projmat(const glm::mat4 &proj);

  private:
    void lazy_init_shaders();
    void add_line_segment(const glm::vec3 &a, const glm::vec3 &b,
        const glm::vec4 &ca, const glm::vec4 &cb);
    void pack_data();

    Canvas::canvas_id _canvas;
    bool _dirty;
    DebugVizShaderData _shader;
    GLuint _vbuf;

    bool _depth_test;
    float _line_width;

    std::vector<LineSegment> _lines;
};

#endif /* DEBUG_VIZ_H_ */
