#pragma once
#include <gamepad.h>
namespace robot {
    struct Gamepad_wrapper : gamepad::Gamepad {
        explicit Gamepad_wrapper(const std::string &);
        explicit Gamepad_wrapper(int);
        ~Gamepad_wrapper();
        std::atomic<bool> ready;
    };
}
