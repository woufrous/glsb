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

struct Specularity {
    float roughness;
    float intensity;
};

struct Camera {
    vec3 pos;
};

in vec3 f_pos;
in vec3 f_normal;
in vec2 f_uv;

out vec4 color;

uniform sampler2D tex;
uniform AmbientLight ambient;
uniform Light diffuse;
uniform Camera camera;
uniform Specularity spec;

void main() {
    vec4 tex_color = texture(tex, f_uv);

    vec3 light_dir = normalize(diffuse.pos - f_pos);
    vec3 view_vector = normalize(f_pos - camera.pos);

    vec3 refl = reflect(view_vector, f_normal);
    float spec_factor = pow(max(dot(refl, light_dir), 0), spec.roughness);
    float mu = max(0, dot(light_dir, f_normal));
    color = \
        vec4(
            spec.intensity * spec_factor * ambient.color +
            ambient.intensity * ambient.color +
            mu * diffuse.intensity * diffuse.color,
            1.0
        ) * tex_color;
}
