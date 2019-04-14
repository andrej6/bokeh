#include "mesh.h"

#include <string>
#include <utility>
#include <vector>

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include "shader_store.h"
#include "util.h"

#define SHADER_PROG_NAME "mesh_gouraud"
#define VERT_SHADER_NAME "mesh_gouraud.vert"
#define FRAG_SHADER_NAME "mesh_gouraud.frag"
#define VERT_SHADER_FILE "shaders/mesh_basic_gouraud.vert"
#define FRAG_SHADER_FILE "shaders/mesh_basic_gouraud.frag"

bool Mesh::_s_inited = false;
MeshShaderInfo Mesh::_shader;

struct MeshVertData {
  glm::vec3 pos;
  glm::vec3 norm;
  glm::vec4 diffuse;
  glm::vec4 specular;
  glm::vec4 ambient;
  float shiny;
};

size_t HashVPair::operator()(const VPair &pair) const {
  size_t idx_a = pair.a->index();
  size_t idx_b = pair.b->index();

  size_t hash = 14235337;

  for (unsigned i = 0; i < sizeof(size_t); ++i) {
    hash = (hash << 13) + hash;
    hash += idx_a & 0xff;
    idx_a >>= 8;
  }

  for (unsigned i = 0; i < sizeof(size_t); ++i) {
    hash = (hash << 13) + hash;
    hash += idx_b & 0xff;
    idx_b >>= 8;
  }

  return hash;
}

struct FaceIndexData {
  std::vector<int> verts;
  std::vector<int> norms;
};

