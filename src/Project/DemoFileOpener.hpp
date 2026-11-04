#ifndef DEMOFILEOPENER_HPP
#define DEMOFILEOPENER_HPP
#include <optional>
#include <fstream>
#include <string>
#include <filesystem>
#include "tinyfiledialogs.h"
using namespace std;
namespace fs = std::filesystem;
std::optional<fs::path> getDBFilePath();
#endif