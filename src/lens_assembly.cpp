#include "lens_assembly.h"

#include <fstream>
#include <string>

#include <cmath>

#include "util.h"

LensAssembly LensAssembly::from_la(const char *filename) {
  std::string dirs = dirname(filename);

  std::ifstream scnfile(filename);
  assert(scnfile.good());

  std::string line;

  float z = 0.0;
  float c = 0.0;
  float r, t, n, a;

  LensAssembly assembly;
  assembly._indices.push_back(1.0);

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

      assembly._dist = parse_float(tokens, 1);

    } else if (tokens[0] == "lens_surface") { // ======== LENS SURFACE ==================
      if (tokens.size() != 5) {
        glerr() << "ERROR: incorrect number of arguments for lens_surface" << std::endl;
        exit(-1);
      }

      r = parse_float(tokens, 1);
      t = parse_float(tokens, 2);
      n = parse_float(tokens, 3);
      a = parse_float(tokens, 4);

      c = z + r;
      z += t;

      assembly._surfaces.push_back(LensSurface(c, r, a / 2.0));
      assembly._indices.push_back(n);
    }
  }

  assembly.find_pupil();
  return assembly;
}

void LensAssembly::reduce(unsigned start, unsigned num,
    float *power, float *p1, float *p2, float *f_front, float *f_rear) const
{
  assert(start < _surfaces.size());
  assert(start + num <= _surfaces.size());

  float n1 = _indices[start];
  float n2 = _indices[start + num];

  const float init_y = 1.0;
  const float init_u = 0.0;

  float final_y, final_u;
  paraxial_raytrace(start, num, init_y, init_u, &final_y, &final_u);

  float phi = -n2 * final_u / init_y;
  float fr = n2 / phi;
  float bfd = -final_y / final_u;
  float p_rear = _surfaces[start + num - 1].vertex() + bfd - fr;

  paraxial_raytrace_rev(start + num - 1, num, init_y, init_u, &final_y, &final_u);

  float ff = -n1 / phi;
  float ffd = -final_y / final_u;
  float p_front = _surfaces[start].vertex() + ffd - ff;

  if (power) {
    *power = phi;
  }

  if (p1) {
    *p1 = p_front;
  }

  if (p2) {
    *p2 = p_rear;
  }

  if (f_front) {
    *f_front = ff;
  }

  if (f_rear) {
    *f_rear = fr;
  }
}

void LensAssembly::paraxial_raytrace(unsigned from, unsigned num, float height, float angle,
    float *final_height, float *final_angle) const
{
  assert(from < _surfaces.size());
  assert(from + num <= _surfaces.size());
  if (!(final_height || final_angle)) {
    return;
  }

  float y = height;
  float u = angle;

  for (unsigned i = from; i < from + num; ++i) {
    u = paraxial_refract(i, y, u);
    if (i < from + num - 1) {
      y = paraxial_transfer(i, y, u);
    }
  }

  if (final_height) {
    *final_height = y;
  }

  if (final_angle) {
    *final_angle = u;
  }
}

void LensAssembly::paraxial_raytrace_rev(unsigned from, unsigned num, float height, float angle,
    float *final_height, float *final_angle) const
{
  assert(from < _surfaces.size());
  assert(from + 1 - num >= 0);
  if (!(final_height || final_angle)) {
    return;
  }

  float y = height;
  float u = angle;

  for (unsigned _i = 0; _i < num; ++_i) {
    unsigned i = from - _i;

    u = paraxial_refract_rev(i, y, u);
    if (_i < num - 1) {
      y = paraxial_transfer_rev(i, y, u);
    }
  }

  if (final_height) {
    *final_height = y;
  }

  if (final_angle) {
    *final_angle = u;
  }
}

