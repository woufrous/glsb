#version 330 core

struct AmbientLight {
    vec3 color;
};

in vec3 f_normal;
in vec2 f_uv;
in vec3 f_light_dir;

out vec4 color;

uniform sampler2D tex;
uniform AmbientLight ambient;

void main() {
    vec4 tex_color = texture(tex, f_uv);

    float mu = max(0, dot(f_light_dir, f_normal));
    vec3 diffuse_color = mu * vec3(1.0, 0.0, 0.0);

    color = vec4(ambient.color + diffuse_color, 1.0) * tex_color;
}
