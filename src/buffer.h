#include "utils.h"

#include <GLFW/glfw3.h>

enum class BufferType : GLenum {
    Array = GL_ARRAY_BUFFER,
    ElementArray = GL_ELEMENT_ARRAY_BUFFER,
};

template <BufferType Type>
class Buffer {
    public:
        struct BufferDeleter {
            void operator()(GLuint buffer_hndl) const noexcept {
                glDeleteBuffers(1, &buffer_hndl);
            }
        };
        using UniqueBufferHandle = UniqueHandle<GLuint, BufferDeleter>;

        Buffer() : buf_{}, is_bound_{false} {
            auto buf = typename UniqueBufferHandle::value_type{};
            glGenBuffers(1, &buf);
            buf_.reset(buf);
        }

        void set_data(void* data, size_t size, GLenum usage) const {
            bool do_unbind = !is_bound_;
            bind();
            glBufferData(
                static_cast<std::underlying_type_t<BufferType>>(Type),
                size,
                data,
                usage);
            if (do_unbind) {
                unbind();
            }
        }

        void bind() const noexcept {
            if (!is_bound_) {
                glBindBuffer(static_cast<std::underlying_type_t<BufferType>>(Type), buf_.get());
                is_bound_ = true;
            }
        }

        void unbind() const noexcept {
            if (is_bound_) {
                glBindBuffer(static_cast<std::underlying_type_t<BufferType>>(Type), 0);
                is_bound_ = false;
            }
        }
    private:
        UniqueBufferHandle buf_;
        mutable bool is_bound_;
};
