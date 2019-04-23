#include "camera.h"

#include "canvas.h"
#include "util.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

Camera::Camera(const glm::vec3 &pos, const glm::vec3 &poi, const glm::vec3 &up) {
  _position = pos;
  _point_of_interest = poi;
  _up = glm::normalize(up);
  _rotate_speed = DEFAULT_ROTATE_SPEED;
}

void Camera::dolly(float dist) {
  float d = glm::length(_position - _point_of_interest);
  _position += 0.004f*d*dist*direction();
}

void Camera::truck(float dx, float dy) {
  float d = glm::length(_position - _point_of_interest);
  glm::vec3 translate = (d*0.0007f)*(horizontal()*dx + screen_up()*dy);
  _position += translate;
  _point_of_interest += translate;
}

void Camera::rotate(float rx, float ry) {
  rx *= _rotate_speed;
  ry *= _rotate_speed;

  float angle = rad_to_deg(acos(glm::dot(_up, direction())));
  if (angle - ry > 175.0 && ry < 0.0) {
    if (angle > 175.0) {
      ry = 0.0;
    } else {
      ry = 175.0 - angle;
    }
  } else if (angle - ry < 5.0 && ry > 0.0) {
    if (angle < 5.0) {
      ry = 0.0;
    } else {
      ry = angle - 5.0;
    }
  }

  glm::mat4 m;
  m = glm::translate(m, _point_of_interest);
  m = glm::rotate<float>(m, deg_to_rad(rx), _up);
  m = glm::rotate<float>(m, deg_to_rad(ry), horizontal());
  m = glm::translate(m, -_point_of_interest);
  glm::vec4 tmp(_position.x, _position.y, _position.z, 1);
  tmp = m * tmp;
  _position = glm::vec3(tmp.x, tmp.y, tmp.z);
}

OrthographicCamera::OrthographicCamera(
    const glm::vec3 &pos,
    const glm::vec3 &poi,
    const glm::vec3 &up,
    float size) :
  Camera(pos, poi, up), _size(size) {}

void OrthographicCamera::zoom(float factor) {
  _size *= pow(1.003, factor);
}

void OrthographicCamera::get_view_projection(glm::mat4 &view, glm::mat4 &projection) const {
  if (!Canvas::active()) {
    return;
  }

  double aspect = Canvas::aspect();
  float w, h;
  if (aspect < 1.0) {
    w = _size / 2.0;
    h = w / aspect;
  } else {
    h = _size / 2.0;
    w = h * aspect;
  }

  projection = glm::ortho(-w, w, -h, h, 0.1f, 100.0f);
  view = glm::lookAt(position(), point_of_interest(), screen_up());
}

Ray OrthographicCamera::cast_ray(double x, double y) const {
  double width = Canvas::width() >= Canvas::height() ? _size : _size * Canvas::aspect();
  double height = width / Canvas::aspect();

  glm::vec3 screen_center = position();
  glm::vec3 x_axis = horizontal() * float(width);
  glm::vec3 y_axis = screen_up() * float(height);
  glm::vec3 bottom_left = screen_center - 0.5f*x_axis - 0.5f*y_axis;
  glm::vec3 point = bottom_left + float(x)*x_axis + float(y)*y_axis;
  return Ray(point, direction());
}

PerspectiveCamera::PerspectiveCamera(
    const glm::vec3 &pos,
    const glm::vec3 &poi,
    const glm::vec3 &up,
    float angle) :
  Camera(pos, poi, up), _angle(angle) {}

void PerspectiveCamera::zoom(float factor) {
  _angle *= pow(1.002, factor);
  if (_angle < 5) {
    _angle = 5;
  } else if (_angle > 175) {
    _angle = 175;
  }
}

void PerspectiveCamera::get_view_projection(glm::mat4 &view, glm::mat4 &projection) const {
  if (!Canvas::active()) {
    return;
  }

  double aspect = Canvas::aspect();

  projection = glm::perspective<float>(deg_to_rad(_angle), aspect, 0.1f, 1000.0f);
  view = glm::lookAt(position(), point_of_interest(), screen_up());
}

Ray PerspectiveCamera::cast_ray(double x, double y) const {
  y = 1.0 - y;
  float screen_h = 2 * tan(deg_to_rad(_angle) * 0.5);
  float screen_w = screen_h * Canvas::aspect();

  glm::vec3 screen_center = position() + direction();
  glm::vec3 x_axis = horizontal() * screen_w;
  glm::vec3 y_axis = screen_up() * screen_h;
  glm::vec3 bottom_left = screen_center - 0.5f*x_axis - 0.5f*y_axis;
  glm::vec3 point = bottom_left + float(x)*x_axis + float(y)*y_axis;
  glm::vec3 dir = point - position();
  return Ray(point, dir);
}

LensCamera::LensCamera(
    const glm::vec3 &pos,
    const glm::vec3 &poi,
    const glm::vec3 &up,
    float angle,
    LensAssembly *la) :
  PerspectiveCamera(pos, poi, up, angle), _lens_assembly(la) {}

void LensCamera::add_surface(const LensSurface &ls) {
  _lens_assembly->add_surface(ls);
}

Ray LensCamera::cast_ray(double x, double y) const {
  return PerspectiveCamera::cast_ray(x, y); // TODO
}
