#ifndef _LENSSURFACE_H_
#define _LENSSURFACE_H_

#include <cmath>

#include "util.h"

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

private:
  // REPRESENTATION
  float c;
  float r;
  float a; // radius of the aperture
};

// ====================================================================
// ====================================================================

#endif
