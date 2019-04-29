#ifndef SCENE_H_
#define SCENE_H_

#include <vector>

#include <glm/glm.hpp>

#include "camera.h"
#include "kd_tree.h"
#include "mesh.h"
#include "primitive.h"
#include "raytracing.h"

class Scene {
  public:
    Scene(Scene &&other) {
      _mesh_instances = std::move(other._mesh_instances);
      _lights = std::move(other._lights);
      _raytree = std::move(other._raytree);
      _camera = other._camera;
      other._camera = NULL;
      _draw_kdtree = other._draw_kdtree;
      _shadow_samples = other._shadow_samples;
      _lens_samples = other._lens_samples;
      _ray_bounces = other._ray_bounces;
      _bg_color = other._bg_color;
    }

    Scene &operator=(Scene &&other) {
      _mesh_instances = std::move(other._mesh_instances);
      _lights = std::move(other._lights);
      _raytree = std::move(other._raytree);
      _camera = other._camera;
      other._camera = NULL;
      _draw_kdtree = other._draw_kdtree;
      _shadow_samples = other._shadow_samples;
      _lens_samples = other._lens_samples;
      _ray_bounces = other._ray_bounces;
      _bg_color = other._bg_color;
      return *this;
    }

    ~Scene() {
      if (_camera) {
        delete _camera;
      }
      for (unsigned i = 0; i < _primitives.size(); ++i) {
        delete _primitives[i];
      }
    }

    static Scene from_scn(const char *filename);

    const glm::vec3 bg_color() const { return _bg_color; }
    unsigned shadow_samples() const { return _shadow_samples; }
    unsigned lens_samples() const { return _lens_samples; }
    unsigned ray_bounces() const { return _ray_bounces; }

    void set_shadow_samples(unsigned n) { _shadow_samples = n; }
    void set_lens_samples(unsigned n) { _lens_samples = n; }
    void set_ray_bounces(unsigned n) { _ray_bounces = n; }

    Camera *camera() { return _camera; }

    glm::vec3 trace_ray(double x, double y, int bounces) const {
      return trace_ray(x, y, NULL, bounces);
    }
    glm::vec3 trace_ray(double x, double y, RayTreeNode *treenode, int bounces) const;
    void visualize_raytree(double x, double y);

    void set_draw_kdtree(bool set) { _draw_kdtree = set; }
    void toggle_draw_kdtree() { _draw_kdtree = !_draw_kdtree; }

    void draw();

  private:
    Scene() : _camera(NULL), _draw_kdtree(false), _shadow_samples(1), _lens_samples(1), _ray_bounces(1) {}
    glm::vec3 trace_ray(const Ray &ray, RayTreeNode *treenode, int level, int type) const;

    std::vector<MeshInstance> _mesh_instances;
    std::vector<Primitive*> _primitives;
    DebugViz _dbviz;
    std::vector<size_t> _lights;
    RayTree _raytree;
    Camera *_camera;
    glm::vec3 _bg_color;
    bool _draw_kdtree;

    unsigned _shadow_samples;
    unsigned _lens_samples;
    unsigned _ray_bounces;
};

#endif /* SCENE_H_ */
