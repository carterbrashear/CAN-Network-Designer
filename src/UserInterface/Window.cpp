#include "Window.hpp"
#include <iostream>

static void glfw_error_callback(const int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << "\n";
}

void Window::init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        exit(-1);
    }

    if (window != nullptr) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    // Configure OpenGL 3.3 Core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, window_name, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW Window\n";
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(vsync ? 1 : 0);

    // Init GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(-1);
    }
}

int Window::getWidth() const {
    if (window) {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        return w;
    }
    return width;
}

int Window::getHeight() const {
    if (window) {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        return h;
    }
    return height;
}

void Window::pollEvents() {
    glfwPollEvents();
}

bool Window::shouldClose() const {
    return window ? glfwWindowShouldClose(window) : true;
}

void Window::swapBuffers() const {
    if (window) {
        glfwSwapBuffers(window);
    }
}

Window::~Window() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}
