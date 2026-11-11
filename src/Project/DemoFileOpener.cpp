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
bool selectedPathVaild(std::optional<fs::path> path, Error& e) {
    if (!path.has_value()) {
        e.setMessage("The Slected Path Is Invalid");
        e.setID("FO");
        e.setCode(101);
        return 1;
    }
    else if (path->extension() != ".db") {
        e.setMessage("The File Chosen Is The Wrong Type");
        e.setID("FO");
        e.setCode(102);
        return 1;
    }
    e.setMessage(path->filename().string() + "Was Chosen");
    e.setID("FO");
    e.setCode(0);
    return 0;
}