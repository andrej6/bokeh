#include "lens_assembly.h"

#include <fstream>
#include <string>

#include <cmath>

#include "util.h"

LensAssembly *LensAssembly::from_la(const char *filename) {
  std::string dirs = dirname(filename);

  std::ifstream scnfile(filename);
  assert(scnfile.good());

  LensAssembly *lens_assembly = new LensAssembly();
  std::string line;

  float z = 0.0;
  glm::vec3 c(0, 0, 0);
  float r, t, n, a;

  while (std::getline(scnfile, line)) {
    line = strip(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    auto tokens = split(line);

    if (tokens[0] == "lens_assembly") { // ======== LENS ASSEMBLY ==================
      if (tokens.size() != 2) {
        glerr() << "ERROR: incorrect number of arguments for lens_assembly" << std::endl;
        exit(-1);
      }

      lens_assembly->set_dist(parse_float(tokens, 1));

    } else if (tokens[0] == "lens_surface") { // ======== LENS SURFACE ==================
      if (tokens.size() != 5) {
        glerr() << "ERROR: incorrect number of arguments for lens_surface" << std::endl;
        exit(-1);
      }

      r = parse_float(tokens, 1);
      t = parse_float(tokens, 2);
      n = parse_float(tokens, 3);
      a = parse_float(tokens, 4);

      c.x = z + r;
      z -= t;

      lens_assembly->add_surface(LensSurface(c, abs(r), n, a / 2.0));

    }
  }
}

void LensAssembly::find_pupil() {
  if (_surfaces.empty()) {
    _exit_pupil_pos = 0.0;
    _exit_pupil_rad = 0.0;
    return;
  }

  float max_ratio = 0.0;
  unsigned aperture_index = _surfaces.size();
  float marginal_angle;
  float marginal_height;

  float index = 1.0;
  float height = 0.1;
  float angle = 0.1;

  // Find the aperture stop surface
  for (unsigned i = _surfaces.size() - 1; i > 0; --i) {
    angle = _surfaces[i].paraxial_refract(angle, height, index);

    float ratio = fabs(height / _surfaces[i].aperture_radius());
    if (ratio > max_ratio) {
      max_ratio = ratio;
      aperture_index = i;
      marginal_angle = angle / ratio;
      marginal_height = height / ratio;
    }

    float dist = _surfaces[i-1].vertex_position() - _surfaces[i].vertex_position();
    height = height + angle*dist;
    index = _surfaces[i].index_of_refraction();
  }

  index = _surfaces[aperture_index].index_of_refraction();
  angle = 0.1;
  height = 0.0;

  // Find location of exit pupil
  for (unsigned _i = aperture_index; _i > 0; --_i) {
    unsigned i = _i - 1;

    float dist = _surfaces[i].vertex_position() - _surfaces[i+1].vertex_position();
    height = height + angle*dist;
    angle = _surfaces[i].paraxial_refract(angle, height, index);

    marginal_height = marginal_height + marginal_angle*dist;
    marginal_angle = _surfaces[i].paraxial_refract(marginal_angle, height, index);

    index = _surfaces[i].index_of_refraction();
  }

  float dz = -height / angle;
  _exit_pupil_pos = _surfaces.back().vertex_position() + dz;
  _exit_pupil_rad = marginal_angle*dz + marginal_height;
}

Ray LensAssembly::generate_ray(float x, float y) const {
  glm::vec3 origin(x, y, _surfaces[0].vertex_position() + _dist);

  float theta = 2*PI*randf(), r = sqrt(randf())*_exit_pupil_rad;
  glm::vec3 pupil_pt(r*cos(theta), r*sin(theta), _exit_pupil_pos);

  RayHit rayhit(origin, pupil_pt - origin);
}
