#pragma once
#include "Window.hpp"
//added for demo
#include <optional>
#include <filesystem>
#include <string>
#include "../Project/DemoFileOpener.hpp"
namespace fs = std::filesystem;

class UserInterface {
    static constexpr bool dark_mode = true;
    Window& window;
    //added for demo
    std::optional<fs::path> selectedDBPath;
    bool fileOpened = false;

public:
    explicit UserInterface(Window& win) : window(win) {}

    void init() const;

    static void begin();
    void end() const;
    //changed from static for demo, will be used as the home panel created upon startup later
    bool homePanel();

    void toolbar();

    void workspace();

    void deviceUI();

    ~UserInterface();
};
