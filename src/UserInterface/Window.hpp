#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window {
    static constexpr const char* window_name = "CAN Network Designer";
    static constexpr bool vsync = true;

    int width = 800;
    int height = 600;

    GLFWwindow* window = nullptr;
public:
    void init();

    GLFWwindow* getWindow() const { return window; }
    int getWidth() const;
    int getHeight() const;

    static void pollEvents();
    bool shouldClose() const;
    void swapBuffers() const;

    ~Window();
};
