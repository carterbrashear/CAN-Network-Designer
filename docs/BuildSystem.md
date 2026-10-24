# Build System Documentation
## GLFW Dependency
cmake will check to see if the system GLFW library 3.3.8 or newer is installed. If not, we ues FetchContent to clone the github repository for version 3.5.1 and compile it.
This method of resolving dependencies is more robust for different people's development environments. On Windows just MinGW or Visual C++ should be good enough to compile GLFW.
- Debian and Ubuntu: `sudo apt install libwayland-dev libxkbcommon-dev xorg-dev`
- Fedora: `sudo dnf install wayland-devel libxkbcommon-devel libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel`
- FreeBSD: `pkg install wayland libxkbcommon evdev-proto xorgproto`
## SQLite Dependency
First we check if SQLite3 system package is installed. If it is not then we use FetchContent to download and extract the amalgamation zip file from the website. The zip file contains `sqlite3.c` which can be compiled and linked as a libarary. We also add the header files into the include directories. SQLite's build dependencies are minimal and included on most machines with a working C compiler.
## Dear ImGui Dependency
This one skips searching for system libraries because they don't exist from my understanding. It just grabs the latest from GitHub on the docking branch. The docking branch is needed because it includes the critical feature of being able to dock elements like windows in most OS UIs. In the future it would be best to pin this to a particular commit to prevent new updates from breaking changes on updates.
