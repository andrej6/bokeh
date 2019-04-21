#ifndef KD_TREE_H_
#define KD_TREE_H_

#include <unordered_set>
#include <vector>

#include <cmath>

#include <glm/glm.hpp>

#include "debug_viz.h"

class Ray;
class Mesh;
class Face;

class BBox {
  public:
    BBox() = default;
    BBox(const glm::vec3 &min, const glm::vec3 &max) : _min(min), _max(max) {
      assert(min.x <= max.x);
      assert(min.y <= max.y);
      assert(min.z <= max.z);
    }

    const glm::vec3 &min() const { return _min; }
    const glm::vec3 &max() const { return _max; }

    void set_min_x(float x) { _min.x = x; }
    void set_min_y(float y) { _min.y = y; }
    void set_min_z(float z) { _min.z = z; }

    void set_max_x(float x) { _max.x = x; }
    void set_max_y(float y) { _max.y = y; }
    void set_max_z(float z) { _max.z = z; }

    float x_range() const { return _max.x - _min.x; }
    float y_range() const { return _max.y - _min.y; }
    float z_range() const { return _max.z - _min.z; }

    bool ray_intersects(const Ray &ray) const;

    float volume() const { return x_range() * y_range() * z_range(); }

    void add_debug_lines(DebugViz &dbviz, const glm::mat4 &modelmat = glm::mat4(1.0)) const;

  private:
    glm::vec3 _min;
    glm::vec3 _max;
};

class KDTree {
  public:
    KDTree() : _child1(NULL), _child2(NULL) {}
    KDTree(const KDTree &other) { copy(other); }
    KDTree(KDTree &&other) { move(std::move(other)); }

    KDTree &operator=(const KDTree &other) { destroy(); copy(other); return *this; }
    KDTree &operator=(KDTree &&other) { destroy(); move(std::move(other)); return *this; }

    ~KDTree() { destroy(); }

    KDTree(const Mesh *mesh);

    std::unordered_set<const Face*> collect_possible_faces(const Ray &ray,
        const glm::mat4 &modelmat = glm::mat4(1.0)) const;

    void add_debug_lines(DebugViz &dbviz, const glm::mat4 &modelmat) const;

  private:
    void copy(const KDTree&);
    void move(KDTree&&);
    void destroy();

    struct sorted_data {
      std::vector<const Face*> by_x;
      std::vector<const Face*> by_y;
      std::vector<const Face*> by_z;
    };

    void construct(const sorted_data &sorted, const BBox &bbox);
    void add_intersecting(const Ray &ray, std::unordered_set<const Face*> &set) const;

    BBox _bbox;
    KDTree *_child1;
    KDTree *_child2;

    int _axis;
    float _plane;

    std::vector<const Face*> _faces;

    friend struct CompByAxis;
};

/*
template <>
struct std::hash<KDTree::entry> {
  size_t operator()(const KDTree::entry &ent) const {
    return (std::hash<const Face*>()(ent.face) << 1)
      + std::hash<const MeshInstance*>()(ent.mesh_instance);
  }
};
*/

#endif /* KD_TREE_H_ */
