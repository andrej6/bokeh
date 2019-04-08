#include "shader_store.h"

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <cstdlib>

#include "util.h"

typedef uint32_t shader_id_t;

static shader_id_t next_shader_id = 1;
static shader_id_t next_program_id = 1;

struct ShaderInfo {
  GLuint index;
  GLenum type;
  std::string src;

  ShaderInfo() : index(0) {}

  ShaderInfo(ShaderInfo &&other) {
    index = other.index;
    type = other.type;
    src = std::move(other.src);

    other.index = 0;
  }

  ~ShaderInfo() {
    if (index != 0) {
      glDeleteShader(index);
      handle_gl_error("[~ShaderInfo] Deleting shader");
    }
  }
};

struct ProgramInfo {
  GLuint index;
  std::vector<shader_id_t> shaders;

  ProgramInfo() : index(0) {}

  ProgramInfo(ProgramInfo &&other) {
    index = other.index;
    shaders = std::move(other.shaders);

    other.index = 0;
  }

  ~ProgramInfo() {
    if (index != 0) {
      glDeleteProgram(index);
      handle_gl_error("[~ProgramInfo] Deleting program");
    }
  }
};

typedef std::unordered_map<std::string, shader_id_t> name_id_map_t;
typedef std::unordered_map<shader_id_t, ShaderInfo> id_shader_map_t;
typedef std::unordered_map<shader_id_t, ProgramInfo> id_program_map_t;

name_id_map_t shader_names;
id_shader_map_t shader_info;

name_id_map_t program_names;
id_program_map_t program_info;

bool add_shader_src_unchecked(shader_id_t id, const char *src, GLenum type) {
  ShaderInfo info;
  info.index = 0;
  info.type = type;
  info.src = std::string(src);

  shader_info.insert(std::make_pair(id, std::move(info)));
  return true;
}

bool add_shader_source_file(const char *shader_name, const char *filename, GLenum type) {
  if (shader_names.find(shader_name) != shader_names.end()) {
    return false;
  }

  std::ifstream src_file(filename);
  if (!src_file.good()) {
    glerr() << "Error opening shader source file " << filename << std::endl;
    exit(-1);
  }

  std::string src, line;
  while (std::getline(src_file, line)) {
    line.push_back('\n');
    src += line;
  }

  shader_names.insert(std::make_pair(std::string(shader_name), next_shader_id));
  ++next_shader_id;

  return add_shader_src_unchecked(next_shader_id - 1, src.c_str(), type);
}

bool add_shader_source_string(const char *shader_name, const char *src, GLenum type) {
  if (shader_names.find(shader_name) != shader_names.end()) {
    return false;
  }

  shader_names.insert(std::make_pair(std::string(shader_name), next_shader_id));
  ++next_shader_id;

  return add_shader_src_unchecked(next_shader_id - 1, src, type);
}

const char *shader_source(const char *shader_name) {
  name_id_map_t::iterator id_itr = shader_names.find(shader_name);
  if (id_itr == shader_names.end()) {
    return NULL;
  }

  ShaderInfo &info = shader_info.find(id_itr->second)->second;

  return info.src.c_str();
}

bool compile_shader(const char *shader_name) {
  name_id_map_t::iterator id_itr = shader_names.find(shader_name);
  if (id_itr == shader_names.end()) {
    return false;
  }

  ShaderInfo &info = shader_info.find(id_itr->second)->second;

  if (info.index == 0) {
    info.index = glCreateShader(info.type);
    handle_gl_error("[compile_shader] Creating shader object");

    const char *src = info.src.c_str();
    glShaderSource(info.index, 1, &src, NULL);
    handle_gl_error("[compile_shader] Adding shader source");

    glCompileShader(info.index);
    handle_shader_error(shader_name, info.index);
  }

  return true;
}

bool add_program(const char *program_name) {
  if (program_names.find(program_name) != program_names.end()) {
    return false;
  }

  shader_id_t id = next_program_id;
  ++next_program_id;

  program_names.insert(std::make_pair(std::string(program_name), id));
  
  ProgramInfo info;
  info.index = glCreateProgram();
  handle_gl_error("[add_program] Creating program object");

  program_info.insert(std::make_pair(id, std::move(info)));
  return true;
}

bool add_shader_to_program(const char *shader_name, const char *program_name) {
  name_id_map_t::iterator shader_itr = shader_names.find(shader_name);
  if (shader_itr == shader_names.end()) {
    return false;
  }

  name_id_map_t::iterator program_itr = program_names.find(program_name);
  if (program_itr == program_names.end()) {
    return false;
  }

  ShaderInfo &sinfo = shader_info.find(shader_itr->second)->second;
  if (sinfo.index == 0) {
    return false;
  }

  ProgramInfo &pinfo = program_info.find(program_itr->second)->second;

  glAttachShader(pinfo.index, sinfo.index);
  handle_gl_error("[add_shader_to_program] Attaching shader to program");

  pinfo.shaders.push_back(shader_itr->second);
  return true;
}

bool link_program(const char *program_name, bool delete_shaders) {
  name_id_map_t::iterator itr = program_names.find(program_name);
  if (itr == program_names.end()) {
    return false;
  }

  ProgramInfo &pinfo = program_info.find(itr->second)->second;
  glLinkProgram(pinfo.index);
  handle_program_error(program_name, pinfo.index);

  for (unsigned i = 0; i < pinfo.shaders.size(); ++i) {
    ShaderInfo &sinfo = shader_info.find(pinfo.shaders[i])->second;
    glDetachShader(pinfo.index, sinfo.index);
    handle_gl_error("[link_program] Detaching shader from program");

    if (delete_shaders) {
      glDeleteShader(sinfo.index);
      handle_gl_error("[link_program] Deleting shader");
      sinfo.index = 0;
    }
  }

  if (delete_shaders) {
    pinfo.shaders.clear();
  }

  return true;
}

GLuint shader_loc(const char *shader_name) {
  name_id_map_t::iterator itr = shader_names.find(shader_name);
  if (itr == shader_names.end()) {
    return 0;
  }

  ShaderInfo &sinfo = shader_info.find(itr->second)->second;
  return sinfo.index;
}

GLuint program_loc(const char *program_name) {
  name_id_map_t::iterator itr = program_names.find(program_name);
  if (itr == program_names.end()) {
    return 0;
  }

  ProgramInfo &pinfo = program_info.find(itr->second)->second;
  return pinfo.index;
}
