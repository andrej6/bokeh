#ifndef _LENSASSEMBLY_H_
#define _LENSASSEMBLY_H_

#include "lens_surface.h"
#include <vector>

// ====================================================================
// ====================================================================


class LensAssembly {

public:
  // CONSTRUCTOR & DESTRUCTOR
  LensAssembly(float d) {
    dist = d;
  }

  void add_surface(const LensSurface &surf) {
    surfaces.push_back(surf);
  }


private:

  // REPRESENTATION
  std::vector<LensSurface> surfaces;
  float dist; // dist from sensor (image plane) to lens
};

// ====================================================================
// ====================================================================

#endif
