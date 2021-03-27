#version 330 core

in vec2 uv;
in float mu;
out vec4 color;
uniform sampler2D tex;

void main() {
    vec4 tex_color = texture(tex, uv);
    color = tex_color;
}
