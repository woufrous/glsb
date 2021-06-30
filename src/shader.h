#pragma once

#include <optional>
#include <string>
using namespace std::string_literals;
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <GL/glew.h>
#define GLFW_INCLUDE_GL_3
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include "utils.h"

struct VertexDescriptor {
    const char* name;
    uint32_t count;
    uint32_t stride;
    uintptr_t offset;
    bool is_normalized;
};

class Shader {
    public:
        using UniqueShaderHandle = UniqueHandle<GLuint, decltype(glDeleteShader)>;

        enum class Type : GLenum {
            Vertex = GL_VERTEX_SHADER,
            Fragment = GL_FRAGMENT_SHADER,
        };

        Shader(Type type, const char* source) : shader_{0, glDeleteShader} {
            compile_shader(type, source);
        }
    private:
        void compile_shader(Type type, const char* source);

        UniqueShaderHandle shader_;

        friend class Program;
};


class Program {
    public:
        using UniqueProgramHandle = UniqueHandle<GLuint, decltype(glDeleteProgram)>;

        Program(const std::vector<Shader>& shaders) : prog_{0, glDeleteProgram} {
            link_program(shaders);
        }

        std::optional<GLuint> get_attrib_location(const char* name) const noexcept {
            auto pos = glGetAttribLocation(prog_.get(), name);
            if ((pos == -1) || (pos == GL_INVALID_OPERATION)) {
                return std::nullopt;
            }
            assert(pos >= 0);
            return pos;
        }

        bool set_attrib_pointer(const VertexDescriptor& desc) const {
            auto pos = get_attrib_location(desc.name);
            if (!pos) {
                spdlog::info("trying to bind unkown attribute \"{}\"", desc.name);
                return false;
            }
            assert(desc.count < INT_MAX);
            assert(desc.stride < INT_MAX);
            glVertexAttribPointer(
                *pos,
                static_cast<GLint>(desc.count),
                GL_FLOAT,
                desc.is_normalized ? GL_TRUE : GL_FALSE,
                static_cast<GLsizei>(desc.stride),
                reinterpret_cast<void*>(desc.offset));
            glEnableVertexAttribArray(*pos);
            return true;
        }

        std::optional<GLint> get_uniform_location(const char* name) const noexcept {
            auto pos = glGetUniformLocation(prog_.get(), name);
            if ((pos == -1) || (pos == GL_INVALID_OPERATION) || pos == GL_INVALID_VALUE) {
                return std::nullopt;
            }
            return pos;
        }

        bool set_uniform(const char* name, float val) const noexcept {
            auto loc = get_uniform_location(name);
            if (!loc) {
                spdlog::info("trying to set unknown uniform \"{}\"", name);
                return false;
            }
            glUniform1f(*loc, val);
            return true;
        }

        bool set_uniform(const char* name, const glm::vec3& val) const noexcept {
            auto loc = get_uniform_location(name);
            if (!loc) {
                spdlog::info("trying to set unknown uniform \"{}\"", name);
                return false;
            }
            glUniform3f(*loc, val[0], val[1], val[2]);
            return true;
        }

        bool set_uniform(const char* name, const glm::vec4& val) const noexcept {
            auto loc = get_uniform_location(name);
            if (!loc) {
                spdlog::info("trying to set unknown uniform \"{}\"", name);
                return false;
            }
            glUniform4f(*loc, val[0], val[1], val[2], val[3]);
            return true;
        }

        bool set_uniform(const char* name, const glm::mat4& val) const noexcept {
            auto loc = get_uniform_location(name);
            if (!loc) {
                spdlog::info("trying to set unknown uniform \"{}\"", name);
                return false;
            }
            glUniformMatrix4fv(*loc, 1, GL_FALSE, &val[0][0]);
            return true;
        }

        void use() const noexcept {
            glUseProgram(prog_.get());
        }
    private:
        void link_program(const std::vector<Shader>& shaders);

        UniqueProgramHandle prog_;
};

class ShaderManager {
    public:
        Program& add_shader(const std::string& name, const std::vector<Shader>& shaders) {
            auto it = shaders_.try_emplace(name, shaders).first;
            return (*it).second;
        }
        Program& add_shader(const std::string& name, Program&& prog) {
            auto it = shaders_.insert(std::make_pair(name, std::move(prog))).first;
            return (*it).second;
        }

        Program& get_shader(const std::string& name) {
            return shaders_.at(name);
        }

        const Program& get_shader(const std::string& name) const {
            return shaders_.at(name);
        }
    private:
        std::unordered_map<std::string, Program> shaders_;
};
