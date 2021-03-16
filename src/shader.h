#pragma once

#include <optional>
#include <string>
#include <vector>

#include <GL/glew.h>
#define GLFW_INCLUDE_GL_3
#include <GLFW/glfw3.h>

#include "utils.h"

class Shader {
    public:
        using UniqueShaderHandle = UniqueHandle<GLuint, decltype(glDeleteShader)>;

        enum class Type : GLenum {
            Vertex = GL_VERTEX_SHADER,
            Fragment = GL_FRAGMENT_SHADER,
        };

        friend class Program;

        Shader(Type type, const char* source) : Shader(static_cast<std::underlying_type_t<Type>>(type), source) {}
        Shader(GLenum type, const char* source) : shader_{0, glDeleteShader} {
            shader_ = UniqueShaderHandle(glCreateShader(type), glDeleteShader);
            glShaderSource(shader_.get(), 1, &source, nullptr);
            glCompileShader(shader_.get());

            GLint status;
            glGetShaderiv(shader_.get(), GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE) {
                GLsizei len;
                char buf[1024];
                glGetShaderInfoLog(shader_.get(), sizeof(buf), &len, buf);

                throw GLSBError(buf);
            }
        }
    private:
        UniqueShaderHandle shader_;
};


class Program {
    public:
        using UniqueProgramHandle = UniqueHandle<GLuint, decltype(glDeleteProgram)>;

        Program() : prog_{0, glDeleteProgram} {}
        Program(const std::vector<Shader>& shaders) : Program{} {
            prog_ = UniqueProgramHandle(glCreateProgram(), glDeleteProgram);
            for (const auto& shader : shaders) {
                glAttachShader(prog_.get(), shader.shader_.get());
            }
            glLinkProgram(prog_.get());

            GLint status;
            glGetProgramiv(prog_.get(), GL_LINK_STATUS, &status);
            if (status != GL_TRUE) {
                GLsizei len;
                char buf[1024];
                glGetProgramInfoLog(prog_.get(), sizeof(buf), &len, buf);

                throw GLSBError(buf);
            }
        }

        std::optional<GLuint> get_attrib_location(const char* name) {
            auto pos = glGetAttribLocation(prog_.get(), name);
            if ((pos == -1) || (pos == GL_INVALID_OPERATION)) {
                return std::nullopt;
            }
            return pos;
        }

        void set_uniform(const char* name, const glm::vec3& val) const {
            auto loc = glGetUniformLocation(prog_.get(), name);
            if (loc == GL_INVALID_VALUE) {
                throw GLSBError("Uniform not found");
            }
            glUniform3f(loc, val[0], val[1], val[2]);
        }

        void set_uniform(const char* name, const glm::vec4& val) const {
            auto loc = glGetUniformLocation(prog_.get(), name);
            if (loc == GL_INVALID_VALUE) {
                throw GLSBError("Uniform not found");
            }
            glUniform4f(loc, val[0], val[1], val[2], val[3]);
        }

        void set_uniform(const char* name, const glm::mat4& val) const {
            auto loc = glGetUniformLocation(prog_.get(), name);
            if (loc == GL_INVALID_VALUE) {
                throw GLSBError("Uniform not found");
            }
            glUniformMatrix4fv(loc, 1, GL_FALSE, &val[0][0]);
        }

        void use() const noexcept {
            glUseProgram(prog_.get());
        }
    private:
        UniqueProgramHandle prog_;
};
