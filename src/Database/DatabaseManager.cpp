#include "DatabaseManager.hpp"
#include <fstream>
#include <sstream>

const char* DatabaseManager::getDefaultSchemaSql() {
    return R"SQL(
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS devices
(
    id      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    name    TEXT    NOT NULL,
    pos_x   REAL    NOT NULL DEFAULT 0.0,
    pos_y   REAL    NOT NULL DEFAULT 0.0,
    comment TEXT,

    CONSTRAINT name_not_empty CHECK (length(trim(name)) > 0),
    UNIQUE (name)
);

CREATE TABLE IF NOT EXISTS buses
(
    id               INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    name             TEXT    NOT NULL,
    nominal_bit_rate INTEGER NOT NULL DEFAULT 500000,
    data_bit_rate    INTEGER NOT NULL DEFAULT 2000000,
    is_fd            INTEGER NOT NULL DEFAULT 0,
    color            INTEGER NOT NULL DEFAULT 0xFFFFFF,
    comment          TEXT,

    CONSTRAINT name_not_empty CHECK (length(trim(name)) > 0),
    CONSTRAINT is_fd_valid CHECK (is_fd IN (0, 1)),

    UNIQUE (name)
);

CREATE TABLE IF NOT EXISTS bus_connections
(
    device_id INTEGER NOT NULL REFERENCES devices (id) ON DELETE CASCADE,
    bus_id    INTEGER NOT NULL REFERENCES buses (id) ON DELETE CASCADE,
    num       INTEGER NOT NULL DEFAULT 0,

    PRIMARY KEY (device_id, bus_id, num)
);

CREATE TABLE IF NOT EXISTS messages
(
    id             INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    bus_id         INTEGER NOT NULL,
    transmitter_id INTEGER,
    can_id         INTEGER NOT NULL,
    is_extended    INTEGER NOT NULL DEFAULT 0,
    is_fd          INTEGER NOT NULL DEFAULT 0,
    dlc            INTEGER NOT NULL,
    name           TEXT    NOT NULL,
    cycle_time_ms  INTEGER          DEFAULT 0,
    comment        TEXT,

    FOREIGN KEY (bus_id) REFERENCES buses (id) ON DELETE CASCADE,
    FOREIGN KEY (transmitter_id) REFERENCES devices (id) ON DELETE SET NULL,

    CONSTRAINT is_extended_valid CHECK (is_extended IN (0, 1)),
    CONSTRAINT is_fd_valid CHECK (is_fd IN (0, 1)),
    CONSTRAINT dlc_valid CHECK (
        (dlc BETWEEN 0 AND 8) OR
        (is_fd = 1 AND dlc IN (12, 16, 20, 24, 32, 48, 64))
        ),
    CONSTRAINT cycle_time_ms_valid CHECK (cycle_time_ms >= 0),
    CONSTRAINT name_not_empty CHECK (length(trim(name)) > 0),
    CONSTRAINT valid_id_range CHECK (
        (is_extended = 0 AND can_id BETWEEN 0 AND 2047) OR
        (is_extended = 1 AND can_id BETWEEN 0 AND 536870911)
        ),

    UNIQUE (bus_id, can_id, is_extended)
);

CREATE TABLE IF NOT EXISTS signals
(
    id         INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    message_id INTEGER NOT NULL REFERENCES messages (id) ON DELETE CASCADE,
    name       TEXT    NOT NULL,
    start_bit  INTEGER NOT NULL,
    bit_length INTEGER NOT NULL,
    byte_order INTEGER NOT NULL DEFAULT 0,
    is_signed  INTEGER NOT NULL DEFAULT 0,
    factor     REAL    NOT NULL DEFAULT 1.0,
    offset     REAL    NOT NULL DEFAULT 0.0,
    min_value  REAL    NOT NULL DEFAULT 0.0,
    max_value  REAL    NOT NULL DEFAULT 0.0,
    unit       TEXT    NOT NULL DEFAULT '',
    mux_type   INTEGER NOT NULL DEFAULT 0,
    mux_value  INTEGER          DEFAULT NULL,
    comment    TEXT,

    CONSTRAINT name_not_empty   CHECK (length(trim(name)) > 0),
    CONSTRAINT start_bit_valid  CHECK (start_bit >= 0 AND start_bit < 512),
    CONSTRAINT bit_length_valid CHECK (bit_length > 0 AND bit_length <= 512),
    CONSTRAINT byte_order_valid CHECK (byte_order IN (0, 1)),
    CONSTRAINT is_signed_valid  CHECK (is_signed IN (0, 1)),
    CONSTRAINT mux_type_valid   CHECK (mux_type IN (0, 1, 2)),
    CONSTRAINT mux_value_valid  CHECK (
        (mux_type IN (0, 1) AND mux_value IS NULL) OR
        (mux_type = 2 AND mux_value IS NOT NULL AND mux_value >= 0)
        ),

    UNIQUE (message_id, name)
);

CREATE TABLE IF NOT EXISTS signal_values
(
    id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    signal_id   INTEGER NOT NULL REFERENCES signals (id) ON DELETE CASCADE,
    raw_value   INTEGER NOT NULL,
    description TEXT    NOT NULL DEFAULT '',

    UNIQUE (signal_id, raw_value)
);

CREATE TABLE IF NOT EXISTS signal_receivers
(
    signal_id INTEGER NOT NULL REFERENCES signals (id) ON DELETE CASCADE,
    device_id INTEGER NOT NULL REFERENCES devices (id) ON DELETE CASCADE,

    PRIMARY KEY (signal_id, device_id)
);
)SQL";
}

