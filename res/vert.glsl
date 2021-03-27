#version 330 core

in vec3 pos;
in vec3 normal;
in vec2 vert_uv;

out vec2 uv;
out float mu;

uniform mat4 u_mvp;
uniform Light {
    vec3 pos;
    vec3 color;
} light;

void main() {
    gl_Position = u_mvp*vec4(pos, 1.0);
    uv = vert_uv;
    vec3 rel_light = light.pos-pos;
    mu = dot(normal, rel_light)/(length(rel_light)*length(normal));
}
