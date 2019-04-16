#version 330

in vec3 vpos;
in vec3 vnorm;

uniform mat4 modelmat;
uniform mat4 viewmat;
uniform mat4 projmat;

out vec3 fpos;
out vec3 fnorm;

void main() {
  gl_Position = projmat * viewmat * modelmat * vec4(vpos, 1.0);

  fpos = vpos;
  fnorm = vnorm;
}
