#ifndef SCENE_H_
#define SCENE_H_

#include <vector>

#include <glm/glm.hpp>

#include "camera.h"
#include "mesh.h"
#include "raytracing.h"
#include "kd_tree.h"

class Scene {
  public:
    Scene(Scene &&other) {
      _mesh_instances = std::move(other._mesh_instances);
      _kd_tree = std::move(other._kd_tree);
      _lights = std::move(other._lights);
      _raytree = std::move(other._raytree);
      _camera = other._camera;
      other._camera = NULL;
      _draw_kdtree = other._draw_kdtree;
    }

    Scene &operator=(Scene &&other) {
      _mesh_instances = std::move(other._mesh_instances);
      _kd_tree = std::move(other._kd_tree);
      _lights = std::move(other._lights);
      _raytree = std::move(other._raytree);
      _camera = other._camera;
      other._camera = NULL;
      _draw_kdtree = other._draw_kdtree;
      return *this;
    }

    ~Scene() {
      if (_camera) {
        delete _camera;
      }
    }

    static Scene from_scn(const char *filename);

    const glm::vec3 bg_color() const { return _bg_color; }

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
    Scene() : _camera(NULL) {}
    glm::vec3 trace_ray(const Ray &ray, RayTreeNode *treenode, int level, int type) const;

    std::vector<MeshInstance> _mesh_instances;
    KDTree _kd_tree;
    DebugViz _dbviz;
    std::vector<size_t> _lights;
    RayTree _raytree;
    Camera *_camera;
    glm::vec3 _bg_color;
    bool _draw_kdtree;
};

#endif /* SCENE_H_ */
