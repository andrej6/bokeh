#version 330

in vec3 fpos;
in vec3 fnorm;

uniform vec4 diffuse;
uniform vec4 specular;
uniform vec4 ambient;
uniform float shiny;

uniform mat4 modelmat;
uniform mat4 viewmat;

uniform vec3 lightpos;
uniform vec3 lightdiffuse;
uniform vec3 lightspecular;
uniform vec3 lightambient;
uniform float lightpower;

void main() {
  mat4 MV = viewmat * modelmat;

  vec4 homog = MV * vec4(fpos, 1.0);
  vec3 fpos_cam = homog.xyz / homog.w;

  homog = viewmat * vec4(lightpos, 1.0);
  vec3 lightpos_cam = homog.xyz / homog.w;

  vec3 fnorm_cam = normalize((MV * vec4(fnorm, 0.0)).xyz);

  float dist = length(lightpos_cam - fpos_cam);
  float dist_div = 1.0 / (dist*dist + 1);

  vec3 eye_dir = normalize(-fpos_cam);
  vec3 light_dir = normalize(lightpos_cam - fpos_cam);
  vec3 reflect_dir = normalize(2*dot(light_dir, fnorm_cam) * fnorm_cam - light_dir);

  vec4 fambient = vec4(lightambient, 1.0) * ambient;

  float dp = max(dot(light_dir, fnorm_cam), 0.0);
  vec4 fdiffuse = diffuse * dp * vec4(lightdiffuse, 1.0) * dist_div;

  dp = max(dot(reflect_dir, eye_dir), 0.0);
  vec4 fspecular = specular * pow(dp, shiny) * vec4(lightspecular, 1.0) * dist_div;

  gl_FragColor = fambient + lightpower*(fdiffuse + fspecular);
}
