#include "kd_tree.h"

#include <algorithm>

#include "mesh.h"
#include "raytracing.h"
#include "util.h"

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

static bool ray_plane_intersect(const Ray &ray, int axis, float plane, glm::vec3 &point) {
  assert(axis == X_AXIS || axis == Y_AXIS || axis == Z_AXIS);
  float orig_dim, dir_dim;
  if (axis == X_AXIS) {
    orig_dim = ray.origin().x;
    dir_dim = ray.direction().x;
  } else if (axis == Y_AXIS) {
    orig_dim = ray.origin().y;
    dir_dim = ray.direction().y;
  } else {
    orig_dim = ray.origin().z;
    dir_dim = ray.direction().z;
  }

  if (fabs(dir_dim) < EPSILON) {
    return false;
  }

  float t = (plane - orig_dim) / dir_dim;
  if (t < 0) {
    return false;
  }

  point = ray.point_at(t);
  return true;
}

static bool point_within_face(const glm::vec3 &point, int axis,
    const glm::vec3 &min, const glm::vec3 &max)
{
  assert(axis == X_AXIS || axis == Y_AXIS || axis == Z_AXIS);
  float dim1_min, dim2_min;
  float dim1_max, dim2_max;
  float dim1, dim2;

  if (axis == X_AXIS) {
    dim1_min = min.y; dim2_min = min.z;
    dim1_max = max.y; dim2_max = max.z;
    dim1 = point.y; dim2 = point.z;
  } else if (axis == Y_AXIS) {
    dim1_min = min.x; dim2_min = min.z;
    dim1_max = max.x; dim2_max = max.z;
    dim1 = point.x; dim2 = point.z;
  } else {
    dim1_min = min.x; dim2_min = min.y;
    dim1_max = max.x; dim2_max = max.y;
    dim1 = point.x; dim2 = point.y;
  }

  return dim1_min <= dim1 && dim1 <= dim1_max
      && dim2_min <= dim2 && dim2 <= dim2_max;
}

bool BBox::ray_intersects(const Ray &ray) const {
  glm::vec3 point;
  if (ray_plane_intersect(ray, X_AXIS, _min.x, point)) {
    if (point_within_face(point, X_AXIS, _min, _max)) return true;
  }
  if (ray_plane_intersect(ray, X_AXIS, _max.x, point)) {
    if (point_within_face(point, X_AXIS, _min, _max)) return true;
  }
  if (ray_plane_intersect(ray, Y_AXIS, _min.y, point)) {
    if (point_within_face(point, Y_AXIS, _min, _max)) return true;
  }
  if (ray_plane_intersect(ray, Y_AXIS, _max.y, point)) {
    if (point_within_face(point, Y_AXIS, _min, _max)) return true;
  }
  if (ray_plane_intersect(ray, Z_AXIS, _min.z, point)) {
    if (point_within_face(point, Z_AXIS, _min, _max)) return true;
  }
  if (ray_plane_intersect(ray, Z_AXIS, _max.z, point)) {
    if (point_within_face(point, Z_AXIS, _min, _max)) return true;
  }

  return false;
}

void BBox::add_debug_lines(DebugViz &dbviz, const glm::mat4 &modelmat) const {
  glm::vec3 pt000 = apply_homog(modelmat, _min, VEC3_POINT);
  glm::vec3 pt001 = apply_homog(modelmat, glm::vec3(_min.x, _min.y, _max.z), VEC3_POINT);
  glm::vec3 pt010 = apply_homog(modelmat, glm::vec3(_min.x, _max.y, _min.z), VEC3_POINT);
  glm::vec3 pt011 = apply_homog(modelmat, glm::vec3(_min.x, _max.y, _max.z), VEC3_POINT);
  glm::vec3 pt100 = apply_homog(modelmat, glm::vec3(_max.x, _min.y, _min.z), VEC3_POINT);
  glm::vec3 pt101 = apply_homog(modelmat, glm::vec3(_max.x, _min.y, _max.z), VEC3_POINT);
  glm::vec3 pt110 = apply_homog(modelmat, glm::vec3(_max.x, _max.y, _min.z), VEC3_POINT);
  glm::vec3 pt111 = apply_homog(modelmat, _max, VEC3_POINT);

  glm::vec4 color(0.7, 0.9, 1.0, 1.0);

  dbviz.add_line(pt000, pt001, color);
  dbviz.add_line(pt000, pt010, color);
  dbviz.add_line(pt000, pt100, color);
  dbviz.add_line(pt001, pt011, color);
  dbviz.add_line(pt001, pt101, color);
  dbviz.add_line(pt010, pt011, color);
  dbviz.add_line(pt010, pt110, color);
  dbviz.add_line(pt011, pt111, color);
  dbviz.add_line(pt100, pt101, color);
  dbviz.add_line(pt100, pt110, color);
  dbviz.add_line(pt101, pt111, color);
  dbviz.add_line(pt110, pt111, color);
}

