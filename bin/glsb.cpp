#include <iostream>
#include <filesystem>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>

#include <application.h>
#include <layer.h>
#include <texture.h>
#include <scene.h>
#include <shader.h>
#include <buffer.h>
#include <renderer.h>
#include <utils.h>

static float g_max_anisotropy = -1.;

static void glfw_error_cb(int error, const char* msg) {
    spdlog::error("GLFW error ({:d}): {}", error, msg);
}

#ifndef NDEBUG
static void gl_error_cb(
        [[maybe_unused]] GLenum source,
        [[maybe_unused]] GLenum type,
        [[maybe_unused]] GLuint id,
        [[maybe_unused]] GLenum severity,
        [[maybe_unused]] GLsizei length,
        const GLchar *message,
        [[maybe_unused]] const void *userParam) {
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

auto tex2d = TextureBindingPoint(TextureTarget::Texture2D);

class SandboxLayer final : public Layer {
    public:
        SandboxLayer(Application& app) : Layer{app}, tex_{tex2d} {
            scene_.cam = Camera{
                {2.f, 2.f, 2.f},
                {0.f, 0.f, 0.f},
                std::make_pair(0.1f, 10.f),
                40.f,
                16.f/9.f
            };
            scene_.diffuse = {
                {3.f, 3.f, 3.f},
                {1.f, 1.f, 1.f},
                1.0f
            };
            scene_.ambient = {
                {0.8f, 0.8f, 1.f},
                0.5f
            };
        }

        void init() override {
            app_.input_manager().register_mouse_scroll_handler([this](double /*x_offs*/, double y_offs){
                this->scene_.cam.fov += static_cast<float>(y_offs)*5;
            });

            auto materials = std::unordered_map<const char*, std::pair<std::filesystem::path, std::filesystem::path>>{
                {"default", std::make_pair("res/vert.glsl", "res/frag.glsl")},
                {"flat", std::make_pair("res/flat.vert.glsl", "res/flat.frag.glsl")},
            };

            for (const auto& [name, files] : materials) {
                auto vert_src = load_file(files.first);
                auto frag_src = load_file(files.second);

                auto shaders = std::vector<Shader>{};
                shaders.emplace_back(Shader::Type::Vertex, vert_src.data());
                shaders.emplace_back(Shader::Type::Fragment, frag_src.data());

                app_.renderer().shader_manager().add_shader(name, shaders);
            }

            mesh_hndls_.emplace_back(app_.renderer().upload_mesh(
                load_obj("res/cube.obj").transform(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 1.f))),
                "default"
            ));
            mesh_hndls_.emplace_back(app_.renderer().upload_mesh(generate_quad(5.f, 5.f), "default"));

            auto img = Bitmap("res/cube.png");

            tex_.set_filtering(TextureFilter::Linear, true);
            tex_.set_wrapping(TextureWrapping::ClampToBorder);
            if (g_max_anisotropy > 0) {
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_max_anisotropy);
            }

            tex_.allocate(img.width(), img.height(), reinterpret_cast<const void*>(img.data()));
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
            ImGui::Begin("Contols", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize);
                if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::DragFloat3("Position", &scene_.cam.pos[0], .1f, -3.f, 3.f, "%.1f", 1.f);
                    ImGui::DragFloat("FoV", &scene_.cam.fov, 1.f, 1.f, 179.f, "%.0f", 1.f);
                }
                if (ImGui::CollapsingHeader("Ambient Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::ColorEdit3("Ambient Color", &scene_.ambient.color[0]);
                    ImGui::DragFloat("Ambient Intensity", &scene_.ambient.intensity, .1f, 0.0f, 2.0f, "%.1f", 1.f);
                }
                if (ImGui::CollapsingHeader("Diffuse Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::DragFloat3("Diffuse Direction", &scene_.diffuse.pos[0], .1f, -5.f, 5.f, "%.1f", 1.f);
                    ImGui::ColorEdit3("Diffuse Color", &scene_.diffuse.color[0]);
                    ImGui::DragFloat("Diffuse Intensity", &scene_.diffuse.intensity, .1f, 0.0f, 2.0f, "%.1f", 1.f);
                }
                if (ImGui::CollapsingHeader("Specular Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::DragFloat("Specular Roughness", &roughness_, 1.f, 1.0f, 1000.0f, "%.0f");
                    ImGui::DragFloat("Specular Intensity", &spec_intensity_, .1f, 0.0f, 10.0f, "%.1f");
                }
            ImGui::End();
        }

        void on_draw() override {
            auto fb_size = app_.renderer().get_viewport_dim();
            scene_.cam.aspect = static_cast<float>(fb_size.width)/static_cast<float>(fb_size.height);

            auto& prog = app_.renderer().shader_manager().get_shader("default");
            prog.use();
            prog.set_uniform("u_view", scene_.cam.get_view_matrix());
            prog.set_uniform("u_proj", scene_.cam.get_proj_matrix());
            prog.set_uniform("ambient.color", scene_.ambient.color);
            prog.set_uniform("ambient.intensity", scene_.ambient.intensity);
            prog.set_uniform("diffuse.pos", scene_.diffuse.pos);
            prog.set_uniform("diffuse.color", scene_.diffuse.color);
            prog.set_uniform("diffuse.intensity", scene_.diffuse.intensity);
            prog.set_uniform("spec.roughness", roughness_);
            prog.set_uniform("spec.intensity", spec_intensity_);
            prog.set_uniform("camera.pos", scene_.cam.pos);
            tex_.bind();

            for (auto mesh : mesh_hndls_) {
                app_.renderer().render(mesh);
            }
            tex_.unbind();
        }

    private:
        Scene scene_;
        float roughness_ = 1.f;
        float spec_intensity_ = 1.f;

        Texture tex_;

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
