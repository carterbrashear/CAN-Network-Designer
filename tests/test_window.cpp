#include <cassert>
#include "../src/UserInterface/Window.hpp"
// Must be included after to prevent GLFW from loading old OpenGL version
#include <GLFW/glfw3.h>

int main() {
    // Make the tests run headless
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_NULL);

    Window test_window;
    test_window.init();

    // Verify window creation and dimensions
    GLFWwindow* handle = test_window.getWindow();
    assert(handle != nullptr && "Window must not be null after init.");
    assert(test_window.getWidth() > 0 && "Window size must be positive.");
    assert(test_window.getHeight() > 0 && "Window size must be positive.");

    // Verify window closing
    assert(!test_window.shouldClose() && "Window should not init with shouldClose returning true.");
    glfwSetWindowShouldClose(handle, GLFW_TRUE);
    assert(test_window.shouldClose() && "Window should close when told to.");

    // Verify loop calls don't crash
    test_window.pollEvents();
    test_window.swapBuffers();

    // Success
    return 0;
}
