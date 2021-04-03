#version 330 core

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_uv;

out vec3 f_pos;
out vec3 f_normal;
out vec2 f_uv;

uniform mat4 u_view;
uniform mat4 u_proj;

void main() {
    gl_Position = u_proj * u_view * vec4(v_pos, 1.0);

    f_pos = v_pos;
    f_normal = v_normal;
    f_uv = v_uv;
}
