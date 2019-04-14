#include "raytracing.h"

#include "mesh.h"
#include "util.h"

bool RayHit::intersect_face(const Face &face, const glm::mat4 &modelmat) {
  glm::vec3 a, b, c, n;
  face.transformed_verts(modelmat, a, b, c);

  n = face.transformed_norm(modelmat);
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
  _face = (Face*) &face;
  _modelmat = modelmat;

  return true;
}

bool RayHit::intersect_mesh(const MeshInstance &mesh) {
  glm::mat4 modelmat = mesh.modelmat();
  Mesh *m = mesh.mesh();

  bool intersected = false;

  for (Mesh::face_iterator itr = m->faces_begin(); itr != m->faces_end(); ++itr) {
    intersected |= this->intersect_face(**itr, modelmat);
  }

  return intersected;
}

RayTreeNode::~RayTreeNode() {
  for (unsigned i = 0; i < _children.size(); ++i) {
    delete _children[i];
  }
}

void RayTreePtr::add_child(const RayHit &ray, const glm::vec3 &color) {
  assert(_node);

  RayTreeNode *child = new RayTreeNode(ray, color);
  _node->_children.push_back(child);
  glm::vec4 start_color(color.r, color.g, color.b, 1.0);
  glm::vec4 end_color(start_color);
  glm::vec3 end_point;

  if (!ray.intersected()) {
    end_color.a = 0.0;
    end_point = ray.ray().point_at(20.0);
  } else {
    end_point = ray.intersection_point();
  }

  _tree->_dbviz.add_line(ray.ray().origin(), end_point, start_color, end_color);
}

void RayTree::clear() {
  for (unsigned i = 0; i < _root._children.size(); ++i) {
    delete _root._children[i];
  }

  _root._children.clear();
  _dbviz.clear();
}
