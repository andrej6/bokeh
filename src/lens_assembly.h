#ifndef _LENSASSEMBLY_H_
#define _LENSASSEMBLY_H_

#include <cmath>

#include <vector>

#include <glm/glm.hpp>

#include "util.h"
#include "raytracing.h"

// ====================================================================
// ====================================================================

class LensSurface {

public:
  // CONSTRUCTOR & DESTRUCTOR
  LensSurface(float center, float radius, float aperture) {
    c = center;
    r = radius;
    a = aperture;
  }

  float center() const { return c; }
  float surface_radius() const { return r; }
  float aperture_radius() const { return a; }
  float curvature() const { return 1.0f / r; }
  float vertex() const { return c - r; }

private:
  // REPRESENTATION
  float c;
  float r;
  float a; // radius of the aperture
};

class LensAssembly {

public:
  // CONSTRUCTOR & DESTRUCTOR
  LensAssembly() : _dist(0.0), _exit_pupil_pos(0.0), _exit_pupil_rad(0.0) {}

  static LensAssembly from_la(const char *filename);

  /*
  void set_dist(float d) { _dist = d; }
  void set_surfaces(std::vector<LensSurface> surfaces) {
    _surfaces = surfaces;
  }
  */

  // Get the number of surfaces in this LensAssembly.
  unsigned size() const { return _surfaces.size(); }

  // Generate a physically-based ray through the lens assembly.
  Ray generate_ray(float x, float y) const;

  // Get the optical power of the surface at the given index.
  float optical_power(unsigned surface) const {
    assert(surface < _surfaces.size());
    if (fabs(_surfaces[surface].surface_radius()) < EPSILON) {
      return 0.0;
    } else {
      return (_indices[surface+1] - _indices[surface]) * _surfaces[surface].curvature();
    }
  }

  // Get the effective focal length of a surface.
  float surface_focal_len(unsigned surface) const {
    return 1.0 / optical_power(surface);
  }

  // Get the front focal length of a surface.
  float surface_front_focal_len(unsigned surface) const {
    return -surface_focal_len(surface) * _indices[surface];
  }

  // Get the rear focal length of a surface.
  float surface_rear_focal_len(unsigned surface) const {
    return surface_focal_len(surface) * _indices[surface + 1];
  }

  // Reduce a series of surfaces to a single set of cardinal points. Returns the
  // comined optical power, the front and rear principal planes, and the front and
  // rear focal lengths of the system in the arguments `power`, `p1`, `p2`,
  // `f_front`, and `f_rear` respectively.
  void reduce(unsigned start, unsigned num_surfaces,
      float *power, float *p1, float *p2, float *f_front, float *f_rear) const;

  // Compute the paraxial refraction angle for a ray proceding towards image space,
  // with the given height and paraxial angle at the given surface.
  float paraxial_refract(unsigned surface, float height, float u) const {
    float phi = optical_power(surface);
    return (_indices[surface]*u - height*phi) / _indices[surface+1];
  }

  // Compute the paraxial refraction angle for a ray proceding toward object space,
  // with the given height and paraxial angle at the given surface.
  float paraxial_refract_rev(unsigned surface, float height, float u) const {
    float phi = optical_power(surface);
    return (_indices[surface+1]*u + height*phi) / _indices[surface];
  }

  // Compute the height at the next surface of a ray proceding from the given surface
  // towards image space, with the given initial height and paraxial angle.
  float paraxial_transfer(unsigned from, float height, float u) const {
    assert(from < _surfaces.size() - 1);
    float dist = _surfaces[from+1].vertex() - _surfaces[from].vertex();
    return height + u*dist;
  }

  // Compute the height at the next surface of a ray proceding from the given surface
  // towards object space, with the given initial height and paraxial angle.
  float paraxial_transfer_rev(unsigned from, float height, float u) const {
    assert(from > 0 && from < _surfaces.size());
    float dist = _surfaces[from].vertex() - _surfaces[from-1].vertex();
    return height - u*dist;
  }

  // Perform a paraxial raytrace from the given surface towards image space, through
  // `num_surfaces` surfaces. Returns the height and paraxial angle of the ray at
  // the final surface of the trace.
  void paraxial_raytrace(unsigned from, unsigned num_surfaces, float height, float angle,
      float *final_height, float *final_angle) const;

  // Perform a paraxial raytrace from the given surface towards object space, through
  // `num_surfaces` surfaces. Returns the height and paraxial angle of the ray at
  // the final surface of the trace.
  void paraxial_raytrace_rev(unsigned from, unsigned num_surfaces, float height, float angle,
      float *final_height, float *final_angle) const;

private:
  // Paraxial raytrace to find which surface is the system aperture stop
  void find_aperture_stop();

  // Find the location and size of the system exit pupil
  void find_pupil();

  // Find the system cardinal points, as well as the cardinal points of the
  // subsystems in front of and behind the aperture stop.
  void find_cardinal_points();

  // REPRESENTATION
  std::vector<LensSurface> _surfaces;
  std::vector<float> _indices;
  float _dist; // dist from sensor (image plane) to backmost surface

  // Cardinal planes (front and rear principal planes) and optical power
  // of the lens system in front of the aperture stop (nearer the scene)
  float _front_p1;
  float _front_p2;
  float _front_power;

  // Cardinal planes of the system behind the aperture stop (nearer the image plane)
  float _back_p1;
  float _back_p2;
  float _back_power;

  // Cardinal planes of the system as a whole
  float _system_p1;
  float _system_p2;
  float _system_power;

  // Which surface is the system aperture stop
  unsigned _aperture;

  // System exit pupil
  float _exit_pupil_pos;
  float _exit_pupil_rad;
};

// ====================================================================
// ====================================================================

#endif
