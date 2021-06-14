#pragma once

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

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

template <typename TgtT>
struct BindingPointTraits {};

template <typename BindingPointT>
class BindingContext;

template <typename TgtT, typename IdxT, typename CtxT, typename BindingTraits=BindingPointTraits<TgtT>>
class BindingPoint {
    public:
        using target_type = TgtT;
        using index_type = IdxT;
        using binding_fn_type = void(*)(std::underlying_type_t<target_type>, index_type);
        using context_type = CtxT;

        constexpr BindingPoint(target_type tgt) : tgt_{tgt} {}

        [[nodiscard]] context_type use(index_type idx) {
            return context_type(*this, idx);
        }

        bool bind(index_type idx) noexcept {
            if (idx == bound_idx_) {
                // already bound: do nothing
                return false;
            }
            // TODO: handle multiple access => lock or at least log
            BindingPointTraits<TgtT>::binding_fn(static_cast<std::underlying_type_t<TgtT>>(tgt_), idx);
            bound_idx_ = idx;
            return true;
        }

        void unbind() noexcept {
            if (bound_idx_ != index_type{}) {
                BindingPointTraits<TgtT>::binding_fn(static_cast<std::underlying_type_t<TgtT>>(tgt_), index_type{});
                bound_idx_ = index_type{};
            }
        }
    protected:
        TgtT tgt_;
        index_type bound_idx_;

        friend context_type;
        template <typename T>
        friend class BindingContext;
};

template <typename BindingPointT>
class BindingContext {
    public:
        BindingContext(BindingPointT& binding, typename BindingPointT::index_type idx) : binding_{binding}, idx_{idx} {
            unbind_on_exit_ = binding_.bind(idx);
        }
        ~BindingContext() {
            if (unbind_on_exit_) {
                binding_.unbind();
            }
        }

        typename BindingPointT::target_type tgt() const noexcept {
            return binding_.tgt_;
        }
    private:
        BindingPointT& binding_;
        typename BindingPointT::index_type idx_;
        bool unbind_on_exit_ = false;
};

inline std::vector<char> load_file(const std::filesystem::path& fpath) {
    auto ifs = std::ifstream(fpath);
    ifs.seekg(0, std::ios_base::end);
    auto len = static_cast<size_t>(ifs.tellg());
    ifs.seekg(0);

    auto ret = std::vector<char>(len+1);
    ifs.read(ret.data(), len);
    ret[len] = 0;

    return ret;
}
