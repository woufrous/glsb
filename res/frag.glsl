#version 330 core

struct AmbientLight {
    vec3 color;
    float intensity;
};

struct DiffuseLight {
    vec3 dir;
    vec3 color;
    float intensity;
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
    color = \
        vec4(
            ambient.intensity * ambient.color +
            mu * diffuse.intensity * diffuse.color,
            1.0
        ) * tex_color;
}
