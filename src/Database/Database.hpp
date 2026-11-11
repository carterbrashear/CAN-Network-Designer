#pragma once
#pragma once

#include <sqlite3.h>
#include <string_view>
#include <string>

class Database {
    sqlite3 *handle = nullptr;
    std::string last_error_message;
public:
    Database() = default;
    ~Database();

    // Prevent copying to maintain single ownership of handle
    Database(const Database &);
    Database &operator=(const Database &);

    // Allow moving
    Database(Database &&other) noexcept;
    Database &operator=(Database &&other) noexcept;

    bool open(std::string_view path);
    void close();

    bool execute(std::string_view sql);
    void beginTransaction();
    void commitTransaction();
    void rollbackTransaction();

    bool isOpen() const;
    sqlite3 *getRawHandle() const;
    std::string_view getLastError() const;
};