void parse_geom_data(
    std::istream &infile,
    std::vector<glm::vec3> &vert_pos,
    std::vector<glm::vec3> &vert_norm,
    std::vector<FaceIndexData> &faces)
{
  std::string line;
  while (std::getline(infile, line)) {
    line = strip(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    auto tokens = split(line);

    if (tokens[0] == "v") {
      if (tokens.size() != 4) {
        glerr() << "ERROR: unsupported number of vertex coordinates in OBJ" << std::endl;
        exit(-1);
      }

      glm::vec3 pt;
      for (unsigned i = 1; i < 4; ++i) {
        float f;
        if (sscanf(tokens[i].c_str(), "%f", &f) != 1) {
          glerr() << "ERROR: invalid vertex coordinate in OBJ" << std::endl;
          exit(-1);
        }
        pt[i-1] = f;
      }
      vert_pos.push_back(pt);

    } else if (tokens[0] == "vt") {
      continue;
    } else if (tokens[0] == "vn") {
      if (tokens.size() != 4) {
        glerr() << "ERROR: more than three components given for vertex normal in OBJ" << std::endl;
        exit(-1);
      }

      glm::vec3 n;
      for (unsigned i = 1; i < 4; ++i) {
        float f;
        if (sscanf(tokens[i].c_str(), "%f", &f) != 1) {
          glerr() << "ERROR: invalid normal component in OBJ" << std::endl;
          exit(-1);
        }
        n[i-1] = f;
      }
      vert_norm.push_back(glm::normalize(n));

    } else if (tokens[0] == "f") {
      if (tokens.size() < 4 || tokens.size() > 5) {
        glerr() << "ERROR: unsupported polygon type in OBJ" << std::endl;
        exit(-1);
      }

      FaceIndexData f;
      for (unsigned i = 1; i < tokens.size(); ++i) {
        auto indices = split(tokens[i], "/", false);
        int pos_idx, norm_idx = 0;

        if (sscanf(indices[0].c_str(), "%d", &pos_idx) != 1) {
          glerr() << "ERROR: face vertex without position in OBJ" << std::endl;
          exit(-1);
        }

        if (pos_idx == 0) {
          glerr() << "ERROR: face with invalid vertex index in OBJ" << std::endl;
          exit(-1);
        }

        if (indices.size() >= 3 && sscanf(indices[2].c_str(), "%d", &norm_idx) != 1) {
          glerr() << "ERROR: face vertex with blank normal vector slot in OBJ" << std::endl;
          exit(-1);
        }

        f.verts.push_back(pos_idx);
        f.norms.push_back(norm_idx);
      }
      faces.push_back(f);

    } else {
      glerr() << "ERROR: unsupported OBJ item '" << tokens[0] << "', aborting" << std::endl;
      exit(-1);
    }
  }

  for (unsigned i = 0; i < faces.size(); ++i) {
    for (unsigned j = 0; j < faces[i].verts.size(); ++j) {
      int pos_idx = faces[i].verts[j];
      int norm_idx = faces[i].norms[j];

      if (pos_idx < 0) {
        pos_idx += vert_pos.size();
        if (pos_idx < 0) {
          glerr() << "ERROR: face with invalid vertex index in OBJ" << std::endl;
          exit(-1);
        }
      }

      if (norm_idx < 0) {
        norm_idx += vert_norm.size();
        if (norm_idx < 0) {
          glerr() << "ERROR: face with invalid vertex normal index in OBJ" << std::endl;
          exit(-1);
        }
      }

      faces[i].verts[j] = pos_idx - 1;
      faces[i].norms[j] = norm_idx - 1;
    }
  }
}

Mesh Mesh::from_obj(std::istream &infile) {
  std::vector<glm::vec3> vert_pos, vert_norm;
  std::vector<FaceIndexData> faces;
  parse_geom_data(infile, vert_pos, vert_norm, faces);

  Mesh m;

  for (unsigned i = 0; i < vert_pos.size(); ++i) {
    m.add_vert(vert_pos[i]);
  }

  for (unsigned i = 0; i < faces.size(); ++i) {
    if (faces[i].verts.size() == 3) {
      m.add_tri(faces[i].verts[0], faces[i].verts[1], faces[i].verts[2]);
    } else {
      m.add_quad(faces[i].verts[0], faces[i].verts[1], faces[i].verts[2], faces[i].verts[3]);
    }

    for (unsigned j = 0; j < faces[i].norms.size(); ++j) {
      if (faces[i].norms[j] < 0) {
        continue;
      }

      size_t i2 = faces[i].verts[j];

      size_t i1 = j-1;
      if (i1 > faces[i].verts.size()) {
        i1 = faces[i].verts.size() - 1;
      }
      i1 = faces[i].verts[i1];

      edge_map_t::iterator itr = m._edge_map.find(VPair { m.vert(i1), m.vert(i2) });
      assert(itr != m._edge_map.end());

      itr->second->set_vert_norm(vert_norm[faces[i].norms[j]]);
    }
  }

  m.compute_vert_norms();
  return m;
}

Edge *Edge::next_ccw() const {
  if (!_opposite) {
    return NULL;
  } else {
    return _opposite->next()->next();
  }
}

Edge *Edge::next_cw() const {
  return _next->opposite();
}

static glm::vec3 compute_vert_norm(Edge *e) {
  Edge *e1 = e;

  while (e->next_cw()) {
    e = e->next_cw();
    if (e == e1) {
      break;
    }
  }

  e1 = e;
  std::vector<glm::vec3> norms;

  while (1) {
    glm::vec3 n = e->face()->norm();
    norms.push_back(n);

    if (!e->next_ccw()) {
      break;
    }

    e = e->next_ccw();
    if (e == e1) {
      break;
    }
  }

  glm::vec3 avg(0.0);
  for (unsigned i = 0; i < norms.size(); ++i) {
    avg += norms[i];
  }

  return glm::normalize(avg / float(norms.size()));
}

void Mesh::compute_vert_norms() {
  for (unsigned i = 0; i < _edges.size(); ++i) {
    Edge *e = _edges[i];
    if (glm::length(e->vert_norm()) < EPSILON) {
      e->set_vert_norm(compute_vert_norm(e));
    }
  }
}

glm::vec3 Face::norm() const {
  glm::vec3 a, b;
  a = vert(1)->position() - vert(0)->position();
  b = vert(2)->position() - vert(0)->position();

  return glm::normalize(glm::cross(a, b));
}

glm::vec3 Face::centroid() const {
  return (vert(0)->position()
      + vert(1)->position()
      + vert(2)->position())
    * 0.3333333333f;
}

float Face::area() const {
  glm::vec3 lega = vert(1)->position() - vert(0)->position();
  glm::vec3 legb = vert(2)->position() - vert(0)->position();
  return 0.5*glm::length(glm::cross(lega, legb));
}

Vertex *Face::vert(unsigned i) const {
  assert(i < 3);
  const Edge *e = _edge;
  --i;
  while (i < 3) {
    e = e->next();
    --i;
  }

  return e->vert();
}

void Face::transformed_verts(const glm::mat4 &transform,
    glm::vec3 &va, glm::vec3 &vb, glm::vec3 &vc) const
{
  glm::vec3 orig_a = vert(0)->position(),
            orig_b = vert(1)->position(),
            orig_c = vert(2)->position();

  glm::vec4 homog(orig_a.x, orig_a.y, orig_a.z, 1.0);
  homog = transform * homog;
  va = glm::vec3(homog.x, homog.y, homog.z) / homog.w;

  homog = glm::vec4(orig_b.x, orig_b.y, orig_b.z, 1.0);
  homog = transform * homog;
  vb = glm::vec3(homog.x, homog.y, homog.z) / homog.w;

  homog = glm::vec4(orig_c.x, orig_c.y, orig_c.z, 1.0);
  homog = transform * homog;
  vc = glm::vec3(homog.x, homog.y, homog.z) / homog.w;
}

glm::vec3 Face::transformed_norm(const glm::mat4 &transform) const {
  glm::vec3 orig_n = norm();
  glm::vec4 n(orig_n.x, orig_n.y, orig_n.z, 0.0);
  n = transform * n;
  return glm::normalize(glm::vec3(n.x, n.y, n.z));
}

glm::vec3 Face::interpolate_norm(float alpha, float beta, float gamma) const {
  Edge *e = _edge;
  glm::vec3 n1 = e->vert_norm();
  e = e->next();
  glm::vec3 n2 = e->vert_norm();
  e = e->next();
  glm::vec3 n3 = e->vert_norm();

  return glm::normalize(alpha*n1 + beta*n2 + gamma*n3);
}

Mesh::Mesh(Mesh &&other) {
  _vertices = std::move(other._vertices);
  _edges = std::move(other._edges);
  _faces = std::move(other._faces);
  _edge_map = std::move(other._edge_map);
  _inited_buf = other._inited_buf;
  _vbuf = other._vbuf;
  _vao = other._vao;

  other._inited_buf = false;
  other._vbuf = 0;
  other._vao = 0;
}

Mesh::~Mesh() {
  for (unsigned i = 0; i < _vertices.size(); ++i) {
    delete _vertices[i];
  }
  _vertices.clear();

  for (unsigned i = 0; i < _edges.size(); ++i) {
    delete _edges[i];
  }
  _edges.clear();

  for (unsigned i = 0; i < _faces.size(); ++i) {
    delete _faces[i];
  }
  _faces.clear();

  _edge_map.clear();

  if (_inited_buf) {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbuf);

    glBindVertexArray(0);

    _inited_buf = false;
    _vbuf = 0;
    _vao = 0;
  }
}

