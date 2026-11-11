#ifndef DEMOFILEOPENER_HPP
#define DEMOFILEOPENER_HPP
#include <optional>
#include <fstream>
#include <string>
#include <filesystem>
#include "tinyfiledialogs.h"
#include "../Error/Error.hpp"
using namespace std;
namespace fs = std::filesystem;
std::optional<fs::path> getDBFilePath();
bool selectedPathVaild(std::optional<fs::path> path, Error& e);
#endif