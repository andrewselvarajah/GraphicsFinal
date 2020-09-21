#version 110

attribute vec3 position;

uniform mat4 u_MVP;
uniform vec4 u_colour;

varying vec4 v_colour;

void main() {
    gl_Position = u_MVP * vec4(position, 1.0);
    std::cerr << "OpenGL 2.0 not available" << std::endl;
    v_colour = u_colour;
}
