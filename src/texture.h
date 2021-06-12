#pragma once

#include <cstdint>
#include <memory>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GL/glew.h>

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

        int width() const noexcept {
            return width_;
        }

        int height() const noexcept {
            return height_;
        }

    private:
        std::unique_ptr<uint8_t, decltype(&stbi_image_free)> ptr_;
        int width_;
        int height_;
};

enum class TextureFormat : GLenum {
    RGBA = GL_RGBA,
};

enum class TextureFilter {
    Nearest,
    Linear,
    Trilinear,
};

enum class TextureWrapping : GLint {
    ClampToBorder = GL_CLAMP_TO_BORDER,
};

enum class TextureTarget : GLenum {
    Texture2D = GL_TEXTURE_2D,
};

template <>
struct BindingPointTraits<TextureTarget> {
    static constexpr auto binding_fn = glBindTexture;
};

class Texture;

class TextureBindingPoint : public BindingPoint<TextureTarget, GLuint> {
    friend Texture;
};

class Texture {
    struct TextureDeleter {
        void operator()(GLuint hndl) const noexcept {
            glDeleteTextures(1, &hndl);
        }
    };
    using UniqueTextureHandle = UniqueHandle<GLuint, TextureDeleter>;

    public:
        Texture(TextureBindingPoint& binding) : binding_{binding}, hndl_{} {
            auto tex = UniqueTextureHandle::value_type{};
            glGenTextures(1, &tex);
            hndl_.reset(tex);
        }

        void bind() const {
            binding_.bind(hndl_.get());
        }

        void allocate(int width, int height, const void* data) {
            this->bind();
            glTexImage2D(static_cast<std::underlying_type_t<TextureTarget>>(binding_.tgt_), 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            if (use_mipmap_) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        }

        void set_filtering(TextureFilter filter, bool use_mipmap) {
            use_mipmap_ = use_mipmap;

            this->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, get_filter_param(filter, use_mipmap));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, get_filter_param(filter, false));
        }

        void set_wrapping(TextureWrapping wrapping) {
            this->bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<std::underlying_type_t<TextureWrapping>>(wrapping));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<std::underlying_type_t<TextureWrapping>>(wrapping));
        }

    private:
        GLint get_filter_param(TextureFilter filter, bool use_mipmap) {
            switch (filter) {
                case TextureFilter::Nearest: {
                    if (use_mipmap) {
                        return GL_NEAREST_MIPMAP_NEAREST;
                    } else {
                        return GL_NEAREST;
                    }
                }
                case TextureFilter::Linear: {
                    if (use_mipmap) {
                        return GL_LINEAR_MIPMAP_NEAREST;
                    } else {
                        return GL_LINEAR;
                    }
                }
                case TextureFilter::Trilinear: {
                    return GL_LINEAR_MIPMAP_LINEAR;
                }
            }
            abort();
        }

        TextureBindingPoint& binding_;
        UniqueTextureHandle hndl_;
        bool use_mipmap_ = false;
};
