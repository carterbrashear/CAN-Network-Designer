cmake_minimum_required(VERSION 3.25.1)

project(
        CAN-Network-Designer
        VERSION 0.0.1
        DESCRIPTION "CAD software for designing CAN networks."
        LANGUAGES C CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Export compile commands for language servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Enable cmake's included fetch content for dependencies on GitHub
include(FetchContent)
# Enable CTest
enable_testing()

# ====================
# GLFW Dependency
# ====================
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.5.1
        SYSTEM
        FIND_PACKAGE_ARGS 3.4 CONFIG
)
FetchContent_MakeAvailable(glfw)
if (TARGET glfw3 AND NOT TARGET glfw)
    add_library(glfw ALIAS glfw3)
endif ()
# ====================
# GLAD Dependency for OpenGL 3.3 Core
# ====================
add_library(glad STATIC
        external/glad/src/glad.c
)
target_include_directories(glad PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include>
)
target_link_libraries(glad PRIVATE
        ${CMAKE_DL_LIBS}
)
# ====================
# Dear ImGui Docking Dependency
# ====================
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking # TODO: Replace with fixed commit to prevent breaking updates
        SYSTEM
)
FetchContent_MakeAvailable(imgui)
if (NOT TARGET imgui)
    add_library(imgui STATIC
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
            $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>
            $<BUILD_INTERFACE:${imgui_SOURCE_DIR}/backends>
    )
    target_link_libraries(imgui PUBLIC
            glfw
            glad
    )
endif ()
# ====================
# SQLite3 Dependency
# ====================
# Look for system package first.
find_package(SQLite3 QUIET)
if (SQLite3_FOUND)
    message(STATUS "Found system SQLite3 ${SQLite3_VERSION}")
    if (TARGET SQLite::SQLite3 AND NOT TARGET SQLite3::SQLite3)
        add_library(SQLite3::SQLite3 ALIAS SQLite::SQLite3)
    endif ()
else ()
    # If SQLite3 system package is not found then download the amalgamation from sqlite.org and extract it
    message(STATUS "System SQLite3 not found. Fetching amalgamation...")
    FetchContent_Declare(
            sqlite3_amalgamation
            URL https://sqlite.org/2026/sqlite-amalgamation-3540100.zip
    )
    FetchContent_MakeAvailable(sqlite3_amalgamation)
    if (NOT TARGET sqlite3)
        add_library(sqlite3 STATIC
                ${sqlite3_amalgamation_SOURCE_DIR}/sqlite3.c
        )
        target_include_directories(sqlite3 PUBLIC
                $<BUILD_INTERFACE:${sqlite3_amalgamation_SOURCE_DIR}>
        )
        find_package(Threads REQUIRED)
        target_link_libraries(sqlite3 PRIVATE
                Threads::Threads
                ${CMAKE_DL_LIBS}
                m
        )
        # Alias so it matches find_package's target name
        add_library(SQLite3::SQLite3 ALIAS sqlite3)
    endif ()
endif ()

# ====================
# Core App Static Library
# ====================
add_library(CAN-Network-Designer-Core STATIC
        src/Application.cpp src/Application.hpp
        src/UserInterface/Window.cpp src/UserInterface/Window.hpp
        src/UserInterface/UserInterface.cpp src/UserInterface/UserInterface.hpp
)
target_include_directories(CAN-Network-Designer-Core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/fonts>
)
target_link_libraries(CAN-Network-Designer-Core
        PUBLIC
        glfw
        glad
        imgui
        SQLite3::SQLite3
)

# ====================
# Main App Executable
# ====================
add_executable(CAN-Network-Designer
        src/main.cpp
)
target_link_libraries(CAN-Network-Designer PRIVATE
    CAN-Network-Designer-Core
)

# ====================
# Windowing Subsystem Test
# ====================
add_executable(test_window
        tests/test_window.cpp
)
target_link_libraries(test_window PRIVATE
        CAN-Network-Designer-Core
)
add_test(NAME WindowSubsystemTest COMMAND test_window)
set_tests_properties(WindowSubsystemTest PROPERTIES TIMEOUT 15)
