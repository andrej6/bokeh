// Scene cameras and projections.
#ifndef CAMERA_H_
#define CAMERA_H_

#ifndef DEFAULT_ROTATE_SPEED
# define DEFAULT_ROTATE_SPEED 0.2f
#endif

#include <glm/glm.hpp>

#include "canvas.h"
#include "raytracing.h"
#include "util.h"

// A top-level, pure virtual class representing a camera (viewpoint) in a 3D
// scene.
class Camera {
  public:
    Camera(const glm::vec3 &pos, const glm::vec3 &poi, const glm::vec3 &up);
    virtual ~Camera() {}

    void set_position(const glm::vec3 &pos) { _position = pos; }
    void set_point_of_interest(const glm::vec3 &poi) { _point_of_interest = poi; }
    void set_up(const glm::vec3 &up) { _up = up; }

    // Move the camera forward (positive values) or backwards (negative
    // values).
    void dolly(float dist);

    // Decrease or increase the camera's field of view (zoom in or out
    // respectively).
    virtual void zoom(float factor) = 0;

    // Move the camera perpendicular to its viewing direction.
    void truck(float dx, float dy);

    // Orbit the camera around the point of interest.
    void rotate(float rx, float ry);

    // Set the rotate speed of the camera.
    void set_rotate_speed(float rs) { _rotate_speed = rs; }

    // Return the rotate speed of the camera.
    float rotate_speed() const { return _rotate_speed; }

    // Return various orientation vectors of the camera. The `up()` vector
    // is unit length.
    const glm::vec3 &position() const { return _position; }
    const glm::vec3 &point_of_interest() const { return _point_of_interest; }
    const glm::vec3 &up() const { return _up; }

    // Compute and return a unit-length direction vector that is horizontal
    // relative to the camera.
    glm::vec3 horizontal() const {
      glm::vec3 answer = glm::cross(direction(), _up);
      return glm::normalize(answer);
    }

    // Compute and return the unit-length vector pointing directly up in the
    // screen space of the camera.
    glm::vec3 screen_up() const {
      glm::vec3 answer = glm::cross(horizontal(), direction());
      return glm::normalize(answer);
    }

    // Compute and return the unit-length vector pointing in the direction of
    // the camera's view.
    glm::vec3 direction() const {
      return glm::normalize(_point_of_interest - _position);
    }

    // Compute the view and projection matrices for this camera. The view matrix
    // takes points from world space into camera space, while the projection
    // matrix takes points from camera space to normalized device coordinates.
    //
    // Places the results in the corresponding arguments.
    virtual void get_view_projection(glm::mat4 &view, glm::mat4 &projection) const = 0;

    // Generate a ray through the given coordinates, according to the active Canvas's
    // aspect ratio.
    //
    // The given coordinates should both be normalized to the [0, 1] range, with 0
    // being the left screen edge for x and the bottom screen edge for y.
    virtual Ray cast_ray(double x, double y) const = 0;

  private:
    Camera() = delete;

    glm::vec3 _point_of_interest;
    glm::vec3 _position;
    glm::vec3 _up;
    float _rotate_speed;
};

// A camera with an orthographic projection.
class OrthographicCamera : public Camera {
  public:
    // An orthographic camera is defined by a rectangular prism with the image plane
    // as one face, and extending infinitely away from the image plane in the view
    // direction. In camera coordinates, the image plane's largest dimension will
    // be `size` units, with the other dimension determined according to the active
    // Canvas's aspect ratio.
    OrthographicCamera(const glm::vec3 &pos = glm::vec3(0,0,1),
        const glm::vec3 &poi = glm::vec3(0,0,0),
        const glm::vec3 &up = glm::vec3(0,1,0),
        float size=100);

    void set_size(float size) { _size = size; }
    void zoom(float factor);
    void get_view_projection(glm::mat4 &view, glm::mat4 &projection) const;
    Ray cast_ray(double x, double y) const;

  private:
    float _size;
};

// A camera with a perspective projection.
class PerspectiveCamera : public Camera {
  public:
    PerspectiveCamera(const glm::vec3 &pos = glm::vec3(0,0,1),
        const glm::vec3 &poi = glm::vec3(0,0,0),
        const glm::vec3 &up = glm::vec3(0,1,0),
        float fov = 45);

    void set_angle(float fov) { _angle = fov; }
    void zoom(float dist);
    void get_view_projection(glm::mat4 &view, glm::mat4 &projection) const;
    Ray cast_ray(double x, double y) const;

  private:
    float _angle;
};

#endif /* CAMERA_H_ */
