# Building CAN Network Designer
Create build files with `cmake -B build`. Then build with `cmake --build build`.
Then your executable will be `build/Can-Network-Designer` or `build/Can-Network-Designer.exe`.
## Dependencies
### Windows Build Dependencies (Windows 7 or later)
- **cmake** 3.25.1 or newer
- **MinGW** or **Microsoft Visual C++**

The rest is handled by cmake and uses the standard windows libraries `opengl32.lib`, `gdi32.lib`, and `shell32.lib`.
### MacOS
Brew helps to satisfy dependencies.

- **xcode:** `xcode-select --install`
- **cmake, git, and sqlite:** `brew install cmake git glfw sqlite`
### Debian and Ubuntu
```shell
sudo apt update && sudo apt install -y \
    build-essential \
    cmake \
    git \
    libgl1-mesa-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libwayland-dev \
    libwayland-bin \
    libxkbcommon-dev \
    libsqlite3-dev \
    libglfw3-dev
```
### Fedora, RHEL, and AlmaLinux
```shell
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    make \
    mesa-libGL-devel \
    libX11-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXcursor-devel \
    libXi-devel \
    wayland-devel \
    wayland-protocols-devel \
    libxkbcommon-devel \
    sqlite-devel \
    glfw-devel
```
### Arch and Manjaro
```shell
sudo pacman -S --needed \
    base-devel \
    cmake \
    git \
    mesa \
    libx11 \
    libxrandr \
    libxinerama \
    libxcursor \
    libxi \
    wayland \
    wayland-protocols \
    libxkbcommon \
    sqlite \
    glfw-x11
```
### FreeBSD
```shell
sudo pkg install \
    cmake \
    git \
    llvm \
    mesa-libs \
    libX11 \
    libXrandr \
    libXinerama \
    libXcursor \
    libXi \
    libxkbcommon \
    sqlite3 \
    glfw
```
### OpenBSD
You might need to tell OpenBSD where to find the X11 headers and disable wayland. Not sure how to handle that.
```shell
doas pkg_add \
    cmake \
    git \
    llvm \
    sqlite3 \
    glfw
```
### NetBSD
Same with OpenBSD, X11 headers and lack of wayland needs to be addressed with cmake.
```shell
sudo pkgin install \
    cmake \
    git \
    gcc12 \
    mesa \
    sqlite3 \
    glfw
```