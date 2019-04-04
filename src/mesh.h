#ifndef MESH_H_
#define MESH_H_

#include <vector>

#include <glm/glm.hpp>

class Vertex {
  public:
    Vertex(const glm::vec3 &position) : _position(position) {}

    void set_position(const glm::vec3 &newpos) { _position = newpos; }

    const glm::vec3 &position() const { return _position; }

  private:
    glm::vec3 _position;
};

class Face;

class Edge {
  public:
    Edge() : _next(NULL), _opposite(NULL), _vert(NULL), _face(NULL) {}

    Edge(Vertex *vert, Face *face) :
      _next(NULL), _opposite(NULL), _vert(vert), _face(face) {}

    const Edge *next() const { return _next; }
    const Edge *opposite() const { return _opposite; }
    const Vertex *vert() const { return _vert; }
    const Face *face() const { return _face; }

    void set_next(Edge *next) { _next = next; }
    void set_opposite(Edge *opposite) { _opposite = opposite; }
    void set_vert(Vertex *vert) { _vert = vert; }
    void set_face(Face *face) { _face = face; }

  private:
    Edge *_next;
    Edge *_opposite;
    Vertex *_vert;
    Face *_face;
};

class Face {
  public:
    Face(unsigned num_verts) : _edge(NULL), _num_verts(num_verts) {}

    const Edge *edge() const { return _edge; }

    void set_edge(Edge *edge) { _edge = edge; }

    unsigned num_verts() const { return _num_verts; }

    glm::vec3 normal() const;
    glm::vec3 centroid() const;
    float area() const;
    const Vertex *vert(unsigned i) const;

  private:
    Edge *_edge;
    unsigned _num_verts;
};

class Mesh {
  public:
    typedef std::vector<Vertex*>::const_iterator vert_iterator;
    typedef std::vector<Edge*>::const_iterator edge_iterator;
    typedef std::vector<Face*>::const_iterator face_iterator;

    ~Mesh();

    size_t add_vert(const glm::vec3 &position);
    const Vertex *vert(size_t i) const {
      assert(i < _vertices.size());
      return _vertices[i];
    }

    size_t add_tri(size_t v1, size_t v2, size_t v3);
    size_t add_quad(size_t v1, size_t v2, size_t v3, size_t v4);
    const Face *face(size_t i) const {
      assert(i < _faces.size());
      return _faces[i];
    }

    vert_iterator vert_begin() const { return _vertices.begin(); }
    vert_iterator vert_end() const { return _vertices.end(); }
    edge_iterator edge_begin() const { return _edges.begin(); }
    edge_iterator edge_end() const { return _edges.end(); }
    face_iterator face_begin() const { return _faces.begin(); }
    face_iterator face_end() const { return _faces.end(); }

    size_t verts_size() const { return _vertices.size(); }
    size_t edges_size() const { return _edges.size(); }
    size_t faces_size() const { return _faces.size(); }

  private:
    std::vector<Vertex*> _vertices;
    std::vector<Edge*> _edges;
    std::vector<Face*> _faces;
};

#endif /* MESH_H_ */
