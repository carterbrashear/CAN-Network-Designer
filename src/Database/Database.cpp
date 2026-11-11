#include "Database.hpp"

Database::~Database() {
    close();
}

Database::Database(Database&& other) noexcept : handle(other.handle), last_error_message(std::move(other.last_error_message)) {
    other.handle = nullptr;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        close();
        handle = other.handle;
        last_error_message = std::move(other.last_error_message);
        other.handle = nullptr;
    }
    return *this;
}

bool Database::open(const std::string_view path) {
    close();
    const std::string path_str(path);
    const int rc = sqlite3_open(path_str.c_str(), &handle);
    if (rc != SQLITE_OK) {
        last_error_message = handle ? sqlite3_errmsg(handle) : "Failed to allocate sqlite3 handle";
        close();
        return false;
    }
    // Enable foreign keys
    execute("PRAGMA foreign_keys = ON;");
    return true;
}

void Database::close() {
    if (handle) {
        sqlite3_close(handle);
        handle = nullptr;
    }
}

bool Database::execute(const std::string_view sql) {
    if (!handle) {
        last_error_message = "Database is not open";
        return false;
    }
    char* err_msg = nullptr;
    const std::string sql_str(sql);
    const int rc = sqlite3_exec(handle, sql_str.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        last_error_message = err_msg ? err_msg : "Unknown SQLite execution error";
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

void Database::beginTransaction() {
    execute("BEGIN TRANSACTION;");
}

void Database::commitTransaction() {
    execute("COMMIT TRANSACTION;");
}

void Database::rollbackTransaction() {
    execute("ROLLBACK TRANSACTION;");
}

bool Database::isOpen() const {
    return handle != nullptr;
}

sqlite3* Database::getRawHandle() const {
    return handle;
}

std::string_view Database::getLastError() const {
    return last_error_message;
}