#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

#define GLFW_INCLUDE_GL_3
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "buffer.h"
#include "mesh.h"
#include "shader.h"

template <typename NumT>
struct Extent2D {
    NumT width;
    NumT height;
};

class Renderer {
    public:
        using handle_type = size_t;

        Renderer(GLFWwindow* win) : win_{win} {}

        void init() {
            glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_DEPTH_TEST);
            glCullFace(GL_BACK);
            glEnable(GL_CULL_FACE);
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

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, norm));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
            glEnableVertexAttribArray(2);

            // TODO: locking
            meshes_.emplace_back(vao, std::move(vbo), std::move(ibo), mesh.index_data.size());
            auto ret_idx = meshes_.size()-1;

            return ret_idx;
        }

        void render(handle_type mesh_hndl) const {
            glDrawElements(GL_TRIANGLES, meshes_[mesh_hndl].ibo_size, GL_UNSIGNED_INT, 0);
        }

        Extent2D<int> get_viewport_dim() const noexcept {
            auto ret = Extent2D<int>{};
            glfwGetFramebufferSize(win_, &(ret.width), &(ret.height));
            return ret;
        }

        void clear_screen() const noexcept {
            auto fb = get_viewport_dim();
            assert((fb.width >0) && (fb.height > 0));
            glViewport(0, 0, fb.width, fb.height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        ShaderManager& shader_manager() {
            return shader_manager_;
        }

        const ShaderManager& shader_manager() const {
            return shader_manager_;
        }

    private:
        GLFWwindow* win_;

        struct mesh_handle {
            uint32_t vao;
            Buffer<BufferType::Array> vbo;
            Buffer<BufferType::ElementArray> ibo;
            size_t ibo_size;
        };
        std::vector<mesh_handle> meshes_;
        ShaderManager shader_manager_;
};
