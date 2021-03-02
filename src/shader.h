#pragma once

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

        Program(const std::vector<Shader>& shaders, const std::vector<std::pair<uint32_t, const char*>> attribs) : prog_{0, glDeleteProgram} {
            prog_ = UniqueProgramHandle(glCreateProgram(), glDeleteProgram);
            for (const auto& shader : shaders) {
                glAttachShader(prog_.get(), shader.shader_.get());
            }
            for (const auto& [idx, name] : attribs) {
                glBindAttribLocation(prog_.get(), idx, name);
            }
            glBindFragDataLocation(prog_.get(), 0, "color");
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

        void use() const noexcept {
            glUseProgram(prog_.get());
        }
    private:
        UniqueProgramHandle prog_;
};
