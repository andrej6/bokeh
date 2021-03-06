#ifndef RAYTRACING_H_
#define RAYTRACING_H_

#include <vector>

#include <cmath>

#include <glm/glm.hpp>

#include "debug_viz.h"
#include "mesh.h"
#include "image.h"
#include "material.h"
#include "threads.h"

class Face;
class Scene;

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
      _t(NAN), _ray(origin, direction), _mesh_instance(NULL), _mtl_id(Material::NONE) {}

    RayHit(const Ray &ray) :
      _t(NAN), _ray(ray), _mesh_instance(NULL), _mtl_id(Material::NONE) {}

    RayHit(const RayHit&) = default;
    RayHit(RayHit&&) = default;
    RayHit &operator=(const RayHit&) = default;
    RayHit &operator=(RayHit&&) = default;

    bool intersect_face(const Face &face, const glm::mat4 &modelmat = glm::mat4(1.0));
    bool intersect_mesh(const MeshInstance &mesh);
    bool intersect_sphere(const glm::vec3 &center, float radius);
    bool intersect_plane(const glm::vec3 &normal, const glm::vec3 &s);

    bool intersected() const { return !std::isnan(_t); }
    glm::vec3 intersection_point() const { return _ray.point_at(_t); }

    const Ray &ray() const { return _ray; }
    float t() const { return _t; }
    const glm::vec3 &norm() const { return _norm; }
    Material::mtl_id material_id() const { return _mtl_id; }
    const Material *material() const;
    const MeshInstance *mesh_instance() const { return _mesh_instance; }

    void set_mesh_instance(const MeshInstance *mi) {
      _mesh_instance = mi;
      _mtl_id = mi->material_id();
    }

  private:
    float _t;
    Ray _ray;
    const MeshInstance *_mesh_instance;
    glm::mat4 _modelmat;
    glm::vec3 _norm;
    Material::mtl_id _mtl_id;
};

class RayTree;

class RayTreeNode {
  public:
    typedef std::vector<RayTreeNode*>::iterator iterator;

    void add_child(const RayHit &hit, const glm::vec3 &color);

    iterator begin() { return _children.begin(); }
    iterator end() { return _children.end(); }

  private:
    RayTreeNode(RayTree *tree) : _ray(glm::vec3(0.0), glm::vec3(0.0)), _color(0.0), _tree(tree) {}
    RayTreeNode(const RayTreeNode&) = default;
    RayTreeNode(RayTreeNode &&other) : _ray(std::move(other._ray)) { move(std::move(other)); }
    RayTreeNode &operator=(const RayTreeNode &other) { destroy(); copy(other); return *this; }
    RayTreeNode &operator=(RayTreeNode &&other) { destroy(); move(std::move(other)); return *this; }

    ~RayTreeNode() { destroy(); }
    void destroy();
    void copy(const RayTreeNode&);
    void move(RayTreeNode&&);

    RayTreeNode(const RayHit &ray, const glm::vec3 color) : _ray(ray), _color(color) {}

    RayHit _ray;
    glm::vec3 _color;
    std::vector<RayTreeNode*> _children;
    RayTree *_tree;

    friend class RayTree;
};

class RayTree {
  public:
    RayTree() : _root(this) {}
    RayTree(const RayTree&) = delete;
    RayTree(RayTree &&other);

    RayTree &operator=(const RayTree&) = delete;
    RayTree &operator=(RayTree &&other) { destroy(); move(std::move(other)); return *this; }

    RayTreeNode &root() { return _root; }

    void set_viewmat(const glm::mat4 &view) { _dbviz.set_viewmat(view); }
    void set_projmat(const glm::mat4 &proj) { _dbviz.set_projmat(proj); }
    void draw() { _dbviz.draw(); }
    void clear();

  private:
    void fix_tree_pointers(RayTreeNode *node);
    RayTreeNode _root;
    DebugViz _dbviz;

    void destroy();
    void move(RayTree&&);

    friend class RayTreeNode;
};

class RayTracing {
  public:
    RayTracing(const Scene *scene, bool progressive = true)
      : _scene(scene), _image(Canvas::width(), Canvas::height()),
      _dirty(true), _tex(0), _fbo(0),
      _trace_x(0), _trace_y(0),
      _section(0), _threaded_raytrace(false)
    {
      set_progressive(progressive);
      _section_lock = create_mutex();
    }

    RayTracing(const Scene *scene, unsigned width, unsigned height, bool progressive = true) :
      _scene(scene), _image(width, height), _dirty(true), _fbo(0),
      _trace_x(0), _trace_y(0),
      _section(0), _threaded_raytrace(false)
    {
      set_progressive(progressive);
      _section_lock = create_mutex();
    }

    ~RayTracing() {
      stop_threaded_raytrace();
      destroy_mutex(_section_lock);
    }

    void draw();
    bool trace_next_pixel();
    void start_threaded_raytrace();
    void stop_threaded_raytrace();

    void reset() {
      _image.clear_to_color(pixel_color(0,0,0,1));
      _trace_x = _trace_y = 0;
      _divs_x = _starting_divs_x;
      _divs_y = _starting_divs_y;
    }

    Image &image() { return _image; }

  private:
    void lazy_init_fbo();
    void pack_data();
    void set_progressive(bool progressive) {
      if (progressive) {
        _starting_divs_y = _divs_y = _image.height() / 20;
        _starting_divs_x = _divs_x = _image.width() / 20;
      } else {
        _starting_divs_y = _divs_y = _image.height();
        _starting_divs_x = _divs_x = _image.width();
      }
    }
    bool increase_divs();

    bool next_section(unsigned &x0, unsigned &y0, unsigned &w, unsigned &h) {
      if (!_threaded_raytrace) {
        return false;
      }

      lock_mutex(_section_lock);
      unsigned sec = _section++;
      unlock_mutex(_section_lock);

      unsigned div_w = ceil((double) _image.width() / _starting_divs_x);
      unsigned div_h = ceil((double) _image.height() / _starting_divs_y);

      unsigned x = sec % _starting_divs_x;
      unsigned y = sec / _starting_divs_x;

      if (y >= _starting_divs_y) {
        return false;
      }

      x0 = x * div_w;
      y0 = y * div_h;
      w = std::min(div_w, _image.width() - x0);
      h = std::min(div_h, _image.height() - y0);

      return true;
    }

    const Scene *_scene;
    Image _image;

    bool _dirty;

    GLuint _tex;
    GLuint _fbo;

    unsigned _starting_divs_x, _starting_divs_y;
    unsigned _divs_x, _divs_y;
    unsigned _trace_x, _trace_y;

    std::vector<thread_id> _threads;
    mutex_t _section_lock;
    unsigned _section;
    bool _threaded_raytrace;
    friend void raytracer_thread(void*);
};

#endif /* RAYTRACING_H_ */
