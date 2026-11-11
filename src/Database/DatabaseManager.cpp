#include "DatabaseManager.hpp"

bool DatabaseManager::initializeSchema(std::string_view schema_sql) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::newProject(std::string_view path, NetworkProject &project) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::loadProject(std::string_view path, NetworkProject &out_project) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::saveProject(std::string_view path, const NetworkProject &project) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::insertDevice(Device &device) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::updateDevice(const Device &device) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::deleteDevice(int64_t device_id) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::insertBus(Bus &bus) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::updateBus(const Bus &bus) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::deleteBus(int64_t bus_id) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::insertMessage(Message &message) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::updateMessage(const Message &message) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::deleteMessage(int64_t message_id) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::insertSignal(Signal &signal) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::updateSignal(const Signal &signal) {
    // TODO: IMPLEMENT THIS
    return false;
}

bool DatabaseManager::deleteSignal(int64_t signal_id) {
    // TODO: IMPLEMENT THIS
    return false;
}

