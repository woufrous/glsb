#pragma once

#include <filesystem>
#include <fstream>
#include <string>
using namespace std::string_literals;
#include <vector>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "renderer.h"
#include "shader.h"
#include "utils.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 uv;

    static std::vector<VertexDescriptor> get_vertex_desc() noexcept {
        return std::vector<VertexDescriptor>{
            {"pos", 3, sizeof(Vertex), offsetof(Vertex, pos), true},
            {"normal", 3, sizeof(Vertex), offsetof(Vertex, norm), true},
            {"vert_uv", 2, sizeof(Vertex), offsetof(Vertex, uv), true},
        };
    }
};

struct Mesh {
    std::vector<Vertex> vertex_data;
    std::vector<uint32_t> index_data;

    void transform(glm::mat4 tmat) noexcept {
        for (auto& vert : vertex_data) {
            vert.pos = static_cast<glm::vec3>(tmat * glm::vec4(vert.pos, 1.f));
            vert.norm = static_cast<glm::vec3>(tmat * glm::vec4(vert.norm, 1.f));
        }
    }
};

Mesh generate_quad(float xscale, float yscale) {
    auto x_half = xscale/2.f;
    auto y_half = yscale/2.f;
    return Mesh{
        {
            {{-x_half, -y_half, 0.f}, {0.f, 0.f, 1.0f}, {0.0f, 0.0f}},
            {{ x_half, -y_half, 0.f}, {0.f, 0.f, 1.0f}, {1.0f, 0.0f}},
            {{ x_half,  y_half, 0.f}, {0.f, 0.f, 1.0f}, {1.0f, 1.0f}},
            {{-x_half,  y_half, 0.f}, {0.f, 0.f, 1.0f}, {0.0f, 1.0f}},
        },
        {0, 1, 2, 2, 3, 0}
    };
}

Mesh load_obj(const std::filesystem::path& fpath) {
    auto attrib = tinyobj::attrib_t{};
    auto shapes = std::vector<tinyobj::shape_t>{};
    auto materials = std::vector<tinyobj::material_t>{};
    auto err = std::string{};

    auto ifs = std::ifstream(fpath);

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &ifs)) {
        throw GLSBError(("Error loading obj file: "s + err).c_str());
    }

    auto mesh = Mesh{};

    for (const auto& shape : shapes) {
        for (const auto& idx : shape.mesh.indices) {
            mesh.vertex_data.emplace_back(Vertex{
                {
                    attrib.vertices[(idx.vertex_index*3) + 0],
                    attrib.vertices[(idx.vertex_index*3) + 1],
                    attrib.vertices[(idx.vertex_index*3) + 2],
                }, {
                    attrib.normals[(idx.normal_index*3) + 0],
                    attrib.normals[(idx.normal_index*3) + 1],
                    attrib.normals[(idx.normal_index*3) + 2],
                }, {
                    attrib.texcoords[(idx.texcoord_index*2) + 0],
                    1.f - attrib.texcoords[(idx.texcoord_index*2) + 1],
                }
            });
            mesh.index_data.push_back(mesh.index_data.size());
        }
    }

    return mesh;
}
