#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <application.h>
#include <layer.h>
#include <vulkan/core.h>

class SandboxLayer final : public Layer {
public:
    SandboxLayer(Application& app, const std::vector<const char*>& extensions) :
        Layer{app}, inst_{glsb::vk::create_instance("Vulkan Sandbox", VK_MAKE_VERSION(0, 1, 0), extensions)} {}

    void
    init() override {}

    void
    cleanup() override {
        // inst_.destroy();
    }
    void
    prepare_frame() override {}

    void
    on_update() override {}

    void
    on_draw() override {}

private:
    vk::UniqueInstance inst_;
};

static void
glfw_error_cb(int error, const char* msg) {
    spdlog::error("GLFW error ({:d}): {}", error, msg);
}

int
main() {
    if (!glfwInit()) {
        spdlog::critical("Error initializing GLFW");
        return 1;
    }

    glfwSetErrorCallback(glfw_error_cb);

    if (!glfwVulkanSupported()) {
        spdlog::critical("GLFW: no Vulkan support found");
        glfwTerminate();
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto win = glfwCreateWindow(800, 600, "Vulkan sandbox", nullptr, nullptr);
    if (!win) {
        spdlog::error("Error creating window");
        glfwTerminate();
        return 1;
    }

    auto extensions = std::vector<const char*>{};
    {
        uint32_t ext_cnt;
        auto ext_names = glfwGetRequiredInstanceExtensions(&ext_cnt);
        extensions.resize(ext_cnt);
        for (size_t i = 0; i < ext_cnt; ++i) {
            extensions[i] = ext_names[i];
        }
    }

    spdlog::info("Required extensions:");
    for (const auto& ext : extensions) {
        spdlog::info("\t{}", ext);
    }

    try {
        auto app = Application(win);
        app.layers().emplace_front(std::make_unique<SandboxLayer>(app, extensions));
        app.init();
        app.run();
    }
    catch (const glsb::vk::VulkanError& ex) {
        spdlog::critical("Vulkan error: {} ({})", ex.what(), ::vk::to_string(ex.get_result()));
        glfwDestroyWindow(win);
        glfwTerminate();
        return 1;
    }
    catch (const GLSBError& ex) {
        spdlog::critical("Error building shaders: {}", ex.what());
        glfwDestroyWindow(win);
        glfwTerminate();
        return 1;
    }

    glfwDestroyWindow(win);

    glfwTerminate();

    return 0;
}
