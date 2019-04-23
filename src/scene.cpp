#include "scene.h"

#include <fstream>
#include <string>
#include <unordered_set>
#include <algorithm>

#include <cstdlib>

#include "material.h"
#include "util.h"

static std::string concat_path(const std::string &dirname, const std::string &basename) {
  return dirname + std::string("/") + basename;
}

Scene Scene::from_scn(const char *filename) {
  std::string dirs = dirname(filename);

  std::ifstream scnfile(filename);
  assert(scnfile.good());

  Scene scene;
  std::string line;

  while (std::getline(scnfile, line)) {
    line = strip(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    auto tokens = split(line);

    if (tokens[0] == "mesh") {
      if (tokens.size() != 3) {
        glerr() << "ERROR: incorrect number of arguments for new mesh in SCN" << std::endl;
        exit(-1);
      }

      add_mesh_from_obj(tokens[1].c_str(), concat_path(dirs, tokens[2]).c_str());
    } else if (tokens[0] == "materials") {
      if (tokens.size() != 2) {
        glerr() << "ERROR: incorrect number of arguments for new MTL file in SCN" << std::endl;
        exit(-1);
      }

      add_materials_from_mtl(concat_path(dirs, tokens[1]).c_str());
    } else if (tokens[0] == "bgc") {
      scene._bg_color = parse_vec3(tokens, 1);
      if (tokens.size() != 4) {
        glerr() << "ERROR: too many parameters to bgc definition in SCN" << std::endl;
        exit(-1);
      }
    } else if (tokens[0] == "camera") {
      if (tokens.size() > 4 || tokens.size() < 3) {
        glerr() << "ERROR: incorrect number of parameters for camera specification in SCN" << std::endl;
        exit(-1);
      }

      if (scene._camera) {
        glerr() << "ERROR: multiple camera specifications in SCN" << std::endl;
        exit(-1);
      }

      float size_angle = parse_float(tokens, 2);
      if (tokens[1] == "orthographic") {
        OrthographicCamera *c = new OrthographicCamera;
        c->set_size(size_angle);
        scene._camera = c;
      } else if (tokens[1] == "perspective") {
        PerspectiveCamera *c = new PerspectiveCamera;
        c->set_angle(size_angle);
        scene._camera = c;
      } else if (tokens[1] == "lens") {
        LensAssembly *la = new LensAssembly(LensAssembly::from_la(concat_path(dirs, tokens[3]).c_str()));
        LensCamera *c = new LensCamera;
        c->set_angle(size_angle);
        c->set_lens_assembly(la);
        scene._camera = c;
      }
    } else if (tokens[0] == "cam_position") {
      if (!scene._camera) {
        glerr() << "ERROR: setting camera position before camera specification" << std::endl;
        exit(-1);
      }

      scene._camera->set_position(parse_vec3(tokens, 1));
      if (tokens.size() > 4) {
        glerr() << "ERROR: too many parameters to cam_position" << std::endl;
        exit(-1);
      }
    } else if (tokens[0] == "cam_poi") {
      if (!scene._camera) {
        glerr() << "ERROR: setting camera point of interest before camera specification" << std::endl;
        exit(-1);
      }

      scene._camera->set_point_of_interest(parse_vec3(tokens, 1));
      if (tokens.size() > 4) {
        glerr() << "ERROR: too many parameters to cam_poi" << std::endl;
        exit(-1);
      }
    } else if (tokens[0] == "cam_up") {
      if (!scene._camera) {
        glerr() << "ERROR: setting camera up vector before camera specification" << std::endl;
        exit(-1);
      }

      scene._camera->set_up(parse_vec3(tokens, 1));
      if (tokens.size() > 4) {
        glerr() << "ERROR: too many parameters to cam_up" << std::endl;
      }
    }  else if (tokens[0] == "mesh_instance") {
      if (tokens.size() != 2) {
        glerr() << "ERROR: incorrect number of parameters for mesh_instance" << std::endl;
        exit(-1);
      }

      if (!scene._mesh_instances.empty()) {
        const Material *mtl = scene._mesh_instances.back().material();
        if (glm::length(mtl->emitted()) > EPSILON) {
          scene._lights.push_back(scene._mesh_instances.size() - 1);
        }
      }

      scene._mesh_instances.push_back(MeshInstance(get_mesh_id(tokens[1].c_str())));
    } else if (tokens[0] == "mtl"
            || tokens[0] == "scale"
            || tokens[0] == "rotate"
            || tokens[0] == "translate"
            || tokens[0] == "scale+"
            || tokens[0] == "rotate+"
            || tokens[0] == "translate+")
    {
      if (scene._mesh_instances.empty()) {
        glerr() << "ERROR: setting mesh instance properties without a mesh instance" << std::endl;
        exit(-1);
      }

      MeshInstance &mi = scene._mesh_instances.back();

      if (tokens[0] == "mtl") {
        if (tokens.size() != 2) {
          glerr() << "ERROR: incorrect number of parameters for mtl" << std::endl;
          exit(-1);
        }

        mi.set_mtl(get_mtl_id(tokens[1].c_str()));
      } else if (strip(tokens[0], "+") == "scale") {
        glm::vec3 scalevec = parse_vec3(tokens, 1);
        if (tokens[0].back() == '+') {
          mi.scale(scalevec);
        } else {
          mi.set_scale(scalevec);
        }
        if (tokens.size() > 4) {
          glerr() << "ERROR: too many parameters for scale" << std::endl;
          exit(-1);
        }
      } else if (strip(tokens[0], "+") == "rotate") {
        glm::vec3 axis = parse_vec3(tokens, 1);
        float angle = deg_to_rad(parse_float(tokens, 4));
        if (tokens[0].back() == '+') {
          mi.rotate(angle, axis);
        } else {
          mi.set_rotate(angle, axis);
        }
        if (tokens.size() > 5) {
          glerr() << "ERROR: too many parameters for rotate" << std::endl;
          exit(-1);
        }
      } else {
        glm::vec3 translatevec = parse_vec3(tokens, 1);
        if (tokens[0].back() == '+') {
          mi.translate(translatevec);
        } else {
          mi.set_translate(translatevec);
        }
        if (tokens.size() > 4) {
          glerr() << "ERROR: too many parameters for translate" << std::endl;
          exit(-1);
        }
      }
    } else {
      glerr() << "ERROR: unrecognized directive '" << tokens[0] << "' in SCN" << std::endl;
      exit(-1);
    }
  }

  return scene;
}

void Scene::draw() {
  glm::mat4 view, proj;
  _camera->get_view_projection(view, proj);

  for (unsigned i = 0; i < _mesh_instances.size(); ++i) {
    _mesh_instances[i].set_viewmat(view);
    _mesh_instances[i].set_projmat(proj);
    _mesh_instances[i].draw();
  }

  _raytree.set_viewmat(view);
  _raytree.set_projmat(proj);
  _raytree.draw();

  if (_draw_kdtree) {
    for (unsigned i = 0; i < _mesh_instances.size(); ++i) {
      _mesh_instances[i].draw_kd_tree();
    }
  }
}

#define RAY_TYPE_ROOT     0
#define RAY_TYPE_REFLECT  1

glm::vec3 Scene::trace_ray(double x, double y, RayTreeNode *treenode, int bounces) const {
  double center_x = x + 0.5;
  double center_y = y + 0.5;

  if (_lens_samples <= 1) {
    double norm_x = center_x / Canvas::width();
    double norm_y = center_y / Canvas::height();
    return trace_ray(_camera->cast_ray(norm_x, norm_y), treenode, bounces + 1, RAY_TYPE_ROOT);
  }

  glm::vec3 color(0.0);

  for (unsigned i = 0; i < _lens_samples; ++i) {
    double rand_x = randf() - 0.5, rand_y = randf() - 0.5;
    double norm_x = (center_x + rand_x) / Canvas::width();
    double norm_y = (center_y + rand_y) / Canvas::height();
    color += trace_ray(_camera->cast_ray(norm_x, norm_y), treenode, bounces + 1, RAY_TYPE_ROOT);
  }

  return color / float(_lens_samples);
}

glm::vec3 Scene::trace_ray(const Ray &ray, RayTreeNode *treenode, int level, int type) const {
  if (level <= 0) {
    return glm::vec3(0,0,0);
  }

  RayHit rayhit(ray);

  for (unsigned i = 0; i < _mesh_instances.size(); ++i) {
    rayhit.intersect_mesh(_mesh_instances[i]);
  }

  glm::vec3 raytree_color;
  if (type == RAY_TYPE_ROOT) {
    raytree_color = glm::vec3(0, 0, 1);
  } else {
    raytree_color = glm::vec3(1, 0, 0);
  }

  if (treenode) {
    treenode->add_child(rayhit, raytree_color);
  }

  if (!rayhit.intersected()) {
    return _bg_color;
  }

  glm::vec3 color;
  const Material *mtl = rayhit.material();
  if (mtl) {
    if (mtl->emittance_power() > 0.0) {
      return glm::vec3(1.0, 1.0, 1.0);
    } else {
      color += mtl->ambient();
    }
  }

  for (unsigned i = 0; i < _lights.size(); ++i) {
    const MeshInstance *mi = &_mesh_instances[_lights[i]];
    const Mesh *m = mi->mesh();
    //const Material *light_mtl = mi->material();
    glm::mat4 modelmat = mi->modelmat();
    glm::vec3 lightcolor;

    for (unsigned j = 0; j < _shadow_samples; ++j) {
      const Face *f = m->face(randi() % m->faces_size());
      glm::vec3 facepoint = f->random_point_transformed(modelmat);
      glm::vec3 origin(rayhit.intersection_point() + EPSILON*rayhit.norm());

      RayHit lightray(origin, facepoint-origin);
      lightray.intersect_mesh(*mi);
      float light_t = lightray.t();

      for (unsigned m = 0; m < _mesh_instances.size(); ++m) {
        lightray.intersect_mesh(_mesh_instances[m]);
      }

      if (treenode) {
        treenode->add_child(lightray, glm::vec3(0, 1, 0));
      }

      if (lightray.t() < light_t) {
        continue;
      }

      lightcolor += mtl->shade(rayhit, lightray);
    }

    color += lightcolor / float(_shadow_samples);
  }

  if (mtl->reflect_on()) {
    glm::vec3 n = rayhit.norm();
    glm::vec3 origin = rayhit.intersection_point() + EPSILON*n;
    glm::vec3 incident = rayhit.ray().direction();
    glm::vec3 reflected = incident - 2.0f*glm::dot(incident, n)*n;

    Ray reflect(origin, reflected);
    color += mtl->specular() * trace_ray(reflect, treenode, level-1, RAY_TYPE_REFLECT);
  }

  color.r = std::min(color.r, 1.0f);
  color.g = std::min(color.g, 1.0f);
  color.b = std::min(color.b, 1.0f);
  return color;
}

void Scene::visualize_raytree(double x, double y) {
  _raytree.clear();
  trace_ray(x, y, &_raytree.root(), _ray_bounces);
}