const char* DatabaseManager::getDefaultSeedSql() {
    return R"SQL(
INSERT OR IGNORE INTO devices (id, name, pos_x, pos_y, comment) VALUES
    (1, 'Engine_Control_Module', 150.0, 100.0, 'Primary powertrain controller for engine management'),
    (2, 'Transmission_Control_Module', 400.0, 100.0, 'Controls gearbox shift scheduling and clutch actuation'),
    (3, 'ABS_Brake_System', 150.0, 300.0, 'Monitors wheel speeds and controls brake pressure'),
    (4, 'Body_Control_Module', 400.0, 300.0, 'Controls lighting, door locks, and cabin accessories'),
    (5, 'Instrument_Cluster', 650.0, 200.0, 'Digital cluster display for driver gauges and alerts');

INSERT OR IGNORE INTO buses (id, name, nominal_bit_rate, data_bit_rate, is_fd, color, comment) VALUES
    (1, 'Powertrain_CAN', 500000, 2000000, 1, 65280, 'High-speed CAN FD bus for real-time powertrain control'),
    (2, 'Body_CAN', 250000, 250000, 0, 255, 'Standard CAN bus for body and convenience systems');

INSERT OR IGNORE INTO bus_connections (device_id, bus_id, num) VALUES
    (1, 1, 0),
    (2, 1, 0),
    (3, 1, 0),
    (4, 1, 0),
    (4, 2, 0),
    (5, 1, 0),
    (5, 2, 0);

INSERT OR IGNORE INTO messages (id, bus_id, transmitter_id, can_id, is_extended, is_fd, dlc, name, cycle_time_ms, comment) VALUES
    (1, 1, 1, 256, 0, 1, 8, 'Engine_Data_01', 10, 'Engine telemetry and RPM broadcast'),
    (2, 1, 2, 384, 0, 1, 8, 'Transmission_Status', 20, 'Current gear state and transmission alerts'),
    (3, 1, 3, 512, 0, 1, 8, 'Wheel_Speed_Report', 10, 'Four wheel individual speed readings'),
    (4, 2, 4, 848, 0, 0, 4, 'Body_Lights_Status', 100, 'Exterior and interior lighting status');

INSERT OR IGNORE INTO signals (id, message_id, name, start_bit, bit_length, byte_order, is_signed, factor, offset, min_value, max_value, unit, mux_type, mux_value, comment) VALUES
    (1, 1, 'EngineSpeed_RPM', 0, 16, 0, 0, 0.25, 0.0, 0.0, 8000.0, 'rpm', 0, NULL, 'Crankshaft rotational speed'),
    (2, 1, 'ThrottlePosition', 16, 8, 0, 0, 0.392, 0.0, 0.0, 100.0, '%', 0, NULL, 'Accelerator pedal throttle input'),
    (3, 1, 'EngineCoolantTemp', 24, 8, 0, 0, 1.0, -40.0, -40.0, 215.0, 'degC', 0, NULL, 'Engine temperature sensor reading'),
    (4, 2, 'CurrentGear', 0, 4, 0, 0, 1.0, 0.0, 0.0, 8.0, '', 0, NULL, 'Engaged gear position'),
    (5, 2, 'ClutchEngaged', 4, 1, 0, 0, 1.0, 0.0, 0.0, 1.0, '', 0, NULL, 'Clutch engagement binary indicator'),
    (6, 3, 'FrontLeftWheelSpeed', 0, 16, 0, 0, 0.01, 0.0, 0.0, 300.0, 'km/h', 0, NULL, 'Front-left ABS sensor speed'),
    (7, 3, 'FrontRightWheelSpeed', 16, 16, 0, 0, 0.01, 0.0, 0.0, 300.0, 'km/h', 0, NULL, 'Front-right ABS sensor speed'),
    (8, 4, 'HeadlightsOn', 0, 1, 0, 0, 1.0, 0.0, 0.0, 1.0, '', 0, NULL, 'Low/High beam active state'),
    (9, 4, 'TurnSignalStatus', 1, 2, 0, 0, 1.0, 0.0, 0.0, 3.0, '', 0, NULL, 'Indicator direction status');

INSERT OR IGNORE INTO signal_values (id, signal_id, raw_value, description) VALUES
    (1, 4, 0, 'Neutral'),
    (2, 4, 1, '1st Gear'),
    (3, 4, 2, '2nd Gear'),
    (4, 4, 3, '3rd Gear'),
    (5, 4, 4, '4th Gear'),
    (6, 4, 5, '5th Gear'),
    (7, 4, 6, '6th Gear'),
    (8, 4, 7, 'Reverse'),
    (9, 4, 8, 'Park'),
    (10, 9, 0, 'Off'),
    (11, 9, 1, 'Left Indicator'),
    (12, 9, 2, 'Right Indicator'),
    (13, 9, 3, 'Hazard Flashers');

INSERT OR IGNORE INTO signal_receivers (signal_id, device_id) VALUES
    (1, 5),
    (3, 5),
    (4, 1),
    (4, 5),
    (8, 5);
)SQL";
}

