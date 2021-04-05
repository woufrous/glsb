#version 330 core

in vec3 v_pos;
in vec4 v_color;

out vec4 f_color;

uniform mat4 u_view;
uniform mat4 u_proj;

void main() {
    gl_Position = u_proj * u_view * vec4(v_pos, 1.0);
    f_color = v_color;
}
