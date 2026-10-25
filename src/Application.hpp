#pragma once
#include "UserInterface/Window.hpp"
#include "UserInterface/UserInterface.hpp"

class Application {
    Window main_window;
    UserInterface ui;

    void OnUpdate();
    void OnRenderScenes();
    void OnRenderUI();
public:
    Application();

    void Run();

    ~Application() = default;
};
