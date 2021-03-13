#pragma once

#include "layer.h"
#include "renderer.h"

class Application {
    public:
        Application(GLFWwindow* win) : win_{win}, is_running_{true} {
            layers_.push_back(std::make_unique<ImGuiLayer>(*this, win));
        }
        ~Application() {
            for (auto& layer : layers_) {
                layer->cleanup();
            }
        }

        void run() {
            while (is_running_) {
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
        void update() {
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
        void draw() {
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
    protected:
        GLFWwindow* win_;
        bool is_running_;

        LayerStack layers_;
        Renderer renderer_;
};
