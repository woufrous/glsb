#version 330 core

struct AmbientLight {
    vec3 color;
};

struct DiffuseLight {
    vec3 dir;
    vec3 color;
};

in vec3 f_normal;
in vec2 f_uv;

out vec4 color;

uniform sampler2D tex;
uniform AmbientLight ambient;
uniform DiffuseLight diffuse;

void main() {
    vec4 tex_color = texture(tex, f_uv);

    float mu = max(0, dot(diffuse.dir, f_normal));
    color = vec4(ambient.color + mu*diffuse.color, 1.0) * tex_color;
}
