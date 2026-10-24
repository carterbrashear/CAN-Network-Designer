#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <sqlite3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <string>

// Helper function to fetch a message from an in-memory SQLite database
std::string get_sqlite_hello_message() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        return "Failed to open SQLite in-memory database.";
    }

    const char* create_query = "CREATE TABLE greetings (msg TEXT);";
    const char* insert_query = "INSERT INTO greetings VALUES ('Hello World from SQLite!');";

    sqlite3_exec(db, create_query, nullptr, nullptr, nullptr);
    sqlite3_exec(db, insert_query, nullptr, nullptr, nullptr);

    std::string result = "No greeting found.";
    sqlite3_stmt* stmt = nullptr;
    const char* select_query = "SELECT msg FROM greetings LIMIT 1;";

    if (sqlite3_prepare_v2(db, select_query, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
    return result;
}

int main() {
    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Configure OpenGL 3.3 Core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // 2. Initialize GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 3. Fetch data using SQLite
    std::string sqlite_message = get_sqlite_hello_message();

    // 4. Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 5. Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render Hello World GUI
        {
            ImGui::Begin("Demo Panel");
            ImGui::Text("Hello World from Dear ImGui!");
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Database Output: %s", sqlite_message.c_str());
            ImGui::Text("GLAD & OpenGL Version: %s", glGetString(GL_VERSION));
            ImGui::End();
        }

        // Render draw data
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}