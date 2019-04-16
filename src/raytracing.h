#ifndef RAYTRACING_H_
#define RAYTRACING_H_

#include <vector>

#include <cmath>

#include <glm/glm.hpp>

#include "debug_viz.h"
#include "mesh.h"
#include "image.h"

class Face;

class Ray {
  public:
    Ray(const glm::vec3 &origin, const glm::vec3 &direction) :
      _origin(origin), _direction(glm::normalize(direction)) {}

    Ray(const Ray&) = default;
    Ray(Ray&&) = default;
    Ray &operator=(const Ray&) = default;
    Ray &operator=(Ray&&) = default;

    const glm::vec3 &origin() const { return _origin; }
    const glm::vec3 &direction() const { return _direction; }
    const glm::vec3 point_at(float t) const {
      return _origin + t*_direction;
    }

  private:
    glm::vec3 _origin;
    glm::vec3 _direction;
};

class RayHit {
  public:
    RayHit(const glm::vec3 &origin, const glm::vec3 &direction) :
      _t(NAN), _ray(origin, direction), _face(NULL) {}

    RayHit(const Ray &ray) : _t(NAN), _ray(ray), _face(NULL) {}

    RayHit(const RayHit&) = default;
    RayHit(RayHit&&) = default;
    RayHit &operator=(const RayHit&) = default;
    RayHit &operator=(RayHit&&) = default;

    bool intersect_face(const Face &face, const glm::mat4 &modelmat = glm::mat4(1.0));
    bool intersect_mesh(const MeshInstance &mesh);

    bool intersected() const { return !std::isnan(_t); }
    glm::vec3 intersection_point() const { return _ray.point_at(_t); }

    float t() const { return _t; }
    const Ray &ray() const { return _ray; }
    Face *face() const { return _face; }

    void set_t(float t) { _t = t; }
    void set_face(Face *face) { _face = face; }

  private:
    float _t;
    Ray _ray;
    Face *_face;
    glm::mat4 _modelmat;
};

class RayTree;

class RayTreeNode {
  private:
    RayTreeNode() : _ray(glm::vec3(0.0), glm::vec3(0.0)), _color(0.0) {}
    RayTreeNode(const RayTreeNode&) = default;
    RayTreeNode(RayTreeNode&&) = default;
    RayTreeNode &operator=(const RayTreeNode&) = default;
    RayTreeNode &operator=(RayTreeNode&&) = default;

    ~RayTreeNode();

    RayTreeNode(const RayHit &ray, const glm::vec3 color) : _ray(ray), _color(color) {}

    RayHit _ray;
    glm::vec3 _color;
    std::vector<RayTreeNode*> _children;

    friend class RayTreePtr;
    friend class RayTree;
};

class RayTreePtr {
  public:
    typedef std::vector<RayTreeNode*>::iterator iterator;
    iterator children_begin() const { return _node->_children.begin(); }
    iterator children_end() const { return _node->_children.end(); }

    size_t children_size() const { return _node->_children.size(); }

    void add_child(const RayHit &ray, const glm::vec3 &color) const;

  private:
    RayTreePtr(RayTree *tree, RayTreeNode *node) :
      _tree(tree), _node(node) {}

    RayTree *_tree;
    RayTreeNode *_node;

    friend class RayTree;
};

class RayTree {
  public:
    RayTree() = default;
    RayTree(const RayTree&) = delete;
    RayTree(RayTree&&) = default;

    RayTree &operator=(const RayTree&) = delete;
    RayTree &operator=(RayTree&&) = default;

    RayTreePtr root() const { return RayTreePtr((RayTree*) this, (RayTreeNode*) &_root); }

    void set_viewmat(const glm::mat4 &view) { _dbviz.set_viewmat(view); }
    void set_projmat(const glm::mat4 &proj) { _dbviz.set_projmat(proj); }
    void draw() { _dbviz.draw(); }
    void clear();

  private:
    RayTreeNode _root;
    DebugViz _dbviz;

    friend class RayTreePtr;
};

#endif /* RAYTRACING_H_ */
