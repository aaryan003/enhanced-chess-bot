cmake_minimum_required(VERSION 3.16)
project(EnhancedChessBot)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg integration (CLion passes toolchain automatically if set)
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Find packages from vcpkg
find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Include directories
include_directories(include)

# Collect source files explicitly
set(CORE_SOURCES
        src/core/Board.cpp
        src/core/Types.cpp
)

set(GAME_SOURCES
        src/game/GameManager.cpp
        src/game/Player.cpp
)

set(ENGINE_SOURCES
        src/engine/Engine.cpp
)

set(MAIN_SOURCE
        src/main.cpp
)

set(SOURCES
        ${CORE_SOURCES}
        ${GAME_SOURCES}
        ${ENGINE_SOURCES}
        ${MAIN_SOURCE}
)

# Create executable
add_executable(chess-bot ${SOURCES})

# Force console subsystem for MinGW
if (WIN32 AND MINGW)
    set_target_properties(chess-bot PROPERTIES
            WIN32_EXECUTABLE FALSE
            LINK_FLAGS "-mconsole"
    )
elseif(WIN32)
    set_target_properties(chess-bot PROPERTIES
            WIN32_EXECUTABLE OFF
    )
endif()

# Link libraries
target_link_libraries(chess-bot PRIVATE
        SFML::Network
        SFML::Graphics
        SFML::Window
        SFML::Audio
        SFML::System
        imgui::imgui
        ImGui-SFML::ImGui-SFML
        nlohmann_json::nlohmann_json
        fmt::fmt
)

# Compiler warnings
if(MSVC)
    target_compile_options(chess-bot PRIVATE /W4 /permissive-)
else()
    target_compile_options(chess-bot PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy DLLs to output directory (for running inside CLion/MSVC)
if(WIN32)
    add_custom_command(TARGET chess-bot POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SFML::Graphics>
            $<TARGET_FILE:SFML::Window>
            $<TARGET_FILE:SFML::System>
            $<TARGET_FILE:SFML::Audio>
            $<TARGET_FILE:ImGui-SFML::ImGui-SFML>
            $<TARGET_FILE:fmt::fmt>
            $<TARGET_FILE_DIR:chess-bot>
            COMMENT "Copying required DLLs"
    )
endif()

# Set working directory for debugging
set_target_properties(chess-bot PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
