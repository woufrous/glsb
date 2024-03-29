#pragma once

#include <deque>
#include <memory>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "imgui/imgui_glfw.h"
#include "imgui/imgui_ogl3.h"

class Application;

class Layer {
    public:
        Layer(Application& app) : app_{app} {}
        virtual ~Layer() = default;

        virtual void init() = 0;
        virtual void cleanup() = 0;

        virtual void prepare_frame() = 0;
        virtual void on_update() = 0;
        virtual void on_draw() = 0;
    protected:
        Application& app_;
};

class ImGuiLayer final : public Layer {
    public:
        ImGuiLayer(Application& app, GLFWwindow* win) : Layer{app}, win_{win}, is_initialized_{false} {}

        void init() override {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            auto io = ImGui::GetIO();
            (void)io;

            ImGui::StyleColorsDark();
            ImGui_ImplGlfw_InitForOpenGL(win_, true);
            ImGui_ImplOpenGL3_Init("#version 330 core");
            is_initialized_ = true;
        }

        void prepare_frame() override {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        void cleanup() override {
            if (is_initialized_) {
                ImGui_ImplOpenGL3_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();
            }
        }

        void on_update() override {
        }

        void on_draw() override {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    private:
        GLFWwindow* win_;
        bool is_initialized_;
};

using LayerStack = std::deque<std::unique_ptr<Layer>>;
