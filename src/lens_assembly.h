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
  LensAssembly(float d, const std::vector<LensSurface> &surfaces = std::vector<LensSurface>()) :
    _surfaces(surfaces)
  {
    _dist = d;
    find_pupil();
  }

  static LensAssembly *from_la(const char *filename);

  void set_dist(float d) { dist = d; }
  void add_surface(const LensSurface &surf) {
    _surfaces.push_back(surf);
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
