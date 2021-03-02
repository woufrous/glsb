#include <iostream>

#define GLFW_INCLUDE_GL_3
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include "shader.h"
#include "buffer.h"

static void glfw_error_cb(int error, const char* msg) {
    spdlog::error("GLFW error ({:d}): {}", error, msg);
}

#ifndef NDEBUG
static void gl_error_cb(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar *message,
        const void *userParam) {
    auto level = spdlog::level::critical;
    switch (severity) {
        case GL_DEBUG_SEVERITY_LOW:
        case GL_DEBUG_SEVERITY_NOTIFICATION: {
            level = spdlog::level::info;
        } break;
        case GL_DEBUG_SEVERITY_MEDIUM: {
            level = spdlog::level::warn;
        } break;
        case GL_DEBUG_SEVERITY_HIGH: {
            level = spdlog::level::err;
        } break;
    }
    spdlog::log(level, "OpenGL: {}", message);
}
#endif // NDEBUG

int main() {
    if (!glfwInit()) {
        spdlog::critical("Error initializing GLFW");
        return 1;
    }

    glfwSetErrorCallback(glfw_error_cb);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // NDEBUG
    auto win = glfwCreateWindow(800, 600, "OpenGL sandbox", nullptr, nullptr);
    if (!win) {
        spdlog::error("Error creating window");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(win);

    if (auto res = glewInit(); res != GLEW_OK) {
        spdlog::critical("Error initalizing GLEW: {}", glewGetErrorString(res));
        glfwDestroyWindow(win);
        glfwTerminate();
        return 1;
    }

    spdlog::info("using OpenGL {}", glGetString(GL_VERSION));
    spdlog::info("GLSL version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));
    spdlog::info("renderer: {}", glGetString(GL_RENDERER));

#ifndef NDEBUG
    glDebugMessageCallback(gl_error_cb, nullptr);
#endif // NDEBUG

    const char* vert_src = \
    "#version 330 core\n"
    ""
    "in vec2 pos;"
    ""
    "void main() {"
    "    gl_Position = vec4(pos.xy, 0.0, 1.0);"
    "}";

    const char* frag_src = \
    "#version 330 core\n"
    ""
    "out vec4 color;"
    ""
    "void main() {"
    "    color = vec4(1.0, 1.0, 1.0, 1.0);"
    "}";

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    struct Vertex {
        glm::vec2 pos;
    };

    auto vertices = std::vector<Vertex>{
        {{-0.5f, 0.5f}},
        {{0.5f, 0.5f}},
        {{0.5f, -0.5f}},
        {{-0.5f, -0.5f}},
    };
    auto indices = std::vector<uint32_t>{
        0, 1, 2, 3, 0, 2
    };

    auto vertex_buffer = Buffer<BufferType::Array>{};
    vertex_buffer.bind();
    vertex_buffer.set_data(vertices.data(), vertices.size()*sizeof(Vertex), GL_STATIC_DRAW);

    auto index_buffer = Buffer<BufferType::ElementArray>{};
    index_buffer.bind();
    index_buffer.set_data(indices.data(), indices.size()*sizeof(uint32_t), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);

    try {
        auto shaders = std::vector<Shader>{};
        shaders.emplace_back(Shader::Type::Vertex, vert_src);
        shaders.emplace_back(Shader::Type::Fragment, frag_src);

        auto attribs = std::vector<std::pair<uint32_t, const char*>>{
            {0, "pos"},
        };

        auto prog = Program(shaders, attribs);
        prog.use();

        glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        while (!glfwWindowShouldClose(win)) {
            glfwPollEvents();

            int fb_width, fb_height;
            glfwGetFramebufferSize(win, &fb_width, &fb_height);
            glViewport(0, 0, fb_width, fb_height);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

            glfwSwapBuffers(win);
        }
    }
    catch (const GLSBError& ex) {
        spdlog::error("Error building shaders: {}", ex.what());
        glfwDestroyWindow(win);
        glfwTerminate();
        return 1;
    }

    glfwDestroyWindow(win);

    glfwTerminate();

    return 0;
}
