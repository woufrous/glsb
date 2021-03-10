#include <iostream>

#define GLFW_INCLUDE_GL_3
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>

#include "application.h"
#include "layer.h"
#include "texture.h"

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

struct Vertex {
    glm::vec2 pos;
    glm::vec2 uv;
};

class SandboxApp final : public Application {
    public:
        SandboxApp(GLFWwindow* win) : Application{}, win_{win} {
            layers_.push_back(std::make_unique<ImGuiLayer>(win));
        }
        ~SandboxApp() {
            for (auto& layer : layers_) {
                layer->cleanup();
            }
        }
        void init() override {
            const char* vert_src = \
            "#version 330 core\n"
            ""
            "in vec2 pos;"
            "in vec2 vert_uv;"
            "out vec2 uv;"
            ""
            "void main() {"
            "    gl_Position = vec4(pos.xy, 0.0, 1.0);"
            "    uv = vert_uv;"
            "}";

            const char* frag_src = \
            "#version 330 core\n"
            ""
            "in vec2 uv;"
            "out vec4 color;"
            "uniform vec4 u_color;"
            "uniform sampler2D tex;"
            ""
            "void main() {"
            //"    color = u_color;"
            "    color = texture(tex, uv);"
            "}";

            GLuint vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            auto vertices_ = std::vector<Vertex>{
                {{-0.5f, 0.5f}, {0.0f, 0.0f}},
                {{0.5f, 0.5f}, {1.0f, 0.0f}},
                {{0.5f, -0.5f}, {1.0f, 1.0f}},
                {{-0.5f, -0.5f}, {0.0f, 1.0f}},
            };
            auto indices_ = std::vector<uint32_t>{
                0, 1, 2, 3, 0, 2
            };

            vertex_buffer_.bind();
            vertex_buffer_.set_data(vertices_.data(), vertices_.size()*sizeof(Vertex), GL_STATIC_DRAW);

            index_buffer_.bind();
            index_buffer_.set_data(indices_.data(), indices_.size()*sizeof(uint32_t), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
            glEnableVertexAttribArray(1);

            auto shaders = std::vector<Shader>{};
            shaders.emplace_back(Shader::Type::Vertex, vert_src);
            shaders.emplace_back(Shader::Type::Fragment, frag_src);

            auto attribs = std::vector<std::pair<uint32_t, const char*>>{
                {0, "pos"},
                {1, "vert_uv"},
            };

            prog_ = Program(shaders, attribs);
            prog_.use();

            color_ = glm::vec4{1.0f, 0.0f, 0.0f, 1.0f};

            auto img = Texture("res/opengl.png");

            GLuint tex;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<const void*>(img.data()));

            glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            for (auto& layer : layers_) {
                layer->init();
            }
        }

        void update() override {
            if (glfwWindowShouldClose(win_)) {
                this->is_running_ = false;
                return;
            }

            glfwPollEvents();

            for (auto& layer : layers_) {
                layer->on_update();
            }

            int fb_width, fb_height;
            glfwGetFramebufferSize(win_, &fb_width, &fb_height);
            glViewport(0, 0, fb_width, fb_height);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void draw() override {
            prog_.set_uniform("u_color", color_);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            for (auto& layer : layers_) {
                layer->on_draw();
            }

            glfwSwapBuffers(win_);
        }

    private:
        GLFWwindow* win_;
        LayerStack layers_;

        Buffer<BufferType::Array> vertex_buffer_;
        Buffer<BufferType::ElementArray> index_buffer_;
        Program prog_;
        glm::vec4 color_;
};

int main() {
    if (!glfwInit()) {
        spdlog::critical("Error initializing GLFW");
        return 1;
    }

    glfwSetErrorCallback(glfw_error_cb);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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

    try {
        auto app = SandboxApp(win);
        app.init();
        app.run();
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
