#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <cstdlib>
#include <cstring>

#include "canvas.h"
#include "bokeh_canvas.h"
#include "util.h"

#include "mesh.h"

static const char *USAGE =
"Usage: bokeh [options] <scene file>\n"
"\n"
"Options:\n"
"  -w<width>   --width <width>             Set the width of the window.\n"
"  -h<height>  --height <height>           Set the height of the window.\n"
"  -s<num>     --shadow-samples <num>      Set the number of shadow samples.\n"
"  -a<num>     --antialias-samples <num>   Set the number of antialias samples.\n"
"  -d<num>     --ray-depth <num>           Set the maximum raytree depth.\n"
"  -h          --help                      Display this text and exit.\n"
;

void usage(std::ostream &out, int code) {
  out << USAGE;
  out.flush();
  exit(code);
}

bool parse_long_opt_uint(int argc, char **argv, const char *name, int *i, unsigned *dest) {
  if (strncmp(argv[*i], "--", 2) != 0) {
    return false;
  }

  if (strcmp(argv[*i] + 2, name) != 0) {
    return false;
  }

  if (*i + 1 >= argc) {
    std::cerr << "ERROR: missing argument to option --" << name << std::endl;
    usage(std::cerr, 2);
  }

  char *val = argv[*i + 1];
  if (sscanf(val, "%u", dest) != 1) {
    std::cerr << "ERROR: invalid argument " << val << " to option --" << name << std::endl;
    usage(std::cerr, 2);
  }

  *i += 2;
  return true;
}

bool parse_short_opt_uint(int argc, char **argv, char name, int *i, unsigned *dest) {
  if (argv[*i][0] != '-' || argv[*i][1] != name) {
    return false;
  }

  char *val;
  if (argv[*i][2] == 0) {
    if (*i + 1 >= argc) {
      std::cerr << "ERROR: missing argument to option -" << name << std::endl;
      usage(std::cerr, 2);
    }

    val = argv[*i + 1];
    *i += 2;
  } else {
    val = argv[*i] + 2;
    *i += 1;
  }

  if (sscanf(val, "%u", dest) != 1) {
    std::cerr << "ERROR: invalid argument " << val << " to option -" << name << std::endl;
    usage(std::cerr, 2);
  }

  return true;
}

int main(int argc, char **argv) {
  BokehCanvasConf conf;
  conf.width = conf.height = 200;
  conf.shadow_samples = 10;
  conf.antialias_samples = 1;
  conf.num_bounces = 1;

  int i = 1;
  while (i < argc) {
    if (argv[i][0] != '-') {
      break;
    }

    if (argv[i][1] == '-') {
      if (parse_long_opt_uint(argc, argv, "width", &i, &conf.width)) continue;
      if (parse_long_opt_uint(argc, argv, "height", &i, &conf.height)) continue;
      if (parse_long_opt_uint(argc, argv, "shadow-samples", &i, &conf.shadow_samples)) continue;
      if (parse_long_opt_uint(argc, argv, "antialias-samples", &i, &conf.antialias_samples)) continue;
      if (parse_long_opt_uint(argc, argv, "ray-depth", &i, &conf.num_bounces)) continue;
      if (strcmp(argv[i], "--help") == 0) {
        usage(std::cout, 0);
      }

      std::cerr << "ERROR: unrecognized option " << argv[i] << std::endl;
      usage(std::cerr, 2);
    } else if (argv[i][1] == 0) {
      std::cerr << "ERROR: stray '-' character in command line" << std::endl;
      usage(std::cerr, 2);
    } else {
      if (parse_short_opt_uint(argc, argv, 'w', &i, &conf.width)) continue;
      if (parse_short_opt_uint(argc, argv, 'h', &i, &conf.height)) continue;
      if (parse_short_opt_uint(argc, argv, 's', &i, &conf.shadow_samples)) continue;
      if (parse_short_opt_uint(argc, argv, 'a', &i, &conf.antialias_samples)) continue;
      if (parse_short_opt_uint(argc, argv, 'd', &i, &conf.num_bounces)) continue;
      if (argv[i][1] == 'h' && argv[i][2] == 0) {
        usage(std::cout, 0);
      }

      std::cerr << "ERROR: unrecognized option " << argv[i] << std::endl;
      usage(std::cerr, 2);
    }
  }

  if (i >= argc) {
    std::cerr << "ERROR: no input file given" << std::endl;
    usage(std::cerr, 2);
  }

  conf.scnfile = argv[i];

  if (i + 1 != argc) {
    std::cerr << "ERROR: trailing options after filename" << std::endl;
    usage(std::cerr, 2);
  }

  BokehCanvas canvas(conf);
  canvas.make_active();

  Canvas::run();

  return 0;
}