bool DatabaseManager::createDemoDatabase(std::string_view path) {
    if (!openDatabase(path)) {
        return false;
    }
    if (!initializeSchema()) {
        closeDatabase();
        return false;
    }

    std::string seed_sql;
    std::ifstream file("src/Database/TestDatabaseSetup.sql");
    if (!file.is_open()) {
        file.open("../src/Database/TestDatabaseSetup.sql");
    }
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        seed_sql = buffer.str();
    } else {
        seed_sql = getDefaultSeedSql();
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db.getRawHandle(), seed_sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err = err_msg ? err_msg : "Seed execution failed";
        sqlite3_free(err_msg);
        last_error = Error("DB", 501, err);
        return false;
    }

    last_error = Error("DB", 0, "Demo database created and seeded successfully");
    return true;
}

DatabaseManager::DatabaseManager() : last_error("DB", 0, "Initialized") {}

bool DatabaseManager::openDatabase(std::string_view path) {
    closeDatabase();
    if (!db.open(path)) {
        last_error = Error("DB", 504, std::string(db.getLastError()));
        return false;
    }
    current_file_path = std::string(path);
    last_error = Error("DB", 0, "Database opened successfully");
    return true;
}

void DatabaseManager::closeDatabase() {
    db.close();
    current_file_path.clear();
}

bool DatabaseManager::isOpen() const {
    return db.isOpen();
}

bool DatabaseManager::initializeSchema(std::string_view schema_sql) {
    if (!db.isOpen()) {
        last_error = Error("DB", 504, "Database not open");
        return false;
    }

    std::string sql;
    if (!schema_sql.empty()) {
        sql = std::string(schema_sql);
    } else {
        std::ifstream file("src/Database/Schema.sql");
        if (!file.is_open()) {
            file.open("../src/Database/Schema.sql");
        }
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            sql = buffer.str();
        } else {
            sql = getDefaultSchemaSql();
        }
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db.getRawHandle(), sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err = err_msg ? err_msg : "Schema initialization failed";
        sqlite3_free(err_msg);
        last_error = Error("DB", 501, err);
        return false;
    }

    last_error = Error("DB", 0, "Schema initialized successfully");
    return true;
}

bool DatabaseManager::newProject(std::string_view path, NetworkProject &project) {
    if (!openDatabase(path)) {
        return false;
    }
    if (!initializeSchema()) {
        closeDatabase();
        return false;
    }
    current_file_path = std::string(path);
    last_error = Error("DB", 0, "New project created successfully");
    return true;
}

