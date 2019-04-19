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

  GLuint program; // shader program

  GLuint vpos_loc; // per-vertex location, pre-transform (vec3)
  GLuint vnorm_loc; // per-vertex normal, pre-transform (vec3)

  GLuint diffuse_loc; // uniform material diffuse color (vec4)
  GLuint specular_loc; // uniform material specular color (vec4)
  GLuint ambient_loc; // uniform material ambient color (vec4)
  GLuint shiny_loc; // uniform material shininess exponent (float)
  GLuint modelmat_loc; // uniform mesh instance model matrix (mat4)
  GLuint viewmat_loc; // uniform camera view matrix (mat4)
  GLuint projmat_loc; // uniform camera projection matrix (mat4)
  GLuint lightpos_loc; // uniform light position (vec3)
  GLuint lightdiffuse_loc; // uniform light diffuse color (vec3)
  GLuint lightspecular_loc; // uniform light specular color (vec3)
  GLuint lightambient_loc; // uniform light ambient color (vec3)
  GLuint lightpower_loc; // uniform light energy/power (float)
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
    // Next edge counterclockwise around the face
    Edge *next() const { return _next; }

    // Opposite half-edge
    Edge *opposite() const { return _opposite; }

    // Counterclockwise-direction vertex
    Vertex *vert() const { return _vert; }

    // Clockwise-direction vertex
    Vertex *root_vert() const { return _root_vert; }

    // The face to the left of this Edge
    Face *face() const { return _face; }

    // The vertex normal of the counterclockwise-direction vertex for this edge's face
    const glm::vec3 &vert_norm() const { return _vert_norm; }

    // Return the next Edge counterclockwise around this edge's vertex (NOT this
    // edge's face)
    Edge *next_ccw() const;

    // Return the next edge clockwise around this edge's vertex (NOT this edge's
    // face)
    Edge *next_cw() const;

    // Set various edge properties/member variables
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
    // One of the edges of this Face
    Edge *edge() const { return _edge; }

    // Set the edge that will be returned by this->edge()
    void set_edge(Edge *edge) { _edge = edge; }

    // Get the surface normal of this Face.
    glm::vec3 norm() const;

    // Get the surface normal of this Face under the given transformation.
    glm::vec3 norm_transformed(const glm::mat4 &modelmat) const;

    // Get the centroid of this Face.
    glm::vec3 centroid() const;

    // Get the centroid of this Face under the given transformation.
    glm::vec3 centroid_transformed(const glm::mat4 &modelmat) const;

    // Get the area of this Face.
    float area() const;

    // Get the i'th vertex of this Face. `i` must be 0, 1, or 2.
    Vertex *vert(unsigned i) const;

    // Get the point on this face with the given barycentric coordinates.
    glm::vec3 point_at(float alpha, float beta, float gamma) const {
      return alpha*vert(0)->position() + beta*vert(1)->position() + gamma*vert(2)->position();
    }

    // Get the point on this face, under the given transformation, with the given
    // barycentric coordinates.
    glm::vec3 point_at_transformed(const glm::mat4 &modelmat,
        float alpha, float beta, float gamma) const;

    // Get the barycentric coordinates of the given point in terms of this Face's vertices.
    void barycentric_coords(const glm::vec3 &point, float &alpha, float &beta, float &gamma) const;

    // Get the barycentric coordinates of the given point in terms of this Face's vertices
    // under the given transformation.
    void barycentric_coords_transformed(
        const glm::vec3 &point, const glm::mat4 &modelmat,
        float &alpha, float &beta, float &gamma) const;

    // Generate a random point on this Face.
    glm::vec3 random_point() const;

    // Generate a random point on this Face under the given transformation.
    glm::vec3 random_point_transformed(const glm::mat4 &modelmat) const;

    // Get the vertices of this Face under the given transformation.
    void verts_transformed(const glm::mat4 &transform, glm::vec3 &va, glm::vec3 &vb, glm::vec3 &vc) const;

    // Get the surface normal of this Face, interpolated from the vertex normals.
    // `alpha`, `beta`, and `gamma` should be barycentric coordinates of a point on
    // this Face.
    glm::vec3 interpolate_norm(float alpha, float beta, float gamma) const;

    // Get the surface normal of this Face under the given transformation, interpolated
    // from the vertex normals. `alpha`, `beta`, and `gamma` should be barycentric
    // coordinates of a point on this Face.
    glm::vec3 interpolate_norm_transformed(const glm::mat4 &modelmat,
        float alpha, float beta, float gamma) const;

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

    // Create a Mesh by loading the given OBJ file
    static Mesh from_obj(std::istream &infile);

    Mesh(Mesh &&other);
    ~Mesh();

    // Add a vertex to this Mesh
    size_t add_vert(const glm::vec3 &position);

    // Return the vertex at index i
    const Vertex *vert(size_t i) const {
      assert(i < _vertices.size());
      return _vertices[i];
    }

    // Add a triangle face to this Mesh and return its index.
    size_t add_tri(size_t v1, size_t v2, size_t v3);

    // Add a quad face to this Mesh by first triangulating it. Returns the
    // pair of indices of the two triangles added.
    std::pair<size_t, size_t> add_quad(size_t v1, size_t v2, size_t v3, size_t v4);

    // Return the Face at index i
    Face *face(size_t i) const {
      assert(i < _faces.size());
      return _faces[i];
    }

    // Iterate over vertices, edges, and faces
    vert_iterator verts_begin() const { return _vertices.begin(); }
    vert_iterator verts_end() const { return _vertices.end(); }
    edge_iterator edges_begin() const { return _edges.begin(); }
    edge_iterator edges_end() const { return _edges.end(); }
    face_iterator faces_begin() const { return _faces.begin(); }
    face_iterator faces_end() const { return _faces.end(); }

    // Get the number of vertices, edges, and faces
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

