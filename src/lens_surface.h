#ifndef _LENSSURFACE_H_
#define _LENSSURFACE_H_

#include <glm/glm.hpp>

// ====================================================================
// ====================================================================

class LensSurface {

public:
  // CONSTRUCTOR & DESTRUCTOR
  LensSurface(const glm::vec3 &center, float radius, float index, float aperture) {
    c = center;
    r = radius;
    a = aperture;
    ior = index;
  }

private:

  // REPRESENTATION
  glm::vec3 c;
  float r;
  float a; // radius of the aperture
  float ior;

};

// ====================================================================
// ====================================================================

#endif