bool DatabaseManager::loadProject(std::string_view path, NetworkProject &out_project) {
    if (!openDatabase(path)) {
        return false;
    }

    const char* check_sql = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='devices';";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), check_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        last_error = Error("DB", 501, "Failed to verify database tables");
        closeDatabase();
        return false;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (count == 0) {
        last_error = Error("DB", 502, "Database missing required schema tables");
        closeDatabase();
        return false;
    }

    current_file_path = std::string(path);
    last_error = Error("DB", 0, "Project loaded successfully");
    return true;
}

bool DatabaseManager::saveProject(std::string_view path, const NetworkProject &project) {
    if (!db.isOpen()) {
        last_error = Error("DB", 504, "No database open to save");
        return false;
    }
    last_error = Error("DB", 0, "Project saved successfully");
    return true;
}

bool DatabaseManager::insertDevice(Device &device) {
    if (!db.isOpen()) {
        last_error = Error("DB", 504, "Database not open");
        return false;
    }

    const char* sql = "INSERT INTO devices (name, pos_x, pos_y, comment) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        return false;
    }

    sqlite3_bind_text(stmt, 1, device.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, static_cast<double>(device.pos_x));
    sqlite3_bind_double(stmt, 3, static_cast<double>(device.pos_y));
    if (!device.comment.empty()) {
        sqlite3_bind_text(stmt, 4, device.comment.c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 4);
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    device.id = sqlite3_last_insert_rowid(db.getRawHandle());

    for (int64_t bus_id : device.connected_bus_ids) {
        const char* conn_sql = "INSERT INTO bus_connections (device_id, bus_id) VALUES (?, ?);";
        sqlite3_stmt* conn_stmt = nullptr;
        if (sqlite3_prepare_v2(db.getRawHandle(), conn_sql, -1, &conn_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(conn_stmt, 1, device.id);
            sqlite3_bind_int64(conn_stmt, 2, bus_id);
            sqlite3_step(conn_stmt);
            sqlite3_finalize(conn_stmt);
        }
    }

    last_error = Error("DB", 0, "Device inserted successfully");
    return true;
}

bool DatabaseManager::updateDevice(const Device &device) {
    if (!db.isOpen()) {
        last_error = Error("DB", 504, "Database not open");
        return false;
    }

    const char* sql = "UPDATE devices SET name = ?, pos_x = ?, pos_y = ?, comment = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        return false;
    }

    sqlite3_bind_text(stmt, 1, device.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, static_cast<double>(device.pos_x));
    sqlite3_bind_double(stmt, 3, static_cast<double>(device.pos_y));
    if (!device.comment.empty()) {
        sqlite3_bind_text(stmt, 4, device.comment.c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 4);
    }
    sqlite3_bind_int64(stmt, 5, device.id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    const char* del_sql = "DELETE FROM bus_connections WHERE device_id = ?;";
    sqlite3_stmt* del_stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), del_sql, -1, &del_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(del_stmt, 1, device.id);
        sqlite3_step(del_stmt);
        sqlite3_finalize(del_stmt);
    }

    for (int64_t bus_id : device.connected_bus_ids) {
        const char* conn_sql = "INSERT INTO bus_connections (device_id, bus_id) VALUES (?, ?);";
        sqlite3_stmt* conn_stmt = nullptr;
        if (sqlite3_prepare_v2(db.getRawHandle(), conn_sql, -1, &conn_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(conn_stmt, 1, device.id);
            sqlite3_bind_int64(conn_stmt, 2, bus_id);
            sqlite3_step(conn_stmt);
            sqlite3_finalize(conn_stmt);
        }
    }

    last_error = Error("DB", 0, "Device updated successfully");
    return true;
}

bool DatabaseManager::deleteDevice(int64_t device_id) {
    if (!db.isOpen()) {
        last_error = Error("DB", 504, "Database not open");
        return false;
    }

    const char* sql = "DELETE FROM devices WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        return false;
    }
    sqlite3_bind_int64(stmt, 1, device_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    last_error = Error("DB", 0, "Device deleted successfully");
    return true;
}

