#ifndef CMJ_SAMPLER_H_
#define CMJ_SAMPLER_H_

#include "util.h"

#include <cmath>

typedef struct Sample {
  double x;
  double y;
} Sample;

// A 2D correlated multi-jittering sampler.
class CmjSampler2D {
  public:
    typedef double (*distr_func)(double);

    // Create a new CmjSampler2D with a linear distribution
    static CmjSampler2D new_linear(unsigned xdivs, unsigned ydivs) {
      return CmjSampler2D(xdivs, ydivs, NULL, NULL);
    }

    // Create a new CmjSampler2D with an arcsin distribution on the y coordinate,
    // and linear on the x coordinate.
    static CmjSampler2D new_hemispherical(unsigned xdivs, unsigned ydivs) {
      return CmjSampler2D(xdivs, ydivs, NULL, asin);
    }

    // Create a new CmjSampler2D with a spherical distribution.
    static CmjSampler2D new_spherical(unsigned xdivs, unsigned ydivs);

    // Create a new CmjSampler2D with an arbitrary distribution given by `distrx`
    // and `distry`, for the x and y coordinate distributions respectively.
    // These should be pointers to functions that accept one double in the
    // range [0,1]. They should return a double in the range [0,1] and
    // be monotonically increasing in this range.
    static CmjSampler2D new_with_distr(unsigned xdivs, unsigned ydivs,
        distr_func distrx, distr_func distry) {
      return CmjSampler2D(xdivs, ydivs, distrx, distry);
    }

    // Re-randomize the samples.
    void jitter() {
      _permutation = randi();
    }

    // Get the sample coordinates for stratum cell i,j.
    Sample sample(unsigned i, unsigned j);

  private:
    CmjSampler2D() = delete;
    CmjSampler2D(unsigned x, unsigned y, distr_func dx, distr_func dy)
      : _xdivs(x), _ydivs(y), _permutation(randi()),
      _distributionx(dx), _distributiony(dy) {}

    CmjSampler2D(unsigned x, unsigned y, distr_func dx, distr_func dy, unsigned seed)
      : _xdivs(x), _ydivs(y), _permutation(seed), _distributionx(dx), _distributiony(dy) {}

    unsigned _xdivs, _ydivs;
    unsigned _permutation;
    distr_func _distributionx, _distributiony;
};

#endif /* CMJ_SAMPLER_H_ */
