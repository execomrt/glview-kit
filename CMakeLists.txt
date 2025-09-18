cmake_minimum_required(VERSION 3.10)

# Project name and language
project(GLViewWin32Project LANGUAGES C CXX)


# Add source files
set(SOURCES src/Main.cpp src/RenderingTester.cpp)

# Add Vulkan include directory (assuming "vulkan/" is at the root)
include_directories(${CMAKE_SOURCE_DIR})

# Create the executable
add_executable(GLViewWin32Project WIN32 ${SOURCES})

