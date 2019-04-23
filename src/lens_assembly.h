#ifndef _LENSASSEMBLY_H_
#define _LENSASSEMBLY_H_

#include "lens_surface.h"
#include "raytracing.h"

#include <vector>

#include <glm/glm.hpp>

// ====================================================================
// ====================================================================


class LensAssembly {

public:
  // CONSTRUCTOR & DESTRUCTOR
  LensAssembly() : _dist(0.0), _exit_pupil_pos(0.0), _exit_pupil_rad(0.0) {}
  LensAssembly(float d, std::vector<LensSurface> &&surfaces = std::vector<LensSurface>()) :
    _surfaces(std::move(surfaces))
  {
    _dist = d;
    find_pupil();
  }

  static LensAssembly from_la(const char *filename);

  void set_dist(float d) { _dist = d; }
  void set_surfaces(std::vector<LensSurface> surfaces) {
    _surfaces = surfaces;
    find_pupil();
  }

  Ray generate_ray(float x, float y) const;

private:
  void find_pupil();

  // REPRESENTATION
  std::vector<LensSurface> _surfaces;
  float _dist; // dist from sensor (image plane) to lens
  float _exit_pupil_pos;
  float _exit_pupil_rad;
};

// ====================================================================
// ====================================================================

#endif
