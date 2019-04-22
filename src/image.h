#ifndef IMAGE_H_
#define IMAGE_H_

#include <vector>
#include <algorithm>

#include <cassert>

#include <glm/glm.hpp>

typedef glm::vec<4, unsigned char, glm::defaultp> pixel_color;

class Image {
  public:
    Image(unsigned width, unsigned height)
      : _data(width*height), _w(width), _h(height) {}

    unsigned width() const { return _w; }
    unsigned height() const { return _h; }

    const pixel_color &pixel(unsigned x, unsigned y) const {
      return _data[index(x, y)];
    }

    glm::vec3 pixelf(unsigned x, unsigned y) const {
      assert(x < _w);
      assert(y < _h);
      return floatvec(pixel(x, y));
    }

    void set_pixel(unsigned x, unsigned y, const pixel_color &color) {
      assert(x < _w);
      assert(y < _h);
      _data[index(x, y)] = color;
    }

    void set_pixel(unsigned x, unsigned y, const glm::vec4 &color) {
      assert(x < _w);
      assert(y < _h);
      set_pixel(x, y, charvec(color));
    }

    void set_pixel_range(unsigned x0, unsigned y0, unsigned width, unsigned height,
        const pixel_color &color)
    {
      unsigned x1 = std::min(x0 + width, _w);
      unsigned y1 = std::min(y0 + height, _h);
      for (unsigned i = x0; i < x1; ++i) {
        for (unsigned j = y0; j < y1; ++j) {
          _data[index(i, j)] = color;
        }
      }
    }

    void set_pixel_range(unsigned x0, unsigned y0, unsigned width, unsigned height,
        const glm::vec4 &color)
    {
      set_pixel_range(x0, y0, width, height, charvec(color));
    }

    void clear_to_color(const pixel_color &color) {
      for (unsigned i = 0; i < _data.size(); ++i) {
        _data[i] = color;
      }
    }

    void clear_to_color(const glm::vec4 &color) {
      clear_to_color(charvec(color));
    }

    const pixel_color *data() const { return _data.data(); }
    size_t num_pixels() const { return _w * _h; }
    size_t data_bytes() const { return _data.size() * sizeof(pixel_color); }

  private:
    unsigned index(unsigned x, unsigned y) const {
      return (_h - y - 1)*_w + x;
    }

    unsigned char charval(float x) const {
      assert(0.0 <= x);
      assert(x <= 1.0);
      return (unsigned char) 255*x;
    }

    float floatval(unsigned char x) const {
      return (float) x / 255.0f;
    }

    pixel_color charvec(const glm::vec4 &vec) const {
      return pixel_color(charval(vec.r), charval(vec.g), charval(vec.b), charval(vec.a));
    }

    glm::vec4 floatvec(const pixel_color &vec) const {
      return glm::vec4(floatval(vec.r), floatval(vec.g), floatval(vec.b), floatval(vec.a));
    }

    std::vector<pixel_color> _data;
    unsigned _w, _h;
};

#endif /* IMAGE_H_ */
