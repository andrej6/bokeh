#include "lens_assembly.h"

#include <fstream>
#include <string>

#include "util.h"


LensAssembly *LensAssembly::from_la(const char *filename) {
  std::string dirs = dirname(filename);

  std::ifstream scnfile(filename);
  assert(scnfile.good());

  LensAssembly *lens_assembly = new LensAssembly();
  std::string line;

  float z = 0.0;
  glm::vec3 c(0, 0, 0);
  float r, t, n, a;

  while (std::getline(scnfile, line)) {
    line = strip(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    auto tokens = split(line);

    if (tokens[0] == "lens_assembly") { // ======== LENS ASSEMBLY ==================
      if (tokens.size() != 2) {
        glerr() << "ERROR: incorrect number of arguments for lens_assembly" << std::endl;
        exit(-1);
      }

      lens_assembly->set_dist(parse_float(tokens, 1));

    } else if (tokens[0] == "lens_surface") { // ======== LENS SURFACE ==================
      if (tokens.size() != 5) {
        glerr() << "ERROR: incorrect number of arguments for lens_surface" << std::endl;
        exit(-1);
      }

      r = parse_float(tokens, 1);
      t = parse_float(tokens, 2);
      n = parse_float(tokens, 3);
      a = parse_float(tokens, 4);

      c.x = z + r;
      z -= t;

      lens_assembly->add_surface(LensSurface(c, abs(r), n, a / 2.0));

    }
  }
}