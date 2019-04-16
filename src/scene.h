#ifndef SCENE_H_
#define SCENE_H_

#include <vector>

#include <glm/glm.hpp>

#include "camera.h"
#include "mesh.h"
#include "raytracing.h"

class Scene {
  public:
    Scene(Scene &&other) {
      _mesh_instances = std::move(other._mesh_instances);
      _raytree = std::move(other._raytree);
      _camera = other._camera;
      other._camera = NULL;
    }

    Scene &operator=(Scene &&other) {
      _mesh_instances = std::move(other._mesh_instances);
      _raytree = std::move(other._raytree);
      _camera = other._camera;
      other._camera = NULL;
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

    void trace_ray(double x, double y, const RayTreePtr &treeptr) const;
    void trace_ray(const Ray &ray, const RayTreePtr &treeptr) const;
    void visualize_raytree(double x, double y);

    void draw();

  private:
    Scene() : _camera(NULL) {}
    std::vector<MeshInstance> _mesh_instances;
    std::vector<size_t> _lights;
    RayTree _raytree;
    Camera *_camera;
    glm::vec3 _bg_color;
};

#endif /* SCENE_H_ */
