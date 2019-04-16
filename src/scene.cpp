#include "scene.h"

#include <fstream>
#include <string>

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
      if (tokens.size() != 3) {
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
    } else if (tokens[0] == "mesh_instance") {
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
}

void Scene::trace_ray(double x, double y, const RayTreePtr &treeptr) const {
  double norm_x = x / Canvas::width();
  double norm_y = 1.0 - (y / Canvas::height());
  trace_ray(_camera->cast_ray(norm_x, norm_y), treeptr);
}

void Scene::trace_ray(const Ray &ray, const RayTreePtr &treeptr) const {
  RayHit rayhit(ray);

  for (unsigned i = 0; i < _mesh_instances.size(); ++i) {
    rayhit.intersect_mesh(_mesh_instances[i]);
  }

  treeptr.add_child(rayhit, glm::vec3(0.0, 0.0, 1.0));
}

void Scene::visualize_raytree(double x, double y) {
  _raytree.clear();
  trace_ray(x, y, _raytree.root());
}
