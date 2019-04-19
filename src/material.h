#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <vector>

#include <glm/glm.hpp>

#define ILLUM_AMBIENT 0x1
#define ILLUM_REFLECT 0x2
#define ILLUM_REFRACT 0x4

class RayHit;

class Material {
  public:
    typedef size_t mtl_id;
    static const mtl_id NONE;

    Material() : _illum_modes(0) {}

    const glm::vec3 &diffuse() const { return _diffuse; }
    const glm::vec3 &ambient() const { return _ambient; }
    glm::vec3 specular() const { return glm::vec3(_specular.r, _specular.g, _specular.b); }
    float shiny() const { return _specular.a; }
    glm::vec3 emitted() const { return glm::vec3(_emitted.r, _emitted.g, _emitted.b); }
    float emittance_power() const { return _emitted.a; }

    void set_diffuse(const glm::vec3 &diffuse) {
      _diffuse = diffuse;
    }

    void set_ambient(const glm::vec3 &ambient) {
      _ambient = ambient;
    }

    void set_specular(const glm::vec3 &specular) {
      _specular = glm::vec4(specular.r, specular.g, specular.b, _specular.a);
    }

    void set_shiny(float shiny) {
      _specular.a = shiny;
    }

    void set_emitted(const glm::vec3 &emitted) {
      _emitted = glm::vec4(emitted.r, emitted.g, emitted.b, _emitted.a);
    }

    void set_emittance_power(float emittance_power) {
      _emitted.a = emittance_power;
    }

    bool ambient_on() const { return _illum_modes & ILLUM_AMBIENT; }
    bool reflect_on() const { return _illum_modes & ILLUM_REFLECT; }
    bool refract_on() const { return _illum_modes & ILLUM_REFRACT; }

    void set_ambient_on(bool on) {
      if (on) {
        _illum_modes |= ILLUM_AMBIENT;
      } else {
        _illum_modes &= ~ILLUM_AMBIENT;
      }
    }

    void set_reflect_on(bool on) {
      if (on) {
        _illum_modes |= ILLUM_REFLECT;
      } else {
        _illum_modes &= ~ILLUM_REFLECT;
      }
    }

    void set_refract_on(bool on) {
      if (on) {
        _illum_modes |= ILLUM_REFRACT;
      } else {
        _illum_modes &= ~ILLUM_REFRACT;
      }
    }

    void set_illum_mode(int mode) { _illum_modes = mode; }

    glm::vec3 shade(const RayHit &incoming, const RayHit &lightray) const;

  private:
    glm::vec3 _diffuse;
    glm::vec3 _ambient;
    glm::vec4 _specular; // 4th component is shininess exponent
    glm::vec4 _emitted;  // 4th component is emittance power
    int _illum_modes;
};

std::vector<Material::mtl_id> add_materials_from_mtl(const char *mtl_filename);
Material::mtl_id get_mtl_id(const char *name);
const Material *get_mtl(const char *name);
const Material *get_mtl(Material::mtl_id id);

#endif /* MATERIAL_H_ */
