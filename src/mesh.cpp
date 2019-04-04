#include "mesh.h"

#include <cassert>

const Vertex *Face::vert(unsigned i) const {
  assert(i < _num_verts);
  const Edge *e = _edge;
  --i;
  while (i < _num_verts) {
    e = e->next();
    --i;
  }

  return e->vert();
}

glm::vec3 Face::normal() const {
  glm::vec3 a, b;
  if (_num_verts == 3) {
    a = vert(1)->position() - vert(0)->position();
    b = vert(2)->position() - vert(0)->position();
  } else {
    a = vert(1)->position() - vert(3)->position();
    b = vert(2)->position() - vert(0)->position();
  }

  return glm::normalize(glm::cross(a, b));
}

glm::vec3 Face::centroid() const {
  if (_num_verts == 3) {
    return (vert(0)->position()
        + vert(1)->position()
        + vert(2)->position())
      * 0.3333333333f;
  } else {
    return (vert(0)->position()
        + vert(1)->position()
        + vert(2)->position()
        + vert(3)->position())
      * 0.25f;
  }
}

size_t Mesh::add_vert(const glm::vec3 &position) {
  _vertices.push_back(new Vertex(position));
  return _vertices.size() - 1;
}

size_t Mesh::add_tri(size_t v1, size_t v2, size_t v3) {
  assert(v1 < _vertices.size());
  assert(v2 < _vertices.size());
  assert(v3 < _vertices.size());

  Vertex *a = _vertices[v1];
  Vertex *b = _vertices[v2];
  Vertex *c = _vertices[v3];

  Face *f = new Face(3);
  Edge *ea = new Edge(a, f);
  Edge *eb = new Edge(b, f);
  Edge *ec = new Edge(c, f);

  _faces.push_back(f);
  _edges.push_back(ea);
  _edges.push_back(eb);
  _edges.push_back(ec);

  ea->set_next(eb);
  eb->set_next(ec);
  ec->set_next(ea);

  f->set_edge(ea);

  return _faces.size() - 1;
}

size_t Mesh::add_quad(size_t v1, size_t v2, size_t v3, size_t v4) {
  assert(v1 < _vertices.size());
  assert(v2 < _vertices.size());
  assert(v3 < _vertices.size());
  assert(v4 < _vertices.size());

  Vertex *a = _vertices[v1];
  Vertex *b = _vertices[v2];
  Vertex *c = _vertices[v3];
  Vertex *d = _vertices[v4];

  Face *f = new Face(4);
  Edge *ea = new Edge(a, f);
  Edge *eb = new Edge(b, f);
  Edge *ec = new Edge(c, f);
  Edge *ed = new Edge(d, f);

  _faces.push_back(f);
  _edges.push_back(ea);
  _edges.push_back(eb);
  _edges.push_back(ec);
  _edges.push_back(ed);

  ea->set_next(eb);
  eb->set_next(ec);
  ec->set_next(ed);
  ed->set_next(ea);

  f->set_edge(ea);

  return _faces.size() - 1;
}
