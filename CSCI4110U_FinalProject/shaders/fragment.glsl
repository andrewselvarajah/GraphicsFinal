#version 110

varying vec4 v_colour;

void main() {
  float ambientStrength = 0.5;
  vec4 ambient = ambientStrength * v_colour;

  vec4 result = ambient * v_colour;
  gl_FragColor = vec4(result);
}
