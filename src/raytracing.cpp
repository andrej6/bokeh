#include "raytracing.h"

#include "scene.h"
#include "mesh.h"
#include "util.h"

bool RayHit::intersect_face(const Face &face, const glm::mat4 &modelmat) {
  glm::vec3 a, b, c, n;
  face.verts_transformed(modelmat, a, b, c);

  n = face.norm_transformed(modelmat);
  float t = (glm::dot(n, a) - glm::dot(n, _ray.origin())) / glm::dot(n, _ray.direction());

  if (std::isnan(t) || std::isinf(t) || t < 0.0) {
    return false;
  }

  if (intersected() && t > _t) {
    return false;
  }

  glm::vec3 r = _ray.point_at(t);

  float alpha, beta, gamma;
  barycentric_coords(r, a, b, c, alpha, beta, gamma);
  if (alpha < 0.0 || beta < 0.0 || gamma < 0.0) {
    return false;
  }

  _t = t;
  _modelmat = modelmat;
  _norm = face.interpolate_norm_transformed(modelmat, alpha, beta, gamma);

  return true;
}

bool RayHit::intersect_mesh(const MeshInstance &mesh) {
  glm::mat4 modelmat = mesh.modelmat();
  Mesh *m = mesh.mesh();

  std::unordered_set<const Face*> culled_faces = m->kd_tree().collect_possible_faces(ray(), modelmat);

  bool intersected = false;

  for (auto itr = culled_faces.begin(); itr != culled_faces.end(); ++itr) {
    intersected |= this->intersect_face(**itr, modelmat);
  }

  if (intersected) {
    _mesh_instance = &mesh;
    _mtl_id = mesh.material_id();
  }

  return intersected;
}

const Material *RayHit::material() const {
  if (_mtl_id == Material::NONE) {
    return NULL;
  }

  return get_mtl(_mtl_id);
}

void RayTreeNode::add_child(const RayHit &hit, const glm::vec3 &color) {
  RayTreeNode *child = new RayTreeNode(_tree);
  _children.push_back(child);

  _color = color;
  glm::vec4 start_color(color.r, color.g, color.b, 1.0);
  glm::vec4 end_color(start_color);
  glm::vec3 end_point;

  if (!hit.intersected()) {
    end_color.a = 0.0;
    end_point = hit.ray().point_at(20.0);
  } else {
    end_point = hit.intersection_point();
  }

  _tree->_dbviz.add_line(hit.ray().origin(), end_point, start_color, end_color);
}

RayTreeNode::~RayTreeNode() {
  for (unsigned i = 0; i < _children.size(); ++i) {
    delete _children[i];
  }
}

void RayTree::clear() {
  for (unsigned i = 0; i < _root._children.size(); ++i) {
    delete _root._children[i];
  }

  _root._children.clear();
  _dbviz.clear();
}

void RayTracing::draw() {
  lazy_init_fbo();

  if (_fbo == 0) {
    return;
  }

  if (_dirty) {
    pack_data();
  }

  glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
  handle_gl_error("[RayTracing::draw] Binding framebuffer");

  glBlitFramebuffer(
      0, 0, Canvas::width(), Canvas::height(),
      0, 0, _image.width(), _image.height(),
      GL_COLOR_BUFFER_BIT, GL_NEAREST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  handle_gl_error("[RayTracing::draw] Leaving function");
}

bool RayTracing::trace_next_pixel() {
  if (_trace_y >= _image.height()) {
    return false;
  }

  double center_x = double(_trace_x) + 0.5;
  double center_y = double(_trace_y) + 0.5;

  glm::vec3 color = _scene->trace_ray(center_x, center_y, 3);
  _image.set_pixel(_trace_x, _trace_y, glm::vec4(color.r, color.g, color.b, 1.0));
  _dirty = true;

  ++_trace_x;
  if (_trace_x >= _image.width()) {
    _trace_x = 0;
    ++_trace_y;
  }

  return true;
}

void RayTracing::lazy_init_fbo() {
  if (_fbo != 0) {
    return;
  }

  if (!Canvas::active()) {
    return;
  }

  glGenTextures(1, &_tex);
  glBindTexture(GL_TEXTURE_RECTANGLE, _tex);
  handle_gl_error("[RayTracing::lazy_init_fbo] Genning/binding texture");

  pack_data();

  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
  handle_gl_error("[RayTracing::lazy_init_fbo] Genning/binding FBO");

  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_RECTANGLE, _tex, 0);
  handle_gl_error("[RayTracing::lazy_init_fbo] Setting FBO attachment");

  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

  handle_gl_error("[RayTracing::lazy_init_fbo] Leaving function");
}

void RayTracing::pack_data() {
  glBindTexture(GL_TEXTURE_RECTANGLE, _tex);
  handle_gl_error("[RayTracing::pack_data] Binding texture");

  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA8, _image.width(), _image.height(), 0,
      GL_RGBA, GL_UNSIGNED_BYTE, (const void*) _image.data());
  handle_gl_error("[RayTracing::pack_data] Leaving function");

  _dirty = false;
}
