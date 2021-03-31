#version 330 core

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_uv;

out vec3 f_normal;
out vec2 f_uv;
out vec3 f_light_dir;

uniform mat4 u_view;
uniform mat4 u_proj;

void main() {
    gl_Position = u_proj * u_view * vec4(v_pos, 1.0);

    f_light_dir = normalize(vec3(0.0, 1.0, 0.0));
    f_normal = v_normal;
    f_uv = v_uv;
}
