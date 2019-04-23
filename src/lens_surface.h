#ifndef _LENSSURFACE_H_
#define _LENSSURFACE_H_

#include <cmath>

#include "util.h"

// ====================================================================
// ====================================================================

class LensSurface {

public:
  // CONSTRUCTOR & DESTRUCTOR
  LensSurface(float center, float radius, float index, float aperture) {
    c = center;
    r = radius;
    a = aperture;
    ior = index;
  }

  float center() const { return c; }
  float radius_of_curvature() const { return r; }
  float aperture_radius() const { return a; }
  float index_of_refraction() const { return ior; }

  float curvature() const { return 1.0f / r; }
  float optical_power(float incoming_index) const {
    if (fabs(r) < EPSILON) {
      return 0.0;
    } else {
      return (ior - incoming_index) / r;
    }
  }
  float effective_focal_length(float incoming_index) const {
    return 1.0 / optical_power(incoming_index);
  }
  float front_focal_length(float incoming_index) const {
    return -incoming_index * effective_focal_length(incoming_index);
  }
  float rear_focal_length(float incoming_index) const {
    return ior * effective_focal_length(incoming_index);
  }
  float vertex_position() const { return c + r; }

  float paraxial_refract(float in_angle, float height, float index) const {
    return (ior*in_angle - height*optical_power(index)) / index;
  }

  float paraxial_refract_rev(float in_angle, float height, float index) const {
    return (index*in_angle + height*optical_power(index)) / ior;
  }

private:

  // REPRESENTATION
  float c;
  float r;
  float a; // radius of the aperture
  float ior;

};

// ====================================================================
// ====================================================================

#endif