size_t Mesh::add_vert(const glm::vec3 &position) {
  _vertices.push_back(new Vertex(position, _vertices.size()));
  return _vertices.size() - 1;
}

size_t Mesh::add_tri(size_t v1, size_t v2, size_t v3) {
  assert(v1 < _vertices.size());
  assert(v2 < _vertices.size());
  assert(v3 < _vertices.size());

  Vertex *a = _vertices[v1];
  Vertex *b = _vertices[v2];
  Vertex *c = _vertices[v3];

  Face *f = new Face();
  Edge *ea = add_edge(c, a, f);
  Edge *eb = add_edge(a, b, f);
  Edge *ec = add_edge(b, c, f);

  ea->set_next(eb);
  eb->set_next(ec);
  ec->set_next(ea);

  f->set_edge(ea);

  _faces.push_back(f);

  return _faces.size() - 1;
}

std::pair<size_t, size_t> Mesh::add_quad(size_t v1, size_t v2, size_t v3, size_t v4) {
  return std::make_pair(add_tri(v1, v2, v3), add_tri(v1, v3, v4));
}

Edge *Mesh::add_edge(Vertex *root_vert, Vertex *vert, Face *face) {
  edge_map_t::iterator itr = _edge_map.find({root_vert, vert});
  if (itr != _edge_map.end()) {
    glerr() << "ERROR: adding edge that already exists" << std::endl;
    exit(-1);
  }

  Edge *e = new Edge(root_vert, vert, face);

  itr = _edge_map.find({vert, root_vert});
  if (itr != _edge_map.end()) {
    e->set_opposite(itr->second);
    itr->second->set_opposite(e);
  }

  _edge_map.insert(std::make_pair(VPair {root_vert, vert}, e));
  _edges.push_back(e);

  return e;
}

