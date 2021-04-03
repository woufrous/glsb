#version 330 core

struct AmbientLight {
    vec3 color;
    float intensity;
};

struct Light {
    vec3 pos;
    vec3 color;
    float intensity;
};

in vec3 f_pos;
in vec3 f_normal;
in vec2 f_uv;

out vec4 color;

uniform sampler2D tex;
uniform AmbientLight ambient;
uniform Light diffuse;

void main() {
    vec4 tex_color = texture(tex, f_uv);

    vec3 light_dir = normalize(diffuse.pos - f_pos);
    float mu = max(0, dot(light_dir, f_normal));
    color = \
        vec4(
            ambient.intensity * ambient.color +
            mu * diffuse.intensity * diffuse.color,
            1.0
        ) * tex_color;
}
