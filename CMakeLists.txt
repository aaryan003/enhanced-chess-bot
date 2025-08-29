cmake_minimum_required(VERSION 3.16)
project(EnhancedChessBot)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Force CMake to use your intended vcpkg toolchain ---
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/devtools/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# --- SAFETY CHECK: Abort if CLion's shadow vcpkg is being used ---
if(CMAKE_TOOLCHAIN_FILE MATCHES "\\.vcpkg-clion")
    message(FATAL_ERROR "?? Wrong vcpkg path detected: ${CMAKE_TOOLCHAIN_FILE}. 
    Please set it to C:/devtools/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# Set default build type to Debug
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# Find required packages using vcpkg
find_package(SFML COMPONENTS Graphics Window System CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Add your project's include directory
include_directories(include)

# Define all your source files
set(SOURCES
    src/core/Board.cpp
    src/core/Types.cpp
    src/game/GameManager.cpp
    src/game/Player.cpp
    src/engine/Engine.cpp
    src/ui/Gui.cpp
    src/main.cpp
)

# --- FIX: Create the executable target ---
add_executable(EnhancedChessBot ${SOURCES})

# --- FIX: Moved these commands AFTER add_executable ---
# Link libraries to the executable
target_link_libraries(EnhancedChessBot PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    imgui::imgui
    ImGui-SFML::ImGui-SFML
    fmt::fmt
)

# Set compiler warnings
if(MSVC)
    target_compile_options(EnhancedChessBot PRIVATE /W4 /permissive-)
else()
    target_compile_options(EnhancedChessBot PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set working directory for debugging in Visual Studio
set_target_properties(EnhancedChessBot PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Ensure console window appears on Windows
if(WIN32)
    set_target_properties(EnhancedChessBot PROPERTIES
        WIN32_EXECUTABLE OFF
    )
endif()