void Mesh::lazy_init_shaders() {
  if (_s_inited) {
    return;
  }

  bool success = true;
  success &= add_shader_source_file(VERT_SHADER_NAME, VERT_SHADER_FILE, GL_VERTEX_SHADER);
  success &= add_shader_source_file(FRAG_SHADER_NAME, FRAG_SHADER_FILE, GL_FRAGMENT_SHADER);

  success &= compile_shader(VERT_SHADER_NAME);
  success &= compile_shader(FRAG_SHADER_NAME);

  success &= add_program(SHADER_PROG_NAME);
  success &= add_shader_to_program(VERT_SHADER_NAME, SHADER_PROG_NAME);
  success &= add_shader_to_program(FRAG_SHADER_NAME, SHADER_PROG_NAME);
  success &= link_program(SHADER_PROG_NAME);

  if (success) {
    _s_inited = true;
  } else {
    return;
  }

  _shader.program = program_loc(SHADER_PROG_NAME);

  handle_gl_error("[Mesh::lazy_init_shaders] Before getting input locations");
  _shader.vpos_loc = glGetAttribLocation(_shader.program, "vpos");
  _shader.vnorm_loc = glGetAttribLocation(_shader.program, "vnorm");
  _shader.vdiffuse_loc = glGetAttribLocation(_shader.program, "vdiffuse");
  _shader.vspecular_loc = glGetAttribLocation(_shader.program, "vspecular");
  _shader.vambient_loc = glGetAttribLocation(_shader.program, "vambient");
  _shader.vshiny_loc = glGetAttribLocation(_shader.program, "vshiny");

  _shader.modelmat_loc = glGetUniformLocation(_shader.program, "modelmat");
  _shader.viewmat_loc = glGetUniformLocation(_shader.program, "viewmat");
  _shader.projmat_loc = glGetUniformLocation(_shader.program, "projmat");
  _shader.lightpos_loc = glGetUniformLocation(_shader.program, "lightpos");
  _shader.lightdiffuse_loc = glGetUniformLocation(_shader.program, "lightdiffuse");
  _shader.lightspecular_loc = glGetUniformLocation(_shader.program, "lightspecular");
  _shader.lightambient_loc = glGetUniformLocation(_shader.program, "lightambient");
  _shader.lightpower_loc = glGetUniformLocation(_shader.program, "lightpower");

  handle_gl_error("[Mesh::lazy_init_shaders] Before enabling attribs");
}

void Mesh::lazy_init_buffers() {
  if (!_s_inited) {
    return;
  }

  if (_inited_buf) {
    return;
  }

  handle_gl_error("[Mesh::lazy_init_buffers] Entering function");
  glUseProgram(_shader.program);

  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbuf);
  handle_gl_error("[Mesh::lazy_init_buffers] After gen buffer");

  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbuf);
  glEnable(GL_CULL_FACE);

  std::vector<MeshVertData> vert_data;

  Edge *face_edges[6];
  memset((void*) face_edges, 0, sizeof(face_edges));
  for (unsigned i = 0; i < _faces.size(); ++i) {
    Face *f = _faces[i];
    Edge *ea = f->edge();
    Edge *eb = ea->next();
    Edge *ec = eb->next();

    face_edges[0] = ea;
    face_edges[1] = eb;
    face_edges[2] = ec;

    glm::vec3 face_norm = f->norm();
    for (unsigned j = 0; j < 6; ++j) {
      Edge *e = face_edges[j];
      if (!e) {
        break;
      }
      MeshVertData vd;
      vd.pos = e->vert()->position();

      vd.diffuse = glm::vec4(0.7, 0.7, 0.7, 1.0);
      vd.specular = glm::vec4(0.9, 0.9, 0.9, 1.0);
      vd.ambient = glm::vec4(0.3, 0.3, 0.3, 1.0);
      vd.shiny = 50.0;

      if (glm::length(e->vert_norm()) < EPSILON) {
        vd.norm = face_norm;
      } else {
        vd.norm = e->vert_norm();
      }

      vert_data.push_back(vd);
    }
  }

  glUseProgram(_shader.program);
  glEnableVertexAttribArray(_shader.vpos_loc);
  glEnableVertexAttribArray(_shader.vnorm_loc);
  glEnableVertexAttribArray(_shader.vdiffuse_loc);
  glEnableVertexAttribArray(_shader.vspecular_loc);
  glEnableVertexAttribArray(_shader.vambient_loc);
  glEnableVertexAttribArray(_shader.vshiny_loc);
  handle_gl_error("[Mesh::lazy_init_buffers] After enabling attribs");

  glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertData) * vert_data.size(),
      vert_data.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(_shader.vpos_loc, 3, GL_FLOAT, false,
      sizeof(MeshVertData), (void*) offsetof(MeshVertData, pos));
  glVertexAttribPointer(_shader.vnorm_loc, 3, GL_FLOAT, false,
      sizeof(MeshVertData), (void*) offsetof(MeshVertData, norm));
  glVertexAttribPointer(_shader.vdiffuse_loc, 4, GL_FLOAT, false,
      sizeof(MeshVertData), (void*) offsetof(MeshVertData, diffuse));
  glVertexAttribPointer(_shader.vspecular_loc, 4, GL_FLOAT, false,
      sizeof(MeshVertData), (void*) offsetof(MeshVertData, specular));
  glVertexAttribPointer(_shader.vambient_loc, 4, GL_FLOAT, false,
      sizeof(MeshVertData), (void*) offsetof(MeshVertData, ambient));
  glVertexAttribPointer(_shader.vshiny_loc, 1, GL_FLOAT, false,
      sizeof(MeshVertData), (void*) offsetof(MeshVertData, shiny));
  handle_gl_error("[Mesh::lazy_init_buffers] After setting attrib ptrs");

  glEnable(GL_DEPTH_TEST);

  _n_verts = vert_data.size();

  _inited_buf = true;
  handle_gl_error("[Mesh::lazy_init_buffers] Finishing function");
}

