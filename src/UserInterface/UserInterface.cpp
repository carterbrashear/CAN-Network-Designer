#include "UserInterface.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <jetbrains_mono.h>

#include "Window.hpp"

void UserInterface::init() const {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.Fonts->AddFontFromMemoryCompressedTTF(
        JetBrainsMono_compressed_data,
        JetBrainsMono_compressed_size,
        18.0f
    );

    if constexpr (dark_mode) {
        ImGui::StyleColorsDark();
    } else {
        ImGui::StyleColorsLight();
    }

    ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void UserInterface::begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
}

void UserInterface::end() const {
    ImGui::Render();
    glViewport(0, 0, window.getWidth(), window.getHeight());
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void UserInterface::homePanel() {
    //demo panel
    //ImGui::ShowDemoWindow();
    //ImGui::Begin("Demo Panel");
    //ImGui::Text("Hello World from Dear ImGui!");
    //ImGui::Separator();
    //ImGui::Text("GLAD & OpenGL Version: %s", glGetString(GL_VERSION));
    //ImGui::End();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("Program Title/ homepage");
    
    ImVec2 boxSize= ImVec2(300.0f, 50.0f);
    ImGui::Dummy(boxSize);

    ImVec2 pMin= ImGui::GetItemRectMin();
    ImVec2 pMax= ImGui::GetItemRectMax();

    ImDrawList* draw_list= ImGui::GetWindowDrawList();
    ImU32 boxColor= IM_COL32(255,255,255,255);
    ImU32 fillColor= IM_COL32(100,100,100,255);

    //draws rectangles
    draw_list->AddRectFilled(pMin, pMax, fillColor, 4.0f);
    draw_list->AddRect(pMin, pMax, boxColor, 4.0f, 0, 2.0f);

    //set text inside box
    ImGui::SetCursorPos(ImVec2(pMin.x + 10.0f, pMin.y + 10.0f));
    ImGui::Text("CAN Network Designer");

    ImGui::SameLine();

    //Added for Demo
    // Database file selector panel
    ImGui::SetNextWindowSize(ImVec2(1050, 200), ImGuiCond_FirstUseEver);

    //ImGui::Begin("Database File Selector");
    ImGui::SetCursorPos(ImVec2(pMax.x + 50.0f, pMin.y + 30.0f));
    if (ImGui::Button("Open Database File")) {
        selectedDBPath = getDBFilePath();
    }

    ImGui::Separator();

    ImGui::SetCursorPos(ImVec2(pMax.x + 50.0f, pMin.y + 60.0f));
    if (selectedDBPath.has_value()) {
        ImGui::Text("Selected Database:");
        ImGui::TextWrapped("%s", selectedDBPath->string().c_str());
    }
    else {
        ImGui::Text("No database file selected.");
    }

    ImGui::Separator();
    string spec;
    int test;
    for(int i=0; i<5; i++){
        //device specifications list, for demo
        ImGui:: SetCursorPos(ImVec2(io.DisplaySize.x/2 - 100, io.DisplaySize.y/2 + i*30));
        //test for device specs, replace values with ones from db
        switch(i){
            case 0:
                spec= "Device ID:";
                test= 0;
                break;
            case 1:
                spec= "Device Name";
                test= 1;
                break;
            case 2:
                spec= "Position: ";
                test= 2;
                break;
            case 3:
                spec= "Comment: ";
                test= 3;
                break;
            case 4:
                spec= "Connected Bus IDs: ";
                test= 4;
                break;
        }
        ImGui::Text("%s %d", spec.c_str(), test);
        ImGui::Separator();
    }

    ImGui::End();
} 

UserInterface::~UserInterface() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
