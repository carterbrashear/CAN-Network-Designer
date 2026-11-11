#include "UserInterface.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <jetbrains_mono.h>

#include "Window.hpp"

//flag to disable docking, can be applied to any window
ImGuiWindowFlags dockFlag= ImGuiWindowFlags_NoDocking;

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

bool UserInterface::homePanel() {
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::Begin("Program Title/ homepage");

    ImDrawList* draw_list= ImGui::GetWindowDrawList();
    ImU32 boxColor= IM_COL32(255,255,255,255);
    ImU32 fillColor= IM_COL32(100,100,100,255);

    //draws title rectangle
    ImVec2 boxSize= ImVec2(150.0f, 50.0f);

    ImVec2 dockedWindowSize= ImGui::GetContentRegionAvail();
    ImVec2 titleLocalPos= ImVec2((dockedWindowSize.x*3)/4, dockedWindowSize.y/2);
    ImGui::SetCursorPos(titleLocalPos);

    ImVec2 pMin= ImGui::GetCursorScreenPos();
    ImVec2 pMax= ImVec2(pMin.x + boxSize.x, pMin.y + boxSize.y);

    draw_list->AddRectFilled(pMin, pMax, fillColor, 4.0f);
    draw_list->AddRect(pMin, pMax, boxColor, 4.0f, 0, 2.0f);
    ImGui::Dummy(boxSize);

    //set text inside box
    string title= "CAN Designer";
    ImVec2 textSize= ImGui::CalcTextSize(title.c_str());
    
    ImGui::SetCursorPos(ImVec2(titleLocalPos.x + (boxSize.x - textSize.x)/2, titleLocalPos.y + (boxSize.y - textSize.y)/2));
    ImGui::PushItemWidth(dockedWindowSize.x);
    ImGui::TextWrapped("%s", title.c_str());
    ImGui::PopItemWidth();

    //Added for Demo
    // Database file selector panel
    ImGui::SetNextWindowSize(ImVec2(1050, 200), ImGuiCond_FirstUseEver);

    ImGui::SetCursorPos(ImVec2(dockedWindowSize.x/6, dockedWindowSize.y/4));
    ImGui::BeginDisabled(fileOpened);
    if (ImGui::Button("Open Database File")) {
        selectedDBPath = getDBFilePath();
    }
    ImGui::EndDisabled();

    if (fileOpened) {
        ImGui::SameLine();
        if (ImGui::Button("Close Database")) {
            db_manager.closeDatabase();
            fileOpened = false;
            selectedDBPath = std::nullopt;
            loaded_db_path.clear();
            cached_devices.clear();
            cached_buses.clear();
            cached_messages.clear();
            cached_signals.clear();
            selected_device_idx = 0;
        }
    }

    ImGui::SetCursorPos(ImVec2(dockedWindowSize.x/6 + 50.0f, (dockedWindowSize.y/4) + 40.0f));
    if (selectedDBPath.has_value() && selectedDBPath->extension() == ".db") {
        // Load database if newly selected
        if (selectedDBPath->string() != loaded_db_path) {
            NetworkProject dummy_project;
            if (db_manager.loadProject(selectedDBPath->string(), dummy_project)) {
                loaded_db_path = selectedDBPath->string();
                fileOpened = true;
                cached_devices = db_manager.getDevices();
                cached_buses = db_manager.getBuses();
                cached_messages = db_manager.getMessages();
                cached_signals = db_manager.getSignals();
                selected_device_idx = 0;
            } else {
                DrawErrorPopup("DB_ERR", db_manager.getLastError().GetError());
                selectedDBPath = std::nullopt;
                fileOpened = false;
            }
        }

        if (fileOpened) {
            ImGui::Text("Selected Database:");
            ImGui::TextWrapped("%s", selectedDBPath->filename().string().c_str());

            if (!cached_devices.empty()) {
                if (selected_device_idx < 0 || selected_device_idx >= static_cast<int>(cached_devices.size())) {
                    selected_device_idx = 0;
                }

                // Device selector controls if more than one device exists
                if (cached_devices.size() > 1) {
                    ImGui::SetCursorPos(ImVec2(dockedWindowSize.x/6, dockedWindowSize.y/2 - 40.0f));
                    if (ImGui::Button("< Prev") && selected_device_idx > 0) {
                        selected_device_idx--;
                    }
                    ImGui::SameLine();
                    ImGui::Text("Device %d of %zu", selected_device_idx + 1, cached_devices.size());
                    ImGui::SameLine();
                    if (ImGui::Button("Next >") && selected_device_idx + 1 < static_cast<int>(cached_devices.size())) {
                        selected_device_idx++;
                    }
                }

                const auto& current_dev = cached_devices[selected_device_idx];

                // Build connected bus IDs string
                std::string bus_ids_str;
                if (current_dev.connected_bus_ids.empty()) {
                    bus_ids_str = "None";
                } else {
                    for (size_t b = 0; b < current_dev.connected_bus_ids.size(); ++b) {
                        if (b > 0) bus_ids_str += ", ";
                        bus_ids_str += std::to_string(current_dev.connected_bus_ids[b]);
                    }
                }

                // Displays specs when dbpath is selected
                for (int i = 0; i < 5; i++) {
                    ImGui::SetCursorPos(ImVec2(dockedWindowSize.x/6, dockedWindowSize.y/2 + (i * 40)));
                    switch (i) {
                        case 0:
                            ImGui::Text("Device ID: %lld", static_cast<long long>(current_dev.id));
                            break;
                        case 1:
                            ImGui::Text("Device Name: %s", current_dev.name.c_str());
                            break;
                        case 2:
                            ImGui::Text("Position: (%.1f, %.1f)", current_dev.pos_x, current_dev.pos_y);
                            break;
                        case 3:
                            ImGui::Text("Comment: %s", current_dev.comment.empty() ? "(none)" : current_dev.comment.c_str());
                            break;
                        case 4:
                            ImGui::Text("Connected Bus IDs: %s", bus_ids_str.c_str());
                            break;
                    }
                }
            } else {
                ImGui::SetCursorPos(ImVec2(dockedWindowSize.x/6, dockedWindowSize.y/2));
                ImGui::Text("No devices found in this database.");
            }
        }
    }
    else if (selectedDBPath.has_value() && selectedDBPath->extension() != ".db") {
        ImGui::Text("No database file selected.");
        DrawErrorPopup("101", "Incorrect File Type Selected");
    }
    else {
        ImGui::Text("No database file selected.");
    }

    ImGui::End();

    if (fileOpened) {
        renderDatabaseInspector();
    }

    return true;
} 

