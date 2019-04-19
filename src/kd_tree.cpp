#include "kd_tree.h"

#include <algorithm>

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

void BBox::add_debug_lines(DebugViz &dbviz) const {
  glm::vec3 pt000(_min);
  glm::vec3 pt001(_min.x, _min.y, _max.z);
  glm::vec3 pt010(_min.x, _max.y, _min.z);
  glm::vec3 pt011(_min.x, _max.y, _max.z);
  glm::vec3 pt100(_max.x, _min.y, _min.z);
  glm::vec3 pt101(_max.x, _min.y, _max.z);
  glm::vec3 pt110(_max.x, _max.y, _min.z);
  glm::vec3 pt111(_max);

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

struct CompVertEntries {
  int axis;
  typedef KDTree::vert_entry vert_entry;
  bool operator()(const vert_entry *ve1, const vert_entry *ve2) const {
    float d1, d2;
    if (axis == X_AXIS) {
      d1 = ve1->pos.x;
      d2 = ve2->pos.x;
    } else if (axis == Y_AXIS) {
      d1 = ve1->pos.y;
      d2 = ve2->pos.y;
    } else {
      d1 = ve1->pos.z;
      d2 = ve2->pos.z;
    }

    return d1 < d2;
  }

  bool operator()(const vert_entry *ve, float plane) const {
    float d;
    if (axis == X_AXIS) {
      d = ve->pos.x;
    } else if (axis == Y_AXIS) {
      d = ve->pos.y;
    } else {
      d = ve->pos.z;
    }

    return d < plane;
  }
};

KDTree::KDTree(const std::vector<MeshInstance> &meshes) {
  std::vector<entry> entries;
  std::vector<vert_entry> vert_entries;
  sorted_data sorted;

  for (unsigned i = 0; i < meshes.size(); ++i) {
    const MeshInstance *mi = &meshes[i];
    const Mesh *m = mi->mesh();

    for (Mesh::face_iterator fi = m->faces_begin(); fi != m->faces_end(); ++fi) {
      entry ent = { *fi, mi };
      entries.push_back(ent);
    }
  }

  for (unsigned i = 0; i < entries.size(); ++i) {
    const entry *e = &entries[i];
    const Face *f = e->face;
    const MeshInstance *mi = e->mesh_instance;
    glm::mat4 modelmat = mi->modelmat();

    for (unsigned i = 0; i < 3; ++i) {
      vert_entry ve;
      ve.ent = e;
      glm::vec3 pos = f->vert(i)->position();
      glm::vec4 homog(pos.x, pos.y, pos.z, 1.0);
      homog = modelmat * homog;
      ve.pos = glm::vec3(homog.x, homog.y, homog.z) / homog.w;
      vert_entries.push_back(ve);
    }
  }

  for (unsigned i = 0; i < vert_entries.size(); ++i) {
    vert_entry *ve = &vert_entries[i];
    sorted.by_x.push_back(ve);
    sorted.by_y.push_back(ve);
    sorted.by_z.push_back(ve);
  }

  std::sort(sorted.by_x.begin(), sorted.by_x.end(), CompVertEntries { X_AXIS });
  std::sort(sorted.by_y.begin(), sorted.by_y.end(), CompVertEntries { Y_AXIS });
  std::sort(sorted.by_z.begin(), sorted.by_z.end(), CompVertEntries { Z_AXIS });

  glm::vec3 min(
    sorted.by_x.front()->pos.x - EPSILON,
    sorted.by_y.front()->pos.y - EPSILON,
    sorted.by_z.front()->pos.z - EPSILON
  );
  glm::vec3 max(
    sorted.by_x.back()->pos.x + EPSILON,
    sorted.by_y.back()->pos.y + EPSILON,
    sorted.by_z.back()->pos.z + EPSILON
  );

  BBox bbox(min, max);

  construct(sorted, bbox);
}

std::unordered_set<KDTree::entry> KDTree::collect_possible_faces(const Ray &ray) const {
  std::unordered_set<entry> set;
  add_intersecting(ray, set);
  return set;
}

void KDTree::add_debug_lines(DebugViz &dbviz) const {
  if (!_child1 || !_child2) {
    _bbox.add_debug_lines(dbviz);
  }

  if (_child1) {
    _child1->add_debug_lines(dbviz);
  }

  if (_child2) {
    _child2->add_debug_lines(dbviz);
  }
}

void KDTree::add_intersecting(const Ray &ray, std::unordered_set<entry> &set) const {
  if (!_bbox.ray_intersects(ray)) {
    return;
  }

  for (unsigned i = 0; i < _entries.size(); ++i) {
    set.insert(_entries[i]);
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

  if (sorted.by_x.size() <= 8) {
    for (unsigned i = 0; i < sorted.by_x.size(); ++i) {
      _entries.push_back(*(sorted.by_x[i]->ent));
    }
    return;
  }

  float range_x = sorted.by_x.back()->pos.x - sorted.by_x.front()->pos.x;
  float range_y = sorted.by_y.back()->pos.y - sorted.by_y.front()->pos.y;
  float range_z = sorted.by_z.back()->pos.z - sorted.by_z.front()->pos.z;

  float mid1, mid2;
  if (range_x >= range_y && range_x >= range_z) {
    _axis = X_AXIS;
    mid1 = sorted.by_x[sorted.by_x.size() / 2 - 1]->pos.x;
    mid2 = sorted.by_x[sorted.by_x.size() / 2]->pos.x;
  } else if (range_y >= range_x && range_y >= range_z) {
    _axis = Y_AXIS;
    mid1 = sorted.by_y[sorted.by_y.size() / 2 - 1]->pos.y;
    mid2 = sorted.by_y[sorted.by_y.size() / 2]->pos.y;
  } else {
    _axis = Z_AXIS;
    mid1 = sorted.by_z[sorted.by_z.size() / 2 - 1]->pos.z;
    mid2 = sorted.by_z[sorted.by_z.size() / 2]->pos.z;
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
      _entries.push_back(*(sorted.by_x[i]->ent));
    }
    return;
  }

  CompVertEntries comp = { _axis };
  sorted_data sorted1, sorted2;
  for (unsigned i = 0; i < sorted.by_x.size(); ++i) {
    if (comp(sorted.by_x[i], _plane)) {
      sorted1.by_x.push_back(sorted.by_x[i]);
    } else {
      sorted2.by_x.push_back(sorted.by_x[i]);
    }

    if (comp(sorted.by_y[i], _plane)) {
      sorted1.by_y.push_back(sorted.by_y[i]);
    } else {
      sorted2.by_y.push_back(sorted.by_y[i]);
    }

    if (comp(sorted.by_z[i], _plane)) {
      sorted1.by_z.push_back(sorted.by_z[i]);
    } else {
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
  _entries = other._entries;
}

void KDTree::move(KDTree &&other) {
  _bbox = other._bbox;
  _child1 = other._child1;
  _child2 = other._child2;
  _axis = other._axis;
  _plane = other._plane;
  _entries = std::move(other._entries);

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

  _entries.clear();
}
