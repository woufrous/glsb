#pragma once

#include "layer.h"
#include "renderer.h"
#include "input.h"

class Application {
    public:
        Application(GLFWwindow* win) : win_{win}, renderer_{win}, input_mngr_{win}, is_running_{true} {
            layers_.push_back(std::make_unique<ImGuiLayer>(*this, win));
        }
        ~Application() {
            for (auto& layer : layers_) {
                layer->cleanup();
            }
        }

        void run() {
            while (is_running_) {
                if (glfwWindowShouldClose(win_)) {
                    this->is_running_ = false;
                    break;
                }
                prepare_frame();
                update();
                draw();
            }
        }

        void init() {
            renderer_.init();
            for (auto& layer : layers_) {
                layer->init();
            }
        }

        void prepare_frame() {
            glfwPollEvents();

            for (auto& layer : layers_) {
                layer->prepare_frame();
            }
        }

        void update() {
            for (auto& layer : layers_) {
                layer->on_update();
            }
        }
        void draw() {
            renderer_.clear_screen();

            for (auto& layer : layers_) {
                layer->on_draw();
            }

            glfwSwapBuffers(win_);
        }

        LayerStack& layers() noexcept {
            return layers_;
        }

        Renderer& renderer() noexcept {
            return renderer_;
        }

        const Renderer& renderer() const noexcept {
            return renderer_;
        }

        InputManager& input_manager() {
            return input_mngr_;
        }

        const InputManager& input_manager() const {
            return input_mngr_;
        }
    protected:
        GLFWwindow* win_;
        Renderer renderer_;
        GLFWInputManager input_mngr_;
        bool is_running_;

        LayerStack layers_;
};
