#pragma once

#include <cstdint>
#include <stdexcept>

#include "pch.h"

namespace glsb::vk {

class VulkanError : public std::runtime_error {
public:
    VulkanError(const char* what, ::vk::Result res) : std::runtime_error(what), res_{res} {}

    ::vk::Result
    get_result() const noexcept {
        return res_;
    }

private:
    ::vk::Result res_;
};

inline ::vk::Instance
create_instance(const char* app_name, uint32_t app_version, const std::vector<const char*>& extensions) {
    auto app_info = ::vk::ApplicationInfo()
                        .setApiVersion(VK_VERSION_1_1)
                        .setPEngineName("Merlin")
                        .setEngineVersion(VK_MAKE_VERSION(0, 1, 0))
                        .setPApplicationName(app_name)
                        .setApplicationVersion(app_version);
    auto create_info = ::vk::InstanceCreateInfo().setPApplicationInfo(&app_info).setPEnabledExtensionNames(extensions);

    auto [res, inst] = ::vk::createInstance(create_info);
    if (res != ::vk::Result::eSuccess) {
        throw VulkanError("Error creating instance", res);
    }
    return inst;
}

}    // namespace glsb::vk
