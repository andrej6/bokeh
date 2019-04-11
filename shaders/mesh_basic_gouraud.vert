#version 330

in vec3 vpos;
in vec3 vnorm;

in vec4 vdiffuse;
in vec4 vspecular;
in vec4 vambient;
in float vshiny;

uniform mat4 modelmat;
uniform mat4 viewmat;
uniform mat4 projmat;

out vec3 fpos;
out vec3 fnorm;

out vec4 fdiffuse;
out vec4 fspecular;
out vec4 fambient;
out float fshiny;

void main() {
  gl_Position = projmat * viewmat * modelmat * vec4(vpos, 1.0);

  fpos = vpos;
  fnorm = vnorm;

  fdiffuse = vdiffuse;
  fspecular = vspecular;
  fambient = vambient;
  fshiny = vshiny;
}
