#include "cmj_sampler.h"

#include <climits>
#include <cassert>

static double full_arcsin(double y) {
  return asin(2*y - 1);
}

static double times_two_pi(double y) {
  return 2*PI*y;
}

static inline unsigned permute(unsigned i, unsigned len, unsigned p) {
  unsigned w = len - 1;
  w |= w >> 1;
  w |= w >> 2;
  w |= w >> 4;
  w |= w >> 8;
  w |= w >> 16;
  do {
    i ^= p;
    i *= 0xe170893d;
    i ^= (i & w) >> 4;
    i ^= p >> 8;
    i *= 0x0929eb3f;
    i ^= p >> 23;
    i ^= (i & w) >> 1;
    i *= 1 | p >> 27;
    i *= 0x6935fa69;
    i ^= (i & w) >> 11;
    i *= 0x74dcb303;
    i ^= (i & w) >> 2;
    i *= 0x9e501cc3;
    i ^= (i & w) >> 2;
    i *= 0xc860a3df;
    i &= w;
    i ^= i >> 5;
  } while (i >= len);
  return (i + p) % len;
}

static inline double rand_float(unsigned i, unsigned p) {
  i ^= p;
  i ^= i >> 17;
  i ^= i >> 10;
  i *= 0xb36534e5;
  i ^= i >> 12;
  i ^= i >> 21;
  i *= 0x93fc4795;
  i ^= 0xdf6e307f;
  i ^= i >> 17;
  i *= 1 | p >> 18;
  return double(i) / (double(UINT_MAX) + 1.0);
}

CmjSampler2D CmjSampler2D::new_spherical(unsigned xdivs, unsigned ydivs) {
  return CmjSampler2D(xdivs, ydivs, times_two_pi, full_arcsin);
}

Sample CmjSampler2D::sample(unsigned i, unsigned j) {
  assert(i < _xdivs);
  assert(j < _ydivs);

  unsigned s = i*_ydivs + j;
  unsigned sx = permute(i, _xdivs, _permutation * 0xa511e9b3);
  unsigned sy = permute(j, _ydivs, _permutation * 0x63d83595);
  double jx = rand_float(s, _permutation * 0xa399d265);
  double jy = rand_float(s, _permutation * 0x711ad6a5);

  Sample r;
  r.x = (i + (sy + jx) / _ydivs) / _xdivs;
  r.y = (j + (sx + jy) / _xdivs) / _ydivs;

  if (_distributionx) {
    r.x = _distributionx(r.x);
  }

  if (_distributiony) {
    r.y = _distributiony(r.y);
  }

  return r;
}
