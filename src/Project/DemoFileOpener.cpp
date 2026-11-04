#include "DemoFileOpener.hpp"
optional<fs::path> getDBFilePath() {
    char const* filterPatterns[1] = { "*.db" };
    char const* filePathStr = tinyfd_openFileDialog("Choose a file to open", "", 1, filterPatterns, "DB files", 0);

    // Check if the user selected a file or cancelled
    if (filePathStr == nullptr) {
        return nullopt;
    }

    // Convert the C-style string to a filesystem path
    fs::path selectedPath(filePathStr);
    return selectedPath;
}