// Add a Mesh with the given name to the global Mesh store by loading the OBJ
// file with the given filename.
Mesh::mesh_id add_mesh_from_obj(const char *name, const char *obj_filename);

// Get the Mesh ID associated with the given name, or Mesh::NONE if no such Mesh
// is in the store.
Mesh::mesh_id get_mesh_id(const char *name);

// An instance of a Mesh with arbitrary transformation and material.
class MeshInstance {
  public:
    // Create a new instance of the Mesh with the given ID.
    MeshInstance(Mesh::mesh_id id) :
      _id(id), _mtl_id(Material::NONE), _translate(0.0), _scale(1.0), _rotate_mat(1.0) {}
    MeshInstance(const MeshInstance&) = default;

    // Set the material ID for this mesh instance.
    void set_mtl(Material::mtl_id id) { _mtl_id = id; }

    // Draw the mesh instance with the current transformation, view, projection, and
    // material.
    void draw();

    // Translate the mesh instance by the given vector.
    void translate(const glm::vec3 &offset) {
      _translate += offset;
    }

    // Rotate the mesh instance around the given axis by the given angle. This rotation
    // is applied in the instance's local coordinate system.
    void rotate(float angle, const glm::vec3 &axis) {
      glm::mat4 rot = glm::rotate(glm::mat4(1.0), angle, axis);
      _rotate_mat = rot * _rotate_mat;
    }

    // Scale the mesh instance by the given axis factors. This scaling is applied in
    // the instance's local coordinate system.
    void scale(const glm::vec3 &factor) {
      _scale *= factor;
    }

    // Set the mesh instance's translation. Equivalently, set the instance's location.
    void set_translate(const glm::vec3 &location) {
      _translate = location;
    }

    // Reset the mesh instance's rotation and apply the given rotation.
    void set_rotate(float angle, const glm::vec3 &axis) {
      _rotate_mat = glm::rotate(glm::mat4(1.0), angle, axis);
    }

    // Set the mesh instance's scale.
    void set_scale(const glm::vec3 &scale) {
      _scale = scale;
    }

    // Reset translation, rotation, and scaling on this mesh instance.
    void reset_transform() {
      _translate = glm::vec3(0.0);
      _scale = glm::vec3(1.0);
      _rotate_mat = glm::mat4(1.0);
    }

    // Set the view and projection for this mesh instance.
    void set_viewmat(const glm::mat4 &viewmat);
    void set_projmat(const glm::mat4 &projmat);

    // Return the current transformation matrix for this mesh instance.
    glm::mat4 modelmat() const;

    // Get a pointer to the Mesh of which this is an instance.
    Mesh *mesh() const;

    // Get a pointer to this mesh instance's material.
    const Material *material() const;

    // Get this mesh instance's material ID.
    Material::mtl_id material_id() const { return _mtl_id; }

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
