#include "shader.h"

void
Program::link_program(const std::vector<Shader>& shaders) {
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

void
Shader::compile_shader(Shader::Type type, const char* source) {
    shader_ = UniqueShaderHandle(
        glCreateShader(static_cast<std::underlying_type_t<Type>>(type)),
        glDeleteShader
    );
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
