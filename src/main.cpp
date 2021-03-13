#include <iostream>

#define GLFW_INCLUDE_GL_3
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>

#include "application.h"
#include "layer.h"
#include "texture.h"

#include "shader.h"
#include "buffer.h"
#include "renderer.h"

static float g_max_anisotropy = -1.;

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

class SandboxLayer final : public Layer {
    public:
        SandboxLayer(Application& app) : Layer{app}, cam_pos_{-1.f, -1.f, 1.f}, cam_fov_{40.f} {
        }

        void init() override {
            const char* vert_src = \
            "#version 330 core\n"
            ""
            "in vec2 pos;"
            "in vec2 vert_uv;"
            "out vec2 uv;"
            "uniform mat4 u_mvp;"
            ""
            "void main() {"
            "    gl_Position = u_mvp*vec4(pos.xy, 0.0, 1.0);"
            "    uv = vert_uv;"
            "}";

            const char* frag_src = \
            "#version 330 core\n"
            ""
            "in vec2 uv;"
            "out vec4 color;"
            "uniform sampler2D tex;"
            ""
            "void main() {"
            "    color = texture(tex, uv);"
            "}";

            auto mesh = Mesh{};
            mesh.vertex_data = std::vector<Vertex>{
                {{-0.5f, 0.5f}, {0.0f, 0.0f}},
                {{0.5f, 0.5f}, {1.0f, 0.0f}},
                {{0.5f, -0.5f}, {1.0f, 1.0f}},
                {{-0.5f, -0.5f}, {0.0f, 1.0f}},
            };
            mesh.index_data = std::vector<uint32_t>{
                0, 1, 2, 3, 0, 2
            };

            mesh_ = app_.renderer().upload_mesh(mesh);

            auto shaders = std::vector<Shader>{};
            shaders.emplace_back(Shader::Type::Vertex, vert_src);
            shaders.emplace_back(Shader::Type::Fragment, frag_src);

            auto attribs = std::vector<std::pair<uint32_t, const char*>>{
                {0, "pos"},
                {1, "vert_uv"},
            };

            prog_ = Program(shaders, attribs);
            prog_.use();

            auto img = Texture("res/opengl.png");

            GLuint tex;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            if (g_max_anisotropy > 0) {
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_max_anisotropy);
            }

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<const void*>(img.data()));
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        void cleanup() override {
        }

        void prepare_frame() override {}

        void on_update() override {
            ImGui::Begin("Camera control", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::SliderFloat3("Position", &cam_pos_[0], -1.f, 1.f, "%.1f", 1.f);
                ImGui::SliderFloat("FoV", &cam_fov_, 1.f, 179.f, "%.0f", 1.f);
            ImGui::End();
            auto view = glm::lookAt(
                cam_pos_,
                glm::vec3(0.f, 0.f, 0.f),
                glm::vec3(0.f, 0.f, 1.f)
            );
            auto fb_size = app_.renderer().get_viewport_dim();
            auto proj = glm::perspective(glm::radians(cam_fov_), (float)fb_size.width/(float)fb_size.height, .1f, 10.f);
            auto mvp = proj * view;
            prog_.set_uniform("u_mvp", mvp);
        }

        void on_draw() override {
            app_.renderer().render(mesh_);
        }

    private:
        glm::vec3 cam_pos_;
        float cam_fov_;

        Renderer::handle_type mesh_;
        Program prog_;
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

    if (glewIsExtensionSupported("GL_EXT_texture_filter_anisotropic")) {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &g_max_anisotropy);
        spdlog::info("Anisotropic filtering supported up to {}", g_max_anisotropy);
    }

#ifndef NDEBUG
    glDebugMessageCallback(gl_error_cb, nullptr);
#endif // NDEBUG

    try {
        auto app = Application(win);
        app.layers().emplace_front(std::make_unique<SandboxLayer>(app));
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
