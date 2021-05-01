#pragma once

#include <cstdint>
#include <memory>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class Bitmap {
    public:
        Bitmap(const std::filesystem::path& fpath) : ptr_(nullptr, stbi_image_free) {
            int channels;
            ptr_ = std::unique_ptr<uint8_t, decltype(&stbi_image_free)>(
                reinterpret_cast<uint8_t*>(
                    stbi_load(fpath.string().c_str(), &width_, &height_, &channels, STBI_rgb_alpha)
                ),
                stbi_image_free
            );

            if (ptr_ == nullptr) {
                throw std::runtime_error("Error loading image");
            }
        }

        size_t size() const noexcept {
            return width_ * height_ * 4;
        }

        const uint8_t* data() const noexcept {
            return ptr_.get();
        }

        uint32_t width() const noexcept {
            return static_cast<uint32_t>(width_);
        }

        uint32_t height() const noexcept {
            return static_cast<uint32_t>(height_);
        }

    private:
        std::unique_ptr<uint8_t, decltype(&stbi_image_free)> ptr_;
        int width_;
        int height_;
};
