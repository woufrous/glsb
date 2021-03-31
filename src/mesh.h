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

    void transform(glm::mat4 tmat) noexcept {
        pos = static_cast<glm::vec3>(tmat * glm::vec4(pos, 1.f));
        auto norm_mat = glm::inverseTranspose(glm::mat3(tmat));
        norm = static_cast<glm::vec3>(norm_mat * glm::vec4(norm, 1.f));
    }

    static std::vector<VertexDescriptor> get_vertex_desc() noexcept {
        return std::vector<VertexDescriptor>{
            {"v_pos", 3, sizeof(Vertex), offsetof(Vertex, pos), true},
            {"v_normal", 3, sizeof(Vertex), offsetof(Vertex, norm), true},
            {"v_uv", 2, sizeof(Vertex), offsetof(Vertex, uv), true},
        };
    }
};

struct FlatVertex {
    glm::vec3 pos;
    glm::vec4 color;

    void transform(glm::mat4 tmat) noexcept {
        pos = static_cast<glm::vec3>(tmat * glm::vec4(pos, 1.f));
    }

    static std::vector<VertexDescriptor> get_vertex_desc() noexcept {
        return std::vector<VertexDescriptor>{
            {"v_pos", 3, sizeof(FlatVertex), offsetof(FlatVertex, pos), true},
            {"v_color", 4, sizeof(FlatVertex), offsetof(FlatVertex, color), true},
        };
    }
};

template <typename VertexT>
struct Mesh {
    using vertex_type = VertexT;

    std::vector<vertex_type> vertex_data;
    std::vector<uint32_t> index_data;

    Mesh<vertex_type>& transform(glm::mat4 tmat) noexcept {
        for (auto& vert : vertex_data) {
            vert.transform(tmat);
        }
        return *this;
    }
};

Mesh<Vertex> generate_quad(float xscale, float yscale) {
    auto x_half = xscale/2.f;
    auto y_half = yscale/2.f;
    return Mesh<Vertex>{
        {
            {{-x_half, -y_half, 0.f}, {0.f, 0.f, 1.0f}, {0.0f, 0.0f}},
            {{ x_half, -y_half, 0.f}, {0.f, 0.f, 1.0f}, {1.0f, 0.0f}},
            {{ x_half,  y_half, 0.f}, {0.f, 0.f, 1.0f}, {1.0f, 1.0f}},
            {{-x_half,  y_half, 0.f}, {0.f, 0.f, 1.0f}, {0.0f, 1.0f}},
        },
        {0, 1, 2, 2, 3, 0}
    };
}

Mesh<Vertex> load_obj(const std::filesystem::path& fpath) {
    auto attrib = tinyobj::attrib_t{};
    auto shapes = std::vector<tinyobj::shape_t>{};
    auto materials = std::vector<tinyobj::material_t>{};
    auto err = std::string{};

    auto ifs = std::ifstream(fpath);

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &ifs)) {
        throw GLSBError(("Error loading obj file: "s + err).c_str());
    }

    auto mesh = Mesh<Vertex>{};

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