void UserInterface::renderDatabaseInspector() {
    ImGui::Begin("Database Inspector");

    if (ImGui::BeginTabBar("DatabaseTabs")) {
        // Tab 1: Devices
        if (ImGui::BeginTabItem("Devices")) {
            ImGui::Text("Total Devices: %zu", cached_devices.size());
            if (ImGui::BeginTable("DevicesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Position");
                ImGui::TableSetupColumn("Connected Buses");
                ImGui::TableSetupColumn("Comment");
                ImGui::TableHeadersRow();

                for (size_t i = 0; i < cached_devices.size(); ++i) {
                    const auto& dev = cached_devices[i];
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    if (ImGui::Selectable(std::to_string(dev.id).c_str(), selected_device_idx == static_cast<int>(i), ImGuiSelectableFlags_SpanAllColumns)) {
                        selected_device_idx = static_cast<int>(i);
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", dev.name.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("(%.1f, %.1f)", dev.pos_x, dev.pos_y);

                    ImGui::TableSetColumnIndex(3);
                    std::string buses;
                    for (size_t b = 0; b < dev.connected_bus_ids.size(); ++b) {
                        if (b > 0) buses += ", ";
                        buses += std::to_string(dev.connected_bus_ids[b]);
                    }
                    ImGui::Text("%s", buses.empty() ? "-" : buses.c_str());

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%s", dev.comment.c_str());
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // Tab 2: Buses
        if (ImGui::BeginTabItem("Buses")) {
            ImGui::Text("Total Buses: %zu", cached_buses.size());
            if (ImGui::BeginTable("BusesTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Nominal Rate");
                ImGui::TableSetupColumn("Type");
                ImGui::TableSetupColumn("Color");
                ImGui::TableSetupColumn("Comment");
                ImGui::TableHeadersRow();

                for (const auto& bus : cached_buses) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%lld", static_cast<long long>(bus.id));

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", bus.name.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d bps", bus.nominal_bit_rate);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%s", bus.is_fd ? "CAN FD" : "Standard CAN");

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("0x%06X", bus.color);

                    ImGui::TableSetColumnIndex(5);
                    ImGui::Text("%s", bus.comment.c_str());
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // Tab 3: Messages
        if (ImGui::BeginTabItem("Messages")) {
            ImGui::Text("Total Messages: %zu", cached_messages.size());
            if (ImGui::BeginTable("MessagesTable", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("CAN ID");
                ImGui::TableSetupColumn("Bus ID");
                ImGui::TableSetupColumn("Transmitter");
                ImGui::TableSetupColumn("Type");
                ImGui::TableSetupColumn("DLC");
                ImGui::TableSetupColumn("Cycle (ms)");
                ImGui::TableHeadersRow();

                for (const auto& msg : cached_messages) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%lld", static_cast<long long>(msg.id));

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", msg.name.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("0x%X (%u)", msg.can_id, msg.can_id);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%lld", static_cast<long long>(msg.bus_id));

                    ImGui::TableSetColumnIndex(4);
                    if (msg.transmitter_id >= 0) {
                        ImGui::Text("%lld", static_cast<long long>(msg.transmitter_id));
                    } else {
                        ImGui::Text("-");
                    }

                    ImGui::TableSetColumnIndex(5);
                    ImGui::Text("%s", msg.is_fd ? "FD" : (msg.is_extended ? "Ext" : "Std"));

                    ImGui::TableSetColumnIndex(6);
                    ImGui::Text("%d", msg.dlc);

                    ImGui::TableSetColumnIndex(7);
                    ImGui::Text("%d", msg.cycle_time_ms);
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // Tab 4: Signals
        if (ImGui::BeginTabItem("Signals")) {
            ImGui::Text("Total Signals: %zu", cached_signals.size());
            if (ImGui::BeginTable("SignalsTable", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("Message ID");
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Start Bit");
                ImGui::TableSetupColumn("Length");
                ImGui::TableSetupColumn("Range");
                ImGui::TableSetupColumn("Unit");
                ImGui::TableSetupColumn("Comment");
                ImGui::TableHeadersRow();

                for (const auto& sig : cached_signals) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%lld", static_cast<long long>(sig.id));

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%lld", static_cast<long long>(sig.message_id));

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", sig.name.c_str());

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", sig.start_bit);

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%d", sig.bit_length);

                    ImGui::TableSetColumnIndex(5);
                    ImGui::Text("[%.1f, %.1f]", sig.min_value, sig.max_value);

                    ImGui::TableSetColumnIndex(6);
                    ImGui::Text("%s", sig.unit.c_str());

                    ImGui::TableSetColumnIndex(7);
                    ImGui::Text("%s", sig.comment.c_str());
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void UserInterface::DrawErrorPopup(string popup_id, string error_message)
{
    ImGui::OpenPopup(popup_id.c_str());
    ImGui::SetNextWindowSize(ImVec2(300, 0));
    if (ImGui::BeginPopupModal(popup_id.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("%s", error_message.c_str());
        ImGui::Separator();
        ImGui::SetCursorPosX(ImGui::GetWindowSize().x - 85);
        if (ImGui::Button("OK", ImVec2(75, 0)))
        {
            ImGui::CloseCurrentPopup();
            //should be removed for general use later
            selectedDBPath = nullopt;
        }

        ImGui::EndPopup();
    }
}

void UserInterface::toolbar(){}

void UserInterface::workspace(){
    ImGui::Begin("Workspace");

    ImGui::End();
}

//no implementation of movable devices yet
void UserInterface::deviceUI(){

}

UserInterface::~UserInterface() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
