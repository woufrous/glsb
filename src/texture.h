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
                stbi_load(fpath.string().c_str(), &width_, &height_, &channels, STBI_rgb_alpha),
                stbi_image_free
            );

            if (ptr_ == nullptr) {
                throw std::runtime_error("Error loading image");
            }
        }

        size_t size() const noexcept {
            return static_cast<size_t>(std::abs(width_)) * static_cast<size_t>(std::abs(height_)) * 4;
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

enum class TextureType : GLenum {
    UnsignedByte = GL_UNSIGNED_BYTE,
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

enum class TextureParameter : GLenum {
    MinFilter = GL_TEXTURE_MIN_FILTER,
    MagFilter = GL_TEXTURE_MAG_FILTER,
    WrapS = GL_TEXTURE_WRAP_S,
    WrapT = GL_TEXTURE_WRAP_T,
};

template <>
struct BindingPointTraits<TextureTarget> {
    static constexpr auto binding_fn = glBindTexture;
};

class TextureBindingContext;

using TextureBindingPoint = BindingPoint<TextureTarget, GLuint>;

class TextureBindingContext : public BindingContext<TextureBindingPoint> {
    public:
        TextureBindingContext(TextureBindingPoint& binding, TextureBindingPoint::index_type idx) :
            BindingContext<TextureBindingPoint>(binding, idx) {}
        void set_parameter(TextureParameter param, GLint val) {
            glTexParameteri(
                static_cast<std::underlying_type_t<TextureTarget>>(tgt()),
                static_cast<std::underlying_type_t<TextureParameter>>(param),
                val);
        }
        void allocate(TextureFormat fmt, TextureType type, int width, int height, const void* data) {
            glTexImage2D(
                static_cast<std::underlying_type_t<TextureTarget>>(tgt()),
                0,
                static_cast<GLint>(fmt),    // the internal format is GLint, not GLenum
                width, height, 0,
                static_cast<std::underlying_type_t<TextureFormat>>(fmt),
                static_cast<std::underlying_type_t<TextureType>>(type),
                data);
        }
        void gen_mipmap() {
            glGenerateMipmap(static_cast<std::underlying_type_t<TextureTarget>>(tgt()));
        }
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

        void allocate(int width, int height, const void* data) {
            auto tex = TextureBindingContext(binding_, hndl_.get());
            tex.allocate(TextureFormat::RGBA, TextureType::UnsignedByte, width, height, data);
            if (use_mipmap_) {
                tex.gen_mipmap();
            }
        }

        void bind() {
            binding_.bind(hndl_.get());
        }

        void unbind() {
            binding_.unbind();
        }

        void set_filtering(TextureFilter filter, bool use_mipmap) {
            use_mipmap_ = use_mipmap;

            {
                auto tex = TextureBindingContext(binding_, hndl_.get());
                tex.set_parameter(TextureParameter::MinFilter, get_filter_param(filter, use_mipmap));
                tex.set_parameter(TextureParameter::MagFilter, get_filter_param(filter, false));
            }
        }

        void set_wrapping(TextureWrapping wrapping) {
            auto tex = TextureBindingContext(binding_, hndl_.get());
            tex.set_parameter(TextureParameter::WrapS, static_cast<std::underlying_type_t<TextureWrapping>>(wrapping));
            tex.set_parameter(TextureParameter::WrapT, static_cast<std::underlying_type_t<TextureWrapping>>(wrapping));
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
