#ifndef PRIMITIVE_H_
#define PRIMITIVE_H_

#include <glm/glm.hpp>

#include "mesh.h"
#include "raytracing.h"

class Primitive {
  public:
    virtual ~Primitive() {}

    virtual void draw() = 0;
    virtual bool intersect(RayHit &rayhit) const = 0;
    virtual void set_viewmat(const glm::mat4 &viewmat) = 0;
    virtual void set_projmat(const glm::mat4 &projmat) = 0;
    virtual void set_mtl(Material::mtl_id id) = 0;
};

class Sphere : public Primitive {
  public:
    Sphere(const glm::vec3 &center, float radius);

    void draw() {
      _mesh_instance.draw();
    }

    bool intersect(RayHit &rayhit) const {
      if (rayhit.intersect_sphere(_center, _radius)) {
        rayhit.set_mesh_instance(&_mesh_instance);
        return true;
      } else {
        return false;
      }
    }

    void set_viewmat(const glm::mat4 &viewmat) {
      _mesh_instance.set_viewmat(viewmat);
    }

    void set_projmat(const glm::mat4 &projmat) {
      _mesh_instance.set_projmat(projmat);
    }

    void set_center(const glm::vec3 &center) {
      _center = center;
      _mesh_instance.set_translate(center);
    }

    void set_radius(float radius) {
      _radius = radius;
      _mesh_instance.set_scale(glm::vec3(radius));
    }

    void set_mtl(Material::mtl_id id) {
      _mesh_instance.set_mtl(id);
    }

  private:
    float _radius;
    glm::vec3 _center;
    MeshInstance _mesh_instance;
};

#endif /* PRIMITIVE_H_ */
