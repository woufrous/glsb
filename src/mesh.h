#pragma once

#include <filesystem>
#include <fstream>
#include <string>
using namespace std::string_literals;
#include <vector>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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
