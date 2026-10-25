#include "Application.hpp"

void Application::Run() {
    main_window.init();
    ui.init();

    while (!main_window.shouldClose()) {
        main_window.pollEvents();

        OnUpdate();

        OnRenderScenes();

        ui.begin();
        OnRenderUI();
        ui.end();

        main_window.swapBuffers();
    }
}

void Application::OnUpdate() {
}

void Application::OnRenderScenes() {
}

void Application::OnRenderUI() {
    ui.helloWorldPanel();
}

Application::Application() : ui(main_window) {}
