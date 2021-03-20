#pragma once

#include <type_traits>
#include <functional>
#include <vector>

#include <GLFW/glfw3.h>

enum class KeyCode : int {
    KEY_A = GLFW_KEY_A,
    KEY_B = GLFW_KEY_B,
    KEY_C = GLFW_KEY_C,
    KEY_D = GLFW_KEY_D,
    KEY_E = GLFW_KEY_E,
    KEY_F = GLFW_KEY_F,
    KEY_G = GLFW_KEY_G,
    KEY_H = GLFW_KEY_H,
    KEY_I = GLFW_KEY_I,
    KEY_J = GLFW_KEY_J,
    KEY_K = GLFW_KEY_K,
    KEY_L = GLFW_KEY_L,
    KEY_M = GLFW_KEY_M,
    KEY_N = GLFW_KEY_N,
    KEY_O = GLFW_KEY_O,
    KEY_P = GLFW_KEY_P,
    KEY_Q = GLFW_KEY_Q,
    KEY_R = GLFW_KEY_R,
    KEY_S = GLFW_KEY_S,
    KEY_T = GLFW_KEY_T,
    KEY_U = GLFW_KEY_U,
    KEY_V = GLFW_KEY_V,
    KEY_W = GLFW_KEY_W,
    KEY_X = GLFW_KEY_X,
    KEY_Y = GLFW_KEY_Y,
    KEY_Z = GLFW_KEY_Z,
    KEY_0 = GLFW_KEY_0,
    KEY_1 = GLFW_KEY_1,
    KEY_2 = GLFW_KEY_2,
    KEY_3 = GLFW_KEY_3,
    KEY_4 = GLFW_KEY_4,
    KEY_5 = GLFW_KEY_5,
    KEY_6 = GLFW_KEY_6,
    KEY_7 = GLFW_KEY_7,
    KEY_8 = GLFW_KEY_8,
    KEY_9 = GLFW_KEY_9,
    KEY_ESC = GLFW_KEY_ESCAPE,
    KEY_TAB = GLFW_KEY_TAB,
    KEY_LSHIFT = GLFW_KEY_LEFT_SHIFT,
    KEY_RSHIFT = GLFW_KEY_RIGHT_SHIFT,
    KEY_LCTRL = GLFW_KEY_LEFT_CONTROL,
    KEY_RCTRL = GLFW_KEY_RIGHT_CONTROL,
    KEY_LALT = GLFW_KEY_LEFT_ALT,
    KEY_RALT = GLFW_KEY_RIGHT_ALT,
    KEY_LSUPER = GLFW_KEY_LEFT_SUPER,
    KEY_RSUPER = GLFW_KEY_RIGHT_SUPER,
};

enum class KeyState : int {
    Pressed = GLFW_PRESS,
    Released = GLFW_RELEASE,
    Repeated = GLFW_REPEAT,
};

namespace details {

template <typename EnumT, typename StorageT = ::std::underlying_type_t<EnumT>>
struct Bitset {
    using value_type = StorageT;
    using enum_type = EnumT;

    struct Bit {
        bool value_;
        Bitset& bitset_;
        enum_type bit_;

        operator bool() const {
            return value_;
        }

        Bit& operator=(bool state) {
            if (state) {
                bitset_.set_bit(bit_);
            } else {
                bitset_.clear_bit(bit_);
            }
            value_ = state;
        }
    };

    Bitset() : bits_{} {}
    Bitset(enum_type bit) : bits_{static_cast<value_type>(bit)} {}
    Bitset(value_type bits) : bits_{bits} {}

    operator value_type() const {
        return bits_;
    }

    void set_bit(enum_type bit) {
        bits_ |= static_cast<value_type>(bit);
    }

    void clear_bit(enum_type bit) {
        bits_ &= ~static_cast<value_type>(bit);
    }

    Bit is_set(enum_type bit) {
        return Bit{
            ((bits_ & static_cast<value_type>(bit)) != 0),
            *this,
            bit
        };
    }

    Bit operator[](enum_type bit) {
        return is_set(bit);
    }

    private:
        value_type bits_;
};

enum class KeyModifierBits : int {
    Shift = GLFW_MOD_SHIFT,
    Ctrl = GLFW_MOD_CONTROL,
    Alt = GLFW_MOD_ALT,
    Super = GLFW_MOD_SUPER,
    CapsLock = GLFW_MOD_CAPS_LOCK,
    NumLock = GLFW_MOD_NUM_LOCK,
};

}

using KeyModifier = details::Bitset<details::KeyModifierBits>;

class InputManager {
    public:
        using key_handler_type = std::function<void(KeyCode, KeyState, KeyModifier)>;

        virtual ~InputManager() = default;

        virtual KeyState key_state(KeyCode) const = 0;

        virtual void register_key_handler(key_handler_type handler) = 0;
};

class GLFWInputManager : public InputManager {
    public:
        GLFWInputManager(GLFWwindow* win) : win_{win} {
            glfwSetWindowUserPointer(win, this);

            glfwSetKeyCallback(win_, GLFWInputManager::on_key);
        }

        static void on_key(GLFWwindow* win, int key, int /*scancode*/, int action, int mods) {
            auto input_mngr = reinterpret_cast<GLFWInputManager*>(glfwGetWindowUserPointer(win));
            for (auto hndlr : input_mngr->key_handlers_) {
                hndlr(KeyCode(key), KeyState(action), KeyModifier(mods));
            }
        }

        KeyState key_state(KeyCode kcode) const {
            return KeyState(
                glfwGetKey(win_, static_cast<std::underlying_type_t<KeyCode>>( kcode))
            );
        }

        void register_key_handler(key_handler_type handler) {
            key_handlers_.push_back(handler);
        }
    private:
        GLFWwindow* win_;
        std::vector<key_handler_type> key_handlers_;
};
