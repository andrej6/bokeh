#include "material.h"

#include <fstream>
#include <string>
#include <unordered_map>

#include <cassert>
#include <cstdlib>

#include "raytracing.h"
#include "util.h"

typedef std::unordered_map<Material::mtl_id, Material*> mtl_map_t;
typedef std::unordered_map<std::string, Material::mtl_id> mtl_name_map_t;

const Material::mtl_id Material::NONE = 0;
static Material::mtl_id next_mtl_id = 1;

static struct MtlManager {
  ~MtlManager() {
    for (mtl_map_t::iterator itr = mtls.begin(); itr != mtls.end(); ++itr) {
      delete itr->second;
    }
  }

  mtl_name_map_t mtl_names;
  mtl_map_t mtls;
} mtl_manager;

std::vector<Material::mtl_id> add_materials_from_mtl(const char *filename) {
  std::ifstream mtlfile(filename);
  assert(mtlfile.good());

  std::vector<Material*> mtls;
  std::vector<std::string> mtl_names;

  Material *mtl = NULL;
  std::string mtl_name;
  std::string line;

  while (std::getline(mtlfile, line)) {
    line = strip(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    auto tokens = split(line);

    if (tokens[0] == "newmtl") {
      if (tokens.size() != 2) {
        glerr() << "ERROR: too many arguments to 'newmtl' directive" << std::endl;
        exit(-1);
      }

      if (mtl) {
        mtls.push_back(mtl);
        mtl_names.push_back(mtl_name);
      }

      mtl = new Material;
      mtl_name = tokens[1];
    } else {
      if (!mtl) {
        glerr() << "ERROR: material properties listed before material name" << std::endl;
        exit(-1);
      }

      if (tokens[0] == "Ka") {            // ambient color
        mtl->set_ambient(parse_vec3(tokens, 1));
      } else if (tokens[0] == "Kd") {     // diffuse color
        mtl->set_diffuse(parse_vec3(tokens, 1));
      } else if (tokens[0] == "Ks") {     // specular color
        mtl->set_specular(parse_vec3(tokens, 1));
      } else if (tokens[0] == "Ns") {     // shininess exponent
        mtl->set_shiny(parse_float(tokens, 1));
      } else if (tokens[0] == "Ke") {     // emitted color
        mtl->set_emitted(parse_vec3(tokens, 1));
      } else if (tokens[0] == "Ne") {     // emittance power
        mtl->set_emittance_power(parse_float(tokens, 1));
      } else if (tokens[0] == "illum") {  // illumination mode
        int mode = parse_int(tokens, 1);
        switch (mode) {
          case 0:           // No ambient, no reflection, no refraction
            mtl->set_illum_mode(0);
            break;

          case 1:           // Ambient, no reflection, no refraction
            mtl->set_illum_mode(ILLUM_AMBIENT);
            break;

          case 3:           // Ambient, reflection, no refraction
            mtl->set_illum_mode(ILLUM_AMBIENT | ILLUM_REFLECT);
            break;

          case 6:           // Ambient, no reflection, refraction
            mtl->set_illum_mode(ILLUM_AMBIENT | ILLUM_REFRACT);
            break;

          default:
            glerr() << "ERROR: unsupported illumination mode in MTL" << std::endl;
            exit(-1);
        }
      } else {                            // unsupported property
        glerr() << "ERROR: unsupported material property '" << tokens[0] << "' in MTL" << std::endl;
        exit(-1);
      }
    }
  }

  mtls.push_back(mtl);
  mtl_names.push_back(mtl_name);

  std::vector<Material::mtl_id> ids;
  for (unsigned i = 0; i < mtls.size(); ++i) {
    ids.push_back(next_mtl_id);
    mtl_manager.mtl_names.insert(std::make_pair(mtl_names[i], next_mtl_id));
    mtl_manager.mtls.insert(std::make_pair(next_mtl_id, mtls[i]));
    ++next_mtl_id;
  }

  return ids;
}

glm::vec3 Material::shade(
    const RayHit &incoming,
    const RayHit &lightray) const
{
  glm::vec3 norm = incoming.norm();
  glm::vec3 eye = -incoming.ray().direction();
  glm::vec3 light = lightray.ray().direction();
  glm::vec3 light_color = lightray.material()->emitted();

  glm::vec3 color = _emitted;

  float dot_nl = std::max(glm::dot(norm, light), 0.0f);
  color += light_color * _diffuse * dot_nl;

  glm::vec3 reflect = glm::normalize(2*dot_nl * norm - light);
  float dot_er = std::max(glm::dot(eye, reflect), 0.0f);
  color += light_color * specular() * std::pow(dot_er, shiny()) * dot_nl;

  return color;
}

Material::mtl_id get_mtl_id(const char *name) {
  mtl_name_map_t::iterator itr = mtl_manager.mtl_names.find(name);
  if (itr == mtl_manager.mtl_names.end()) {
    return Material::NONE;
  } else {
    return itr->second;
  }
}

const Material *get_mtl(const char *name) {
  Material::mtl_id id = get_mtl_id(name);
  if (id == Material::NONE) {
    return NULL;
  } else {
    return mtl_manager.mtls.find(id)->second;
  }
}

const Material *get_mtl(Material::mtl_id id) {
  mtl_map_t::iterator itr = mtl_manager.mtls.find(id);
  if (itr == mtl_manager.mtls.end()) {
    return NULL;
  } else {
    return itr->second;
  }
}
