// A half-edge mesh data structure.
#ifndef MESH_H_
#define MESH_H_

#include <fstream>
#include <unordered_map>
#include <vector>

#include <cstring>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "debug_viz.h"
#include "material.h"

class Vertex;
class Edge;
class Face;

struct MeshShaderInfo {
  MeshShaderInfo() {
    memset((void*) this, 0, sizeof(MeshShaderInfo));
  }

  GLuint program;

  GLuint vpos_loc;
  GLuint vnorm_loc;

  GLuint diffuse_loc;
  GLuint specular_loc;
  GLuint ambient_loc;
  GLuint shiny_loc;
  GLuint modelmat_loc;
  GLuint viewmat_loc;
  GLuint projmat_loc;
  GLuint lightpos_loc;
  GLuint lightdiffuse_loc;
  GLuint lightspecular_loc;
  GLuint lightambient_loc;
  GLuint lightpower_loc;
};

class Vertex {
  public:
    void set_position(const glm::vec3 &newpos) { _position = newpos; }

    const glm::vec3 &position() const { return _position; }
    size_t index() const { return _index; }

  private:
    Vertex(const glm::vec3 &position, size_t index) : _index(index), _position(position) {}

    size_t _index;
    glm::vec3 _position;

    friend class Mesh;
};

struct VPair {
  const Vertex *a, *b;
  bool operator==(const VPair &other) const {
    return a->index() == other.a->index() && b->index() == other.b->index();
  }
};

struct HashVPair {
  size_t operator()(const VPair &pair) const;
};

typedef std::unordered_map<VPair, Edge*, HashVPair> edge_map_t;

class Edge {
  public:
    Edge *next() const { return _next; }
    Edge *opposite() const { return _opposite; }
    Vertex *vert() const { return _vert; }
    Vertex *root_vert() const { return _root_vert; }
    Face *face() const { return _face; }
    const glm::vec3 &vert_norm() const { return _vert_norm; }

    Edge *next_ccw() const;
    Edge *next_cw() const;

    void set_next(Edge *next) { _next = next; }
    void set_opposite(Edge *opposite) { _opposite = opposite; }
    void set_vert(Vertex *vert) { _vert = vert; }
    void set_root_vert(Vertex *root_vert) { _root_vert = root_vert; }
    void set_face(Face *face) { _face = face; }
    void set_vert_norm(const glm::vec3 &norm) { _vert_norm = norm; }

  private:
    Edge() : _next(NULL), _opposite(NULL), _vert(NULL), _root_vert(NULL), _face(NULL) {}
    Edge(Vertex *root_vert, Vertex *vert, Face *face) :
      _next(NULL), _opposite(NULL), _vert(vert), _root_vert(root_vert), _face(face) {}

    Edge *_next;
    Edge *_opposite;
    Vertex *_vert;
    Vertex *_root_vert;
    Face *_face;

    glm::vec3 _vert_norm;

    friend class Mesh;
};

class Face {
  public:
    Edge *edge() const { return _edge; }

    void set_edge(Edge *edge) { _edge = edge; }

    glm::vec3 norm() const;
    glm::vec3 centroid() const;
    float area() const;
    Vertex *vert(unsigned i) const;

    //void barycentric_coords(const glm::vec3 &point, float &alpha, float &beta, float &gamma) const;

    void transformed_verts(const glm::mat4 &transform, glm::vec3 &va, glm::vec3 &vb, glm::vec3 &vc) const;
    glm::vec3 transformed_norm(const glm::mat4 &transform) const;

    glm::vec3 interpolate_norm(float alpha, float beta, float gamma) const;

  private:
    Face() : _edge(NULL) {}

    Edge *_edge;

    friend class Mesh;
};

class Mesh {
  public:
    typedef std::vector<Vertex*>::const_iterator vert_iterator;
    typedef std::vector<Edge*>::const_iterator edge_iterator;
    typedef std::vector<Face*>::const_iterator face_iterator;

    typedef size_t mesh_id;
    static const mesh_id NONE;

    static Mesh from_obj(std::istream &infile);

    Mesh(Mesh &&other);
    ~Mesh();

    size_t add_vert(const glm::vec3 &position);
    const Vertex *vert(size_t i) const {
      assert(i < _vertices.size());
      return _vertices[i];
    }

    size_t add_tri(size_t v1, size_t v2, size_t v3);
    std::pair<size_t, size_t> add_quad(size_t v1, size_t v2, size_t v3, size_t v4);
    Face *face(size_t i) const {
      assert(i < _faces.size());
      return _faces[i];
    }

    vert_iterator verts_begin() const { return _vertices.begin(); }
    vert_iterator verts_end() const { return _vertices.end(); }
    edge_iterator edges_begin() const { return _edges.begin(); }
    edge_iterator edges_end() const { return _edges.end(); }
    face_iterator faces_begin() const { return _faces.begin(); }
    face_iterator faces_end() const { return _faces.end(); }

    size_t verts_size() const { return _vertices.size(); }
    size_t edges_size() const { return _edges.size(); }
    size_t faces_size() const { return _faces.size(); }

  private:
    Mesh() : _inited_buf(false), _vbuf(0), _vao(0) {}

    Edge *add_edge(Vertex *root_vert, Vertex *vert, Face *face);
    void compute_vert_norms();

    std::vector<Vertex*> _vertices;
    std::vector<Edge*> _edges;
    std::vector<Face*> _faces;

    edge_map_t _edge_map;

    static bool _s_inited;
    static void lazy_init_shaders();
    static MeshShaderInfo _shader;

    bool _inited_buf;
    void lazy_init_buffers();

    GLuint _vbuf;
    GLuint _vao;
    unsigned _n_verts;

    DebugViz _dbviz;

    friend class MeshInstance;
};

Mesh::mesh_id add_mesh_from_obj(const char *name, const char *obj_filename);
Mesh::mesh_id get_mesh_id(const char *name);

class MeshInstance {
  public:
    MeshInstance(Mesh::mesh_id id) :
      _id(id), _mtl_id(Material::NONE), _translate(0.0), _scale(1.0), _rotate_mat(1.0) {}
    MeshInstance(const MeshInstance&) = default;

    void set_mtl(Material::mtl_id id) { _mtl_id = id; }

    void draw();

    void translate(const glm::vec3 &offset) {
      _translate += offset;
    }

    void rotate(float angle, const glm::vec3 &axis) {
      glm::mat4 rot = glm::rotate(glm::mat4(1.0), angle, axis);
      _rotate_mat = rot * _rotate_mat;
    }

    void scale(const glm::vec3 &factor) {
      _scale *= factor;
    }

    void set_translate(const glm::vec3 &location) {
      _translate = location;
    }

    void set_rotate(float angle, const glm::vec3 &axis) {
      _rotate_mat = glm::rotate(glm::mat4(1.0), angle, axis);
    }

    void set_scale(const glm::vec3 &scale) {
      _scale = scale;
    }

    void reset_transform() {
      _translate = glm::vec3(0.0);
      _scale = glm::vec3(1.0);
      _rotate_mat = glm::mat4(1.0);
    }

    void set_viewmat(const glm::mat4 &viewmat);
    void set_projmat(const glm::mat4 &projmat);

    glm::mat4 modelmat() const;

    Mesh *mesh() const;
    const Material *material() const;

  private:
    Mesh::mesh_id _id;
    Material::mtl_id _mtl_id;
    glm::vec3 _translate;
    glm::vec3 _scale;
    glm::mat4 _rotate_mat;

    glm::mat4 _viewmat;
    glm::mat4 _projmat;
};

#endif /* MESH_H_ */
