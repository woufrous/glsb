#pragma once

#include <exception>

class GLSBError : public std::runtime_error {
    public:
        GLSBError(const char* what) : std::runtime_error(what) {}
};

template <typename T>
struct NoopDeleter {
    void operator()(T obj) const noexcept {
        (void)obj;
    }
};

template <typename T, typename Deleter=NoopDeleter<T>>
class UniqueHandle {
    public:
        using value_type = T;
        using deleter_type = Deleter;

        UniqueHandle() : hndl_{0}, deleter_{} {
            static_assert(!std::is_pointer_v<deleter_type>, "deleter would be null-pointer");
        }
        UniqueHandle(T hndl) : hndl_{hndl}, deleter_{} {
            static_assert(!std::is_pointer_v<deleter_type>, "deleter would be null-pointer");
        }
        UniqueHandle(T hndl, Deleter deleter) : hndl_{hndl}, deleter_{deleter} {}

        UniqueHandle(const UniqueHandle&) = delete;
        UniqueHandle& operator=(const UniqueHandle&) = delete;

        UniqueHandle(UniqueHandle&& other) :
            hndl_{std::exchange(other.hndl_, 0)},
            deleter_{std::move(other.deleter_)} {}
        UniqueHandle& operator=(UniqueHandle&& other) {
            this->hndl_ = std::exchange(other.hndl_, 0);
            this->deleter_ = std::move(other.deleter_);
            return *this;
        }

        T get() const noexcept {
            return hndl_;
        }

        void reset(T new_hndl=T{}) {
            if (new_hndl != hndl_) {
                auto old = std::exchange(hndl_, new_hndl);
                deleter_(old);
            }
        }

        ~UniqueHandle() {
            if ((hndl_ != T{})) {
                deleter_(hndl_);
            }
        }
    private:
        T hndl_;
        Deleter deleter_;
};
