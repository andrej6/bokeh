#version 330

in vec3 vpos;
in vec4 vcol;

uniform mat4 modelmat;
uniform mat4 viewmat;
uniform mat4 projmat;

out vec4 fcol;

void main() {
  mat4 MVP = projmat * viewmat * modelmat;
  gl_Position = MVP * vec4(vpos, 1.0);
  fcol = vcol;
}
