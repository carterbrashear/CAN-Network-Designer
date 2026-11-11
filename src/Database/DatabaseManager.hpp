#pragma once

#include "Database.hpp"
#include "../Project/NetworkProject.hpp"
#include "../Error/Error.hpp"
#include <string_view>
#include <string>

class DatabaseManager {
    Database db;
    std::string current_file_path;
    Error last_error;

public:
    DatabaseManager();
    ~DatabaseManager() = default;

    bool initializeSchema(std::string_view schema_sql = "");

    bool newProject(std::string_view path, NetworkProject& project);
    bool loadProject(std::string_view path, NetworkProject& out_project);
    bool saveProject(std::string_view path, const NetworkProject& project);

    // Entity CRUD methods
    bool insertDevice(Device& device);
    bool updateDevice(const Device& device);
    bool deleteDevice(int64_t device_id);

    bool insertBus(Bus& bus);
    bool updateBus(const Bus& bus);
    bool deleteBus(int64_t bus_id);

    bool insertMessage(Message& message);
    bool updateMessage(const Message& message);
    bool deleteMessage(int64_t message_id);

    bool insertSignal(Signal& signal);
    bool updateSignal(const Signal& signal);
    bool deleteSignal(int64_t signal_id);

    const std::string& getCurrentFilePath() const { return current_file_path; }
    const Error& getLastError() const { return last_error; }
    Database& getDatabase() { return db; }
};