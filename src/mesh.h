#pragma once

#include <vector>

#include "renderer.h"
#include "utils.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 uv;
};

struct Mesh {
    std::vector<Vertex> vertex_data;
    std::vector<uint32_t> index_data;
};