#define SPLIT_LEFT    0
#define SPLIT_RIGHT   1
#define SPLIT_NEITHER 2

struct CompByAxis {
  int axis;

  float get_coord(const glm::vec3 &v) const {
    if (axis == X_AXIS) {
      return v.x;
    } else if (axis == Y_AXIS) {
      return v.y;
    } else {
      return v.z;
    }
  }

  bool operator()(const Face *f1, const Face *f2) const {
    glm::vec3 p1 = f1->centroid();
    glm::vec3 p2 = f2->centroid();
    return (*this)(p1, get_coord(p2));
  }

  bool operator()(const Face *f, float plane) const {
    glm::vec3 p = f->centroid();
    return (*this)(p, plane);
  }

  bool operator()(const glm::vec3 &v, float plane) const {
    return get_coord(v) < plane;
  }

  int face_split(const Face *f, float plane) const {
    int count = 0;
    for (unsigned i = 0; i < 3; ++i) {
      if (!(*this)(f->vert(i)->position(), plane)) {
        count++;
      }
    }

    if (count == 0) {
      return SPLIT_LEFT;
    } else if (count == 3) {
      return SPLIT_RIGHT;
    } else {
      return SPLIT_NEITHER;
    }
  }
};

KDTree::KDTree(const Mesh *mesh) : _child1(NULL), _child2(NULL) {
  std::vector<const Face*> faces;
  sorted_data sorted;

  glm::vec3 min(1.0f/0.0f); // Get ourselves some nice infinities up in here...
  glm::vec3 max(-1.0f/0.0f);

  for (Mesh::face_iterator fi = mesh->faces_begin(); fi != mesh->faces_end(); ++fi) {
    for (unsigned v = 0; v < 3; ++v) {
      max.x = std::max((*fi)->vert(v)->position().x, max.x);
      max.y = std::max((*fi)->vert(v)->position().y, max.y);
      max.z = std::max((*fi)->vert(v)->position().z, max.z);
      min.x = std::min((*fi)->vert(v)->position().x, min.x);
      min.y = std::min((*fi)->vert(v)->position().y, min.y);
      min.z = std::min((*fi)->vert(v)->position().z, min.z);
    }

    faces.push_back(*fi);
    sorted.by_x.push_back(*fi);
    sorted.by_y.push_back(*fi);
    sorted.by_z.push_back(*fi);
  }

  std::sort(sorted.by_x.begin(), sorted.by_x.end(), CompByAxis { X_AXIS });
  std::sort(sorted.by_y.begin(), sorted.by_y.end(), CompByAxis { Y_AXIS });
  std::sort(sorted.by_z.begin(), sorted.by_z.end(), CompByAxis { Z_AXIS });

  BBox bbox(min - glm::vec3(EPSILON), max + glm::vec3(EPSILON));

  construct(sorted, bbox);
}

std::unordered_set<const Face*> KDTree::collect_possible_faces(const Ray &ray,
    const glm::mat4 &modelmat) const
{
  glm::mat4 inv_modelmat = glm::inverse(modelmat);
  Ray inv_ray(apply_homog(inv_modelmat, ray.origin(), VEC3_POINT),
      apply_homog(inv_modelmat, ray.direction(), VEC3_DIR));
  std::unordered_set<const Face*> set;
  add_intersecting(inv_ray, set);
  return set;
}

void KDTree::add_debug_lines(DebugViz &dbviz, const glm::mat4 &modelmat) const {
  if (!_child1 || !_child2) {
    _bbox.add_debug_lines(dbviz, modelmat);
  }

  if (_child1) {
    _child1->add_debug_lines(dbviz, modelmat);
  }

  if (_child2) {
    _child2->add_debug_lines(dbviz, modelmat);
  }
}

void KDTree::add_intersecting(const Ray &ray, std::unordered_set<const Face*> &set) const {
  if (!_bbox.ray_intersects(ray)) {
    return;
  }

  for (unsigned i = 0; i < _faces.size(); ++i) {
    set.insert(_faces[i]);
  }

  if (_child1) {
    _child1->add_intersecting(ray, set);
  }

  if (_child2) {
    _child2->add_intersecting(ray, set);
  }
}

