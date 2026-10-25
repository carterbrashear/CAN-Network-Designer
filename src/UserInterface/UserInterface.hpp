#pragma once
#include "Window.hpp"

class UserInterface {
    static constexpr bool dark_mode = true;
    Window& window;

public:
    explicit UserInterface(Window& win) : window(win) {}

    void init() const;

    static void begin();
    void end() const;

    static void helloWorldPanel();

    ~UserInterface();
};
