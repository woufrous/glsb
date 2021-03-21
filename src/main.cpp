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
#include "scene.h"
#include "shader.h"
#include "buffer.h"
#include "renderer.h"
#include "utils.h"

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
        SandboxLayer(Application& app) : Layer{app} {
            scene_.cam = Camera{
                {2.f, 2.f, 2.f},
                {0.f, 0.f, 0.f},
                std::make_pair(0.1f, 10.f),
                40.f,
                16.f/9.f
            };
            scene_.light = Light{
                {-1.f, 0.f, 1.f},
                {1.f, 1.f, 1.f},
                1.0f
            };
        }

        void init() override {
            app_.input_manager().register_key_handler([](KeyCode code, KeyState state, KeyModifier mods) -> void {
                const char* state_str = nullptr;
                switch (state) {
                    case KeyState::Pressed: {
                        state_str = "pressed";
                    } break;
                    case KeyState::Released: {
                        state_str = "released";
                    } break;
                    case KeyState::Repeated: {
                        state_str = "repeated";
                    } break;
                }
                spdlog::info("Key {} {} with modifiers 0b{:b}", static_cast<int>(code), state_str, static_cast<int>(mods));
            });

            app_.input_manager().register_mouse_scroll_handler([this](double /*x_offs*/, double y_offs){
                this->scene_.cam.fov += y_offs*5;
            });

            const char* vert_src = \
            "#version 330 core\n"
            ""
            "in vec3 pos;"
            "in vec3 normal;"
            "in vec2 vert_uv;"
            ""
            "out vec2 uv;"
            "out float mu;"
            ""
            "uniform mat4 u_mvp;"
            "uniform vec3 light;"
            ""
            "void main() {"
            "    gl_Position = u_mvp*vec4(pos, 1.0);"
            "    uv = vert_uv;"
            "    vec3 rel_light = light-pos;"
            "    mu = dot(normal, rel_light)/(length(rel_light)*length(normal));"
            "}";

            const char* frag_src = \
            "#version 330 core\n"
            ""
            "in vec2 uv;"
            "in float mu;"
            "out vec4 color;"
            "uniform sampler2D tex;"
            ""
            "void main() {"
            "    vec4 tex_color = texture(tex, uv);"
            "    color = tex_color;"
            "}";

            auto shaders = std::vector<Shader>{};
            shaders.emplace_back(Shader::Type::Vertex, vert_src);
            shaders.emplace_back(Shader::Type::Fragment, frag_src);

            auto& prog = app_.renderer().shader_manager().add_shader(
                "default", shaders
            );

            prog.use();

            scene_.meshes.emplace_back(load_obj("res/room.obj"));
            upload_meshes();

            auto img = Texture("res/room.png");

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

        void cleanup() override {}
        void prepare_frame() override {}

        void on_update() override {
            if (app_.input_manager().key_state(KeyCode::KEY_W) == KeyState::Pressed) {
                scene_.cam.pos += scene_.cam.local_ccs().e_z*0.1f;
            }
            if (app_.input_manager().key_state(KeyCode::KEY_S) == KeyState::Pressed) {
                scene_.cam.pos -= scene_.cam.local_ccs().e_z*0.1f;
            }
            if (app_.input_manager().key_state(KeyCode::KEY_D) == KeyState::Pressed) {
                scene_.cam.pos += scene_.cam.local_ccs().e_y*0.1f;
            }
            if (app_.input_manager().key_state(KeyCode::KEY_A) == KeyState::Pressed) {
                scene_.cam.pos -= scene_.cam.local_ccs().e_y*0.1f;
            }
            if (app_.input_manager().key_state(KeyCode::KEY_Q) == KeyState::Pressed) {
                scene_.cam.pos += scene_.cam.local_ccs().e_x*0.1f;
            }
            if (app_.input_manager().key_state(KeyCode::KEY_Z) == KeyState::Pressed) {
                scene_.cam.pos -= scene_.cam.local_ccs().e_x*0.1f;
            }
            ImGui::Begin("Camera control", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::SliderFloat3("Position", &scene_.cam.pos[0], -3.f, 3.f, "%.1f", 1.f);
                ImGui::SliderFloat("FoV", &scene_.cam.fov, 1.f, 179.f, "%.0f", 1.f);
            ImGui::End();
            ImGui::Begin("Light", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::SliderFloat3("Position", &scene_.light.pos[0], -3.f, 3.f, "%.1f", 1.f);
            ImGui::End();
        }

        void on_draw() override {
            auto fb_size = app_.renderer().get_viewport_dim();
            scene_.cam.aspect = (float)fb_size.width/(float)fb_size.height;
            auto vp = scene_.cam.get_vp_matrix();

            auto& prog = app_.renderer().shader_manager().get_shader("default");
            prog.use();
            prog.set_uniform("u_mvp", vp);
            prog.set_uniform("light", scene_.light.pos);

            for (auto mesh : mesh_hndls_) {
                app_.renderer().render(mesh);
            }
        }

    private:
        void upload_meshes() noexcept {
            for (const auto& mesh : scene_.meshes) {
                mesh_hndls_.emplace_back(app_.renderer().upload_mesh(mesh));
            }
        }

        Scene scene_;

        std::vector<Renderer::handle_type> mesh_hndls_;
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