void KDTree::construct(const sorted_data &sorted, const BBox &bbox) {
  _bbox = bbox;

  if (sorted.by_x.size() <= 16) {
    for (unsigned i = 0; i < sorted.by_x.size(); ++i) {
      _faces.push_back(sorted.by_x[i]);
    }
    return;
  }

  float range_x = sorted.by_x.back()->centroid().x - sorted.by_x.front()->centroid().x;
  float range_y = sorted.by_y.back()->centroid().y - sorted.by_y.front()->centroid().y;
  float range_z = sorted.by_z.back()->centroid().z - sorted.by_z.front()->centroid().z;

  float mid1, mid2;
  if (range_x >= range_y && range_x >= range_z) {
    _axis = X_AXIS;
    mid1 = sorted.by_x[sorted.by_x.size() / 2 - 1]->centroid().x;
    mid2 = sorted.by_x[sorted.by_x.size() / 2]->centroid().x;
  } else if (range_y >= range_x && range_y >= range_z) {
    _axis = Y_AXIS;
    mid1 = sorted.by_y[sorted.by_y.size() / 2 - 1]->centroid().y;
    mid2 = sorted.by_y[sorted.by_y.size() / 2]->centroid().y;
  } else {
    _axis = Z_AXIS;
    mid1 = sorted.by_z[sorted.by_z.size() / 2 - 1]->centroid().z;
    mid2 = sorted.by_z[sorted.by_z.size() / 2]->centroid().z;
  }

  _plane = 0.5f * (mid1 + mid2);

  BBox bbox1(_bbox), bbox2(_bbox);
  if (_axis == X_AXIS) {
    bbox1.set_max_x(_plane);
    bbox2.set_min_x(_plane);
  } else if (_axis == Y_AXIS) {
    bbox1.set_max_y(_plane);
    bbox2.set_min_y(_plane);
  } else {
    bbox1.set_max_z(_plane);
    bbox2.set_min_z(_plane);
  }

  if (bbox1.volume() < EPSILON || bbox2.volume() < EPSILON) {
    for (unsigned i = 0; i < sorted.by_x.size(); ++i) {
      _faces.push_back(sorted.by_x[i]);
    }
    return;
  }

  CompByAxis comp = { _axis };
  sorted_data sorted1, sorted2;
  for (unsigned i = 0; i < sorted.by_x.size(); ++i) {
    int splitx = comp.face_split(sorted.by_x[i], _plane);
    int splity = comp.face_split(sorted.by_y[i], _plane);
    int splitz = comp.face_split(sorted.by_z[i], _plane);

    if (splitx == SPLIT_LEFT) {
      sorted1.by_x.push_back(sorted.by_x[i]);
    } else if (splitx == SPLIT_RIGHT) {
      sorted2.by_x.push_back(sorted.by_x[i]);
    } else {
      _faces.push_back(sorted.by_x[i]);
    }

    if (splity == SPLIT_LEFT) {
      sorted1.by_y.push_back(sorted.by_y[i]);
    } else if (splity == SPLIT_RIGHT) {
      sorted2.by_y.push_back(sorted.by_y[i]);
    }

    if (splitz == SPLIT_LEFT) {
      sorted1.by_z.push_back(sorted.by_z[i]);
    } else if (splitz == SPLIT_RIGHT) {
      sorted2.by_z.push_back(sorted.by_z[i]);
    }
  }

  _child1 = new KDTree();
  _child2 = new KDTree();

  _child1->construct(sorted1, bbox1);
  _child2->construct(sorted2, bbox2);
}

void KDTree::copy(const KDTree &other) {
  _bbox = other._bbox;
  _child1 = new KDTree(*other._child1);
  _child2 = new KDTree(*other._child2);
  _axis = other._axis;
  _plane = other._plane;
  _faces = other._faces;
}

void KDTree::move(KDTree &&other) {
  _bbox = other._bbox;
  _child1 = other._child1;
  _child2 = other._child2;
  _axis = other._axis;
  _plane = other._plane;
  _faces = std::move(other._faces);

  other._child1 = other._child2 = NULL;
}

void KDTree::destroy() {
  if (_child1) {
    delete _child1;
  }

  if (_child2) {
    delete _child2;
  }

  _child1 = _child2 = NULL;

  _faces.clear();
}
