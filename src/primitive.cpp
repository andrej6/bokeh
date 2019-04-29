#include "primitive.h"

#include "util.h"

#define SPHERE_MESH_NAME "__PRIMITIVE_sphere"

static Mesh::mesh_id sphere_mesh(unsigned latdivs, unsigned londivs) {
  Mesh m;

  m.add_vert(glm::vec3(0.0, 0.0, 1.0));

  for (unsigned i = 1; i < latdivs; ++i) {
    for (unsigned j = 0; j < londivs; ++j) {
      double theta = j*2*PI / londivs;
      double phi = i*PI / latdivs;
      glm::vec3 pos(
          sin(phi) * cos(theta),
          sin(phi) * sin(theta),
          cos(phi)
      );
      m.add_vert(pos);
    }
  }

  m.add_vert(glm::vec3(0.0, 0.0, -1.0));

  // Top
  for (unsigned i = 0; i < londivs; ++i) {
    m.add_tri(0, i + 1, (i + 1) % londivs + 1);
  }

  // Bottom
  for (unsigned i = 0; i < londivs; ++i) {
    m.add_tri(
        m.verts_size() - 1,
        m.verts_size() - londivs - 1 + (i+1)%londivs,
        m.verts_size() - londivs - 1 + i
    );
  }

  for (unsigned i = 0; i < latdivs - 2; ++i) {
    for (unsigned j = 0; j < londivs; ++j) {
      size_t a = 1 + i*londivs + j;
      size_t b = 1 + (i+1)*londivs + j;
      size_t c = 1 + (i+1)*londivs + (j+1)%londivs;
      size_t d = 1 + i*londivs + (j+1)%londivs;

      m.add_quad(a, b, c, d);
    }
  }

  m.compute_vert_norms();

  return add_mesh(SPHERE_MESH_NAME, std::move(m));
}

Sphere::Sphere(const glm::vec3 &center, float radius) {
  Mesh::mesh_id mesh_id = get_mesh_id(SPHERE_MESH_NAME);
  if (mesh_id == Mesh::NONE) {
    mesh_id = sphere_mesh(8, 16);
  }

  _mesh_instance = MeshInstance(mesh_id);
  set_radius(radius);
  set_center(center);
}
