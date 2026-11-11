#pragma once
#include "Window.hpp"
//added for demo
#include <optional>
#include <filesystem>
#include <string>
#include <vector>
#include "../Project/DemoFileOpener.hpp"
#include "../Database/DatabaseManager.hpp"
namespace fs = std::filesystem;

class UserInterface {
    static constexpr bool dark_mode = true;
    Window& window;
    //added for demo
    std::optional<fs::path> selectedDBPath;
    bool fileOpened = false;

    DatabaseManager db_manager;
    std::string loaded_db_path;
    std::vector<Device> cached_devices;
    std::vector<Bus> cached_buses;
    std::vector<MessageInfo> cached_messages;
    std::vector<SignalInfo> cached_signals;
    int selected_device_idx = 0;

    void renderDatabaseInspector();

public:
    explicit UserInterface(Window& win) : window(win) {}

    void init() const;

    static void begin();
    void end() const;
    //changed from static for demo, will be used as the home panel created upon startup later
    bool homePanel();

    void DrawErrorPopup(string popup_id, string error_message);

    void toolbar();

    void workspace();

    void deviceUI();

    ~UserInterface();
};
