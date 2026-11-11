#pragma once

#include "Database.hpp"
#include "../Project/NetworkProject.hpp"
#include "../Error/Error.hpp"
#include <string_view>
#include <string>
#include <vector>
#include <cstdint>

struct MessageInfo {
    int64_t id = 0;
    int64_t bus_id = 0;
    int64_t transmitter_id = -1; // -1 if NULL
    uint32_t can_id = 0;
    bool is_extended = false;
    bool is_fd = false;
    int dlc = 0;
    std::string name;
    int cycle_time_ms = 0;
    std::string comment;
};

struct SignalInfo {
    int64_t id = 0;
    int64_t message_id = 0;
    std::string name;
    int start_bit = 0;
    int bit_length = 0;
    int byte_order = 0;
    bool is_signed = false;
    double factor = 1.0;
    double offset = 0.0;
    double min_value = 0.0;
    double max_value = 0.0;
    std::string unit;
    int mux_type = 0;
    std::string comment;
};

class DatabaseManager {
    Database db;
    std::string current_file_path;
    Error last_error;

    static const char* getDefaultSchemaSql();
    static const char* getDefaultSeedSql();

public:
    DatabaseManager();
    ~DatabaseManager() = default;

    bool initializeSchema(std::string_view schema_sql = "");
    bool createDemoDatabase(std::string_view path = "demo.db");

    bool openDatabase(std::string_view path);
    void closeDatabase();
    bool isOpen() const;

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

    // Query methods
    std::vector<Device> getDevices();
    std::vector<Bus> getBuses();
    std::vector<MessageInfo> getMessages();
    std::vector<SignalInfo> getSignals();

    const std::string& getCurrentFilePath() const { return current_file_path; }
    Error getLastError() const { return last_error; }
    Database& getDatabase() { return db; }
};
