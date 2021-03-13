#pragma once

#include <cstdint>
#include <vector>

#define GLFW_INCLUDE_GL_3
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "buffer.h"

struct Vertex {
    glm::vec2 pos;
    glm::vec2 uv;
};

struct Mesh {
    std::vector<Vertex> vertex_data;
    std::vector<uint32_t> index_data;
};

class Renderer {
    public:
        using handle_type = size_t;

        void init() {
            glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        void cleanup() {}

        handle_type upload_mesh(const Mesh& mesh) {
            GLuint vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            auto vbo = Buffer<BufferType::Array>{};
            vbo.bind();
            vbo.set_data(
                mesh.vertex_data.data(),
                mesh.vertex_data.size()*sizeof(Vertex),
                GL_STATIC_DRAW
            );

            auto ibo = Buffer<BufferType::ElementArray>{};
            ibo.bind();
            ibo.set_data(
                mesh.index_data.data(),
                mesh.index_data.size()*sizeof(uint32_t),
                GL_STATIC_DRAW
            );

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
            glEnableVertexAttribArray(1);

            // TODO: locking
            meshes_.emplace_back(vao, std::move(vbo), std::move(ibo), mesh.index_data.size());
            auto ret_idx = meshes_.size()-1;

            return ret_idx;
        }

        void render(handle_type mesh_hndl) const {
            glDrawElements(GL_TRIANGLES, meshes_[mesh_hndl].ibo_size, GL_UNSIGNED_INT, 0);
        }
    private:
        struct mesh_handle {
            uint32_t vao;
            Buffer<BufferType::Array> vbo;
            Buffer<BufferType::ElementArray> ibo;
            size_t ibo_size;
        };
        std::vector<mesh_handle> meshes_;
};