const Mesh::mesh_id Mesh::NONE = 0;
static Mesh::mesh_id next_mesh_id = 1;

typedef std::unordered_map<Mesh::mesh_id, Mesh*> mesh_map_t;
typedef std::unordered_map<std::string, Mesh::mesh_id> mesh_name_map_t;

static struct MeshManager {
  ~MeshManager() {
    for (mesh_map_t::iterator itr = meshes.begin(); itr != meshes.end(); ++itr) {
      delete itr->second;
    }
  }

  mesh_name_map_t mesh_names;
  mesh_map_t meshes;
} meshes;

Mesh::mesh_id add_mesh_from_obj(const char *name, const char *obj_filename) {
  Mesh::mesh_id id = next_mesh_id;
  ++next_mesh_id;

  std::ifstream objfile(obj_filename);
  assert(objfile.good());

  Mesh *m = new Mesh(Mesh::from_obj(objfile));
  meshes.meshes.insert(std::make_pair(id, m));
  meshes.mesh_names.insert(std::make_pair(name, id));

  return id;
}

Mesh::mesh_id get_mesh_id(const char *name) {
  mesh_name_map_t::iterator itr = meshes.mesh_names.find(name);
  if (itr == meshes.mesh_names.end()) {
    return Mesh::NONE;
  } else {
    return itr->second;
  }
}

void MeshInstance::draw() {
  Mesh::lazy_init_shaders();
  Mesh *m = this->mesh();
  m->lazy_init_buffers();
  if (!m->_inited_buf) {
    return;
  }

  MeshShaderInfo *shader = &Mesh::_shader;

  handle_gl_error("[MeshInstance::draw] Entering function");

  glBindVertexArray(m->_vao);
  glUseProgram(shader->program);
  glEnable(GL_DEPTH_TEST);

  glm::mat4 modelmat(this->modelmat());

  glUniformMatrix4fv(shader->modelmat_loc, 1, false, (float*) &modelmat);
  glUniformMatrix4fv(shader->viewmat_loc, 1, false, (float*) &_viewmat);
  glUniformMatrix4fv(shader->projmat_loc, 1, false, (float*) &_projmat);
  handle_gl_error("[MeshInstance::draw] Set modelmat");

  glm::vec3 lightpos(10, 10, 10);
  glm::vec3 lightdiffuse(1.0, 1.0, 1.0);
  glm::vec3 lightspecular(1.0, 1.0, 1.0);
  glm::vec3 lightambient(0.4, 0.3, 0.5);
  float lightpower = 200.0;

  glUniform3fv(shader->lightpos_loc, 1, (float*) &lightpos);
  glUniform3fv(shader->lightdiffuse_loc, 1, (float*) &lightdiffuse);
  glUniform3fv(shader->lightspecular_loc, 1, (float*) &lightspecular);
  glUniform3fv(shader->lightambient_loc, 1, (float*) &lightambient);
  glUniform1f(shader->lightpower_loc, lightpower);
  handle_gl_error("[MeshInstance::draw] Set other uniforms");

  glDrawArrays(GL_TRIANGLES, 0, m->_n_verts);

  handle_gl_error("[MeshInstance::draw] Finishing function");

  DebugViz *dbviz = &m->_dbviz;
  dbviz->set_modelmat(modelmat);
  dbviz->set_viewmat(_viewmat);
  dbviz->set_projmat(_projmat);
  dbviz->draw();
}

void MeshInstance::set_viewmat(const glm::mat4 &viewmat) {
  _viewmat = viewmat;
}

void MeshInstance::set_projmat(const glm::mat4 &projmat) {
  _projmat = projmat;
}

glm::mat4 MeshInstance::modelmat() const {
  glm::mat4 model(1.0);
  model = glm::scale(model, _scale);
  model = _rotate_mat * model;
  model = glm::translate(model, _translate);
  return model;
}

Mesh *MeshInstance::mesh() const {
  return meshes.meshes.find(_id)->second;
}