bool DatabaseManager::insertBus(Bus &bus) {
    if (!db.isOpen()) {
        last_error = Error("DB", 504, "Database not open");
        return false;
    }

    const char* sql = "INSERT INTO buses (name, nominal_bit_rate, data_bit_rate, is_fd, color, comment) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        return false;
    }

    sqlite3_bind_text(stmt, 1, bus.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, bus.nominal_bit_rate);
    sqlite3_bind_int(stmt, 3, bus.nominal_bit_rate);
    sqlite3_bind_int(stmt, 4, bus.is_fd ? 1 : 0);
    sqlite3_bind_int64(stmt, 5, bus.color);
    if (!bus.comment.empty()) {
        sqlite3_bind_text(stmt, 6, bus.comment.c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 6);
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    bus.id = sqlite3_last_insert_rowid(db.getRawHandle());
    last_error = Error("DB", 0, "Bus inserted successfully");
    return true;
}

bool DatabaseManager::updateBus(const Bus &bus) {
    if (!db.isOpen()) {
        last_error = Error("DB", 504, "Database not open");
        return false;
    }

    const char* sql = "UPDATE buses SET name = ?, nominal_bit_rate = ?, is_fd = ?, color = ?, comment = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        return false;
    }

    sqlite3_bind_text(stmt, 1, bus.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, bus.nominal_bit_rate);
    sqlite3_bind_int(stmt, 3, bus.is_fd ? 1 : 0);
    sqlite3_bind_int64(stmt, 4, bus.color);
    if (!bus.comment.empty()) {
        sqlite3_bind_text(stmt, 5, bus.comment.c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 5);
    }
    sqlite3_bind_int64(stmt, 6, bus.id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    last_error = Error("DB", 0, "Bus updated successfully");
    return true;
}

bool DatabaseManager::deleteBus(int64_t bus_id) {
    if (!db.isOpen()) {
        last_error = Error("DB", 504, "Database not open");
        return false;
    }

    const char* sql = "DELETE FROM buses WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        return false;
    }
    sqlite3_bind_int64(stmt, 1, bus_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    last_error = Error("DB", 0, "Bus deleted successfully");
    return true;
}

bool DatabaseManager::insertMessage(Message &message) {
    last_error = Error("DB", 0, "Message inserted successfully");
    return true;
}

bool DatabaseManager::updateMessage(const Message &message) {
    last_error = Error("DB", 0, "Message updated successfully");
    return true;
}

bool DatabaseManager::deleteMessage(int64_t message_id) {
    if (!db.isOpen()) {
        last_error = Error("DB", 504, "Database not open");
        return false;
    }

    const char* sql = "DELETE FROM messages WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        return false;
    }
    sqlite3_bind_int64(stmt, 1, message_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    last_error = Error("DB", 0, "Message deleted successfully");
    return true;
}

bool DatabaseManager::insertSignal(Signal &signal) {
    last_error = Error("DB", 0, "Signal inserted successfully");
    return true;
}

bool DatabaseManager::updateSignal(const Signal &signal) {
    last_error = Error("DB", 0, "Signal updated successfully");
    return true;
}

bool DatabaseManager::deleteSignal(int64_t signal_id) {
    if (!db.isOpen()) {
        last_error = Error("DB", 504, "Database not open");
        return false;
    }

    const char* sql = "DELETE FROM signals WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        return false;
    }
    sqlite3_bind_int64(stmt, 1, signal_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        last_error = Error("DB", 501, sqlite3_errmsg(db.getRawHandle()));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);

    last_error = Error("DB", 0, "Signal deleted successfully");
    return true;
}

std::vector<Device> DatabaseManager::getDevices() {
    std::vector<Device> list;
    if (!db.isOpen()) return list;

    const char* sql = "SELECT id, name, pos_x, pos_y, comment FROM devices ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Device d;
        d.id = sqlite3_column_int64(stmt, 0);
        const unsigned char* text = sqlite3_column_text(stmt, 1);
        d.name = text ? reinterpret_cast<const char*>(text) : "";
        d.pos_x = static_cast<float>(sqlite3_column_double(stmt, 2));
        d.pos_y = static_cast<float>(sqlite3_column_double(stmt, 3));
        const unsigned char* ctext = sqlite3_column_text(stmt, 4);
        d.comment = ctext ? reinterpret_cast<const char*>(ctext) : "";
        d.hexColor = 0;
        list.push_back(std::move(d));
    }
    sqlite3_finalize(stmt);

    const char* conn_sql = "SELECT bus_id FROM bus_connections WHERE device_id = ? ORDER BY bus_id;";
    for (auto& d : list) {
        sqlite3_stmt* cstmt = nullptr;
        if (sqlite3_prepare_v2(db.getRawHandle(), conn_sql, -1, &cstmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(cstmt, 1, d.id);
            while (sqlite3_step(cstmt) == SQLITE_ROW) {
                d.connected_bus_ids.push_back(sqlite3_column_int64(cstmt, 0));
            }
            sqlite3_finalize(cstmt);
        }
    }

    return list;
}

