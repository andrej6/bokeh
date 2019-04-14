#include "debug_viz.h"

#include "shader_store.h"
#include "util.h"

#define SHADER_PROG_NAME "dbviz"
#define VERT_SHADER_NAME "dbviz.vert"
#define FRAG_SHADER_NAME "dbviz.frag"
#define VERT_SHADER_FILE "shaders/" VERT_SHADER_NAME
#define FRAG_SHADER_FILE "shaders/" FRAG_SHADER_NAME

bool DebugViz::_depth_test = true;

DebugViz::DebugViz() : _dirty(false), _line_width(1.0) {
  _vbuf = 0;
  _vao = 0;
  lazy_init_shaders();
}

DebugViz::~DebugViz() {
  if (_vao != 0) {
    glDeleteBuffers(1, &_vbuf);
    glDeleteBuffers(1, &_vao);
    handle_gl_error("[~DebugViz] Deleting DebugViz vertex buffer");
  }
}

void DebugViz::add_line(const glm::vec3 &start, const glm::vec3 &end,
    const glm::vec4 &color)
{
  add_line_segment(start, end, color, color);
}

void DebugViz::add_line(const glm::vec3 &start, const glm::vec3 &end,
    const glm::vec4 &start_color, const glm::vec4 &end_color)
{
  add_line_segment(start, end, start_color, end_color);
}

void DebugViz::add_ray(const glm::vec3 &start, const glm::vec3 &dir, float len,
    const glm::vec4 &color)
{
  add_line_segment(start, start + len*glm::normalize(dir), color, color);
}

void DebugViz::add_ray(const glm::vec3 &start, const glm::vec3 &dir, float len,
    const glm::vec4 &start_color, const glm::vec4 &end_color)
{
  add_line_segment(start, start + len*glm::normalize(dir), start_color, end_color);
}

void DebugViz::clear() {
  _lines.clear();
  _dirty = true;
}

void DebugViz::draw() {
  lazy_init_shaders();
  glBindVertexArray(_vao);
  glUseProgram(_shader.program);
  handle_gl_error("[DebugViz::draw] Using DebugViz program");

  if (_dirty) {
    pack_data();
  }

  if (_lines.empty()) {
    return;
  }

  if (_depth_test) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glLineWidth(_line_width);

  glUniformMatrix4fv(_shader.modelmat_loc, 1, false, (float*) &_modelmat);
  glUniformMatrix4fv(_shader.viewmat_loc, 1, false, (float*) &_viewmat);
  glUniformMatrix4fv(_shader.projmat_loc, 1, false, (float*) &_projmat);

  glDrawArrays(GL_LINES, 0, 2 * _lines.size());
  handle_gl_error("[DebugViz::draw] Leaving function");
}

void DebugViz::set_modelmat(const glm::mat4 &model) {
  _modelmat = model;
}

void DebugViz::set_viewmat(const glm::mat4 &view) {
  _viewmat = view;
}

void DebugViz::set_projmat(const glm::mat4 &proj) {
  _projmat = proj;
}

void DebugViz::pack_data() {
  lazy_init_shaders();
  if (_lines.empty()) {
    return;
  }

  glBindVertexArray(_vao);
  glUseProgram(_shader.program);
  glBindBuffer(GL_ARRAY_BUFFER, _vbuf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(LineSegment) * _lines.size(), _lines.data(), GL_DYNAMIC_DRAW);
  handle_gl_error("[DebugViz::pack_data] Packing DebugViz lines");
  _dirty = false;
}

void DebugViz::add_line_segment(const glm::vec3 &a, const glm::vec3 &b,
    const glm::vec4 &ca, const glm::vec4 &cb)
{
  _dirty = true;
  _lines.push_back({{ a, ca }, { b, cb }});
}

void DebugViz::lazy_init_shaders() {
  if (_vao != 0) {
    return;
  }

  if (!Canvas::active()) {
    return;
  }

  add_shader_source_file(VERT_SHADER_NAME, VERT_SHADER_FILE, GL_VERTEX_SHADER);
  add_shader_source_file(FRAG_SHADER_NAME, FRAG_SHADER_FILE, GL_FRAGMENT_SHADER);

  compile_shader(VERT_SHADER_NAME);
  compile_shader(FRAG_SHADER_NAME);

  add_program(SHADER_PROG_NAME);

  add_shader_to_program(VERT_SHADER_NAME, SHADER_PROG_NAME);
  add_shader_to_program(FRAG_SHADER_NAME, SHADER_PROG_NAME);

  link_program(SHADER_PROG_NAME);

  _shader.program = program_loc(SHADER_PROG_NAME);

  _shader.vpos_loc = glGetAttribLocation(_shader.program, "vpos");
  _shader.vcol_loc = glGetAttribLocation(_shader.program, "vcol");

  _shader.modelmat_loc = glGetUniformLocation(_shader.program, "modelmat");
  _shader.viewmat_loc = glGetUniformLocation(_shader.program, "viewmat");
  _shader.projmat_loc = glGetUniformLocation(_shader.program, "projmat");

  handle_gl_error("[DebugViz::lazy_init_shaders] Before using program");

  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbuf);
  glBindVertexArray(_vao);

  glUseProgram(_shader.program);
  glEnableVertexAttribArray(_shader.vpos_loc);
  glEnableVertexAttribArray(_shader.vcol_loc);
  handle_gl_error("[DebugViz::lazy_init_shaders] After enabling attribs");

  glm::mat4 viewprojmat(1.0);
  glUniformMatrix4fv(_shader.modelmat_loc, 1, false, (float*) &viewprojmat);
  glUniformMatrix4fv(_shader.viewmat_loc, 1, false, (float*) &viewprojmat);
  glUniformMatrix4fv(_shader.projmat_loc, 1, false, (float*) &viewprojmat);
  handle_gl_error("[DebugViz::lazy_init_shaders] After setting matrices");

  glBindBuffer(GL_ARRAY_BUFFER, _vbuf);
  glVertexAttribPointer(_shader.vpos_loc, 3, GL_FLOAT, false,
      sizeof(DebugVizPoint), (void*) offsetof(DebugVizPoint, pos));
  glVertexAttribPointer(_shader.vcol_loc, 4, GL_FLOAT, false,
      sizeof(DebugVizPoint), (void*) offsetof(DebugVizPoint, col));
  handle_gl_error("[DebugViz::draw] After setting attrib ptrs");
  handle_gl_error("[DebugViz::lazy_init_shaders] Initializing DebugViz vertex buffer");
}
