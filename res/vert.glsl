#version 330 core

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_uv;

out vec2 f_uv;

uniform mat4 u_mvp;

void main() {
    gl_Position = u_mvp*vec4(v_pos, 1.0);
    f_uv = v_uv;
}
