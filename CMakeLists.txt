cmake_minimum_required(VERSION 3.29)
project(ScrapMechanicChellangeExtractor C CXX)

set(CMAKE_CXX_STANDARD 20)

option(BUILD_AS_WIN32 "Build as a Windows (Win32) GUI application instead of console" OFF)


# Setting parameters for raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games

add_subdirectory(libs/raylib)

set(SOURCES src/main.cpp
        src/json.hpp
        src/registry.cpp
        src/messageBox.cpp
        src/messageBox.h
        src/main.h)

if(BUILD_AS_WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES} src/winMain.cpp)
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") # Set the asset path macro to the absolute path on the dev machine
#target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="relative-path-to-assets-in-the-game-package") # Set the asset path macro in release mode