void LensAssembly::find_aperture_stop() {
  if (_surfaces.empty()) {
    _aperture = (unsigned) -1;
    return;
  }

  float u = 0.001;
  float y = 1.0;

  float min_ratio = _surfaces.front().aperture_radius() / y;
  unsigned apt_idx = 0;

  for (unsigned i = 0; i < _surfaces.size() - 1; ++i) {
    u = paraxial_refract(i, y, u);
    y = paraxial_transfer(i, y, u);
    float ratio = _surfaces[i+1].aperture_radius() / y;

    if (fabs(ratio) < min_ratio) {
      apt_idx = i + 1;
      min_ratio = fabs(ratio);
    }
  }

  _aperture = apt_idx;
}

void LensAssembly::find_pupil() {
  find_cardinal_points();

  float apt_pos = _surfaces[_aperture].vertex();
  float z = apt_pos - _back_p1;
  float h = _surfaces[_aperture].aperture_radius();

  if (fabs(_back_power) <= EPSILON) {
    _exit_pupil_pos = apt_pos;
    _exit_pupil_rad = h;
    return;
  }

  float ff = -_indices[_aperture+1] / _back_power;
  float fr = _indices.back() / _back_power;

  float m = ff / (ff - z);
  float h_im = m*h;
  float z_im = (1 - m)*fr;

  _exit_pupil_pos = _back_p2 + z_im;
  _exit_pupil_rad = h_im;
}

void LensAssembly::find_cardinal_points() {
  find_aperture_stop();
  if (_aperture > 0) {
    reduce(0, _aperture, &_front_power, &_front_p1, &_front_p2, NULL, NULL);
  } else {
    _front_power = 0.0;
    _front_p1 = _front_p2 = _surfaces.front().vertex();
  }

  if (_aperture < _surfaces.size() - 1) {
    reduce(_aperture + 1, _surfaces.size() - _aperture - 1, &_back_power, &_back_p1, &_back_p2, NULL, NULL);
  } else {
    _back_power = 0.0;
    _back_p1 = _back_p2 = _surfaces.back().vertex();
  }
  reduce(0, _surfaces.size(), &_system_power, &_system_p1, &_system_p2, NULL, NULL);
}

Ray LensAssembly::generate_ray(float x, float y) const {
  glm::vec3 origin(x, y, _system_p2 + _dist);

  float theta = 2*PI*randf(), r = sqrt(randf())*_exit_pupil_rad;
  glm::vec3 pupil_pt(r*cos(theta), r*sin(theta), _exit_pupil_pos);

  glm::vec3 direction = glm::normalize(pupil_pt - origin);

  RayHit rayhit(origin, direction);

  for (unsigned _i = 0; _i < _surfaces.size(); ++_i) {
    unsigned i = _surfaces.size() - _i - 1;
    glm::vec3 center(0.0, 0.0, _surfaces[i].center());

    if (fabs(_surfaces[i].surface_radius()) < EPSILON) {
      if (! rayhit.intersect_plane(glm::vec3(0.0, 0.0, 1.0), center)) {
        return generate_ray(x, y);
      }
    } else {
      if (!rayhit.intersect_sphere(center, fabs(_surfaces[i].surface_radius()))) {
        // Ray didn't make it through the lenses
        return generate_ray(x, y);
      }
    }

    float index_a = _indices[i+1];
    float index_b = _indices[i];
    glm::vec3 new_origin = rayhit.intersection_point();
    glm::vec3 n = rayhit.norm();
    if (n.z < 0.0) {
      n = -n;
    }

    float costheta = glm::dot(n, -rayhit.ray().direction());
    float sintheta = glm::length(glm::cross(n, -rayhit.ray().direction()));
    float sinthetap = (index_a / index_b) * sintheta;
    float costhetap = sqrt(1 - sinthetap*sinthetap);
    glm::vec3 m = glm::normalize((-rayhit.ray().direction() - n*costheta) / sintheta);

    glm::vec3 new_dir = -n*costhetap - m*sinthetap;

    rayhit = RayHit(new_origin, new_dir);
  }

  origin = rayhit.ray().origin();
  direction = rayhit.ray().direction();

  return Ray(glm::vec3(origin.x, origin.y, 0.0), direction);
}