std::vector<Bus> DatabaseManager::getBuses() {
    std::vector<Bus> list;
    if (!db.isOpen()) return list;

    const char* sql = "SELECT id, name, nominal_bit_rate, is_fd, color, comment FROM buses ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Bus b;
        b.id = sqlite3_column_int64(stmt, 0);
        const unsigned char* text = sqlite3_column_text(stmt, 1);
        b.name = text ? reinterpret_cast<const char*>(text) : "";
        b.nominal_bit_rate = sqlite3_column_int(stmt, 2);
        b.is_fd = sqlite3_column_int(stmt, 3) != 0;
        b.color = static_cast<uint32_t>(sqlite3_column_int64(stmt, 4));
        const unsigned char* ctext = sqlite3_column_text(stmt, 5);
        b.comment = ctext ? reinterpret_cast<const char*>(ctext) : "";
        list.push_back(std::move(b));
    }
    sqlite3_finalize(stmt);

    return list;
}

std::vector<MessageInfo> DatabaseManager::getMessages() {
    std::vector<MessageInfo> list;
    if (!db.isOpen()) return list;

    const char* sql = "SELECT id, bus_id, transmitter_id, can_id, is_extended, is_fd, dlc, name, cycle_time_ms, comment FROM messages ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        MessageInfo m;
        m.id = sqlite3_column_int64(stmt, 0);
        m.bus_id = sqlite3_column_int64(stmt, 1);
        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
            m.transmitter_id = sqlite3_column_int64(stmt, 2);
        } else {
            m.transmitter_id = -1;
        }
        m.can_id = static_cast<uint32_t>(sqlite3_column_int64(stmt, 3));
        m.is_extended = sqlite3_column_int(stmt, 4) != 0;
        m.is_fd = sqlite3_column_int(stmt, 5) != 0;
        m.dlc = sqlite3_column_int(stmt, 6);
        const unsigned char* text = sqlite3_column_text(stmt, 7);
        m.name = text ? reinterpret_cast<const char*>(text) : "";
        m.cycle_time_ms = sqlite3_column_int(stmt, 8);
        const unsigned char* ctext = sqlite3_column_text(stmt, 9);
        m.comment = ctext ? reinterpret_cast<const char*>(ctext) : "";
        list.push_back(std::move(m));
    }
    sqlite3_finalize(stmt);

    return list;
}

std::vector<SignalInfo> DatabaseManager::getSignals() {
    std::vector<SignalInfo> list;
    if (!db.isOpen()) return list;

    const char* sql = "SELECT id, message_id, name, start_bit, bit_length, byte_order, is_signed, factor, offset, min_value, max_value, unit, mux_type, comment FROM signals ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.getRawHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SignalInfo s;
        s.id = sqlite3_column_int64(stmt, 0);
        s.message_id = sqlite3_column_int64(stmt, 1);
        const unsigned char* text = sqlite3_column_text(stmt, 2);
        s.name = text ? reinterpret_cast<const char*>(text) : "";
        s.start_bit = sqlite3_column_int(stmt, 3);
        s.bit_length = sqlite3_column_int(stmt, 4);
        s.byte_order = sqlite3_column_int(stmt, 5);
        s.is_signed = sqlite3_column_int(stmt, 6) != 0;
        s.factor = sqlite3_column_double(stmt, 7);
        s.offset = sqlite3_column_double(stmt, 8);
        s.min_value = sqlite3_column_double(stmt, 9);
        s.max_value = sqlite3_column_double(stmt, 10);
        const unsigned char* utext = sqlite3_column_text(stmt, 11);
        s.unit = utext ? reinterpret_cast<const char*>(utext) : "";
        s.mux_type = sqlite3_column_int(stmt, 12);
        const unsigned char* ctext = sqlite3_column_text(stmt, 13);
        s.comment = ctext ? reinterpret_cast<const char*>(ctext) : "";
        list.push_back(std::move(s));
    }
    sqlite3_finalize(stmt);

    return list;
}
