#version 330 core

struct AmbientLight {
    vec3 color;
};

in vec2 f_uv;

out vec4 color;

uniform sampler2D tex;
uniform AmbientLight ambient;

void main() {
    vec4 tex_color = texture(tex, f_uv);
    color = vec4(ambient.color, 1.0) * tex_color;
}
