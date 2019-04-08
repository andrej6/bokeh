#ifndef SHADER_STORE_H_
#define SHADER_STORE_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Add shader source to the global store, under the given name, if a shader with
// that name did not already exist in the store.
//
// `type` indicates the type of shader being added, and should be one of the values
// accepted by glCreateShader(). See OpenGL documentation for further details.
//
// Returns true if the shader source did not already exist and was created; returns
// false if an existing shader with the same name exists, and nothing was changed.
bool add_shader_source_file(const char *shader_name, const char *filename, GLenum type);
bool add_shader_source_string(const char *shader_name, const char *src, GLenum type);

// Return the shader source with the given name, or NULL if it does not exist
// in the store.
const char *shader_source(const char *shader_name);

// Compile the given shader for the current context, if it has not been compiled
// already. Returns true if the shader was compiled or has already been compiled;
// returns false if a shader with the given name does not exist in the store.
bool compile_shader(const char *shader_name);

// Add a new program with the given name to the store, if a program with the same
// name does not already exist. Returns true if the program was added, and false
// otherwise.
//
// The new program is entirely empty; it must have shaders added and linked before
// it is usable.
bool add_program(const char *program_name);

// Add the given (compiled) shader to the given program, for subsequent linking.
// Returns true if both shader and program names exist in the store, and the
// shader was added successfully. Returns false otherwise.
bool add_shader_to_program(const char *shader_name, const char *program_name);

// Link the program with the given name. If `delete_shaders` is true, also
// delete the shaders associated with that program.
//
// Returns true if the program exists and links successfully; returns false
// otherwise.
bool link_program(const char *program_name, bool delete_shaders = true);

// Return the OpenGL index of the shader with the given name, or 0 if it does
// not exist in the store or has not yet been compiled for the current context.
GLuint shader_loc(const char *shader_name);

// Return the OpenGL index of the program with the given name, or 0 if it does
// not exist in the store or has not yet been linked for the current context.
GLuint program_loc(const char *program_name);

#endif /* SHADER_STORE_H_ */
