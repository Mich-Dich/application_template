# Root [CMakeLists.txt]

cmake_minimum_required(VERSION 3.15)
project(application LANGUAGES CXX)

# Set C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories for binaries (similar to your targetdir and objdir)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin-int)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin-int)

# Define configurations (Debug, RelWithDebInfo, Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif()
set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release")

# Platform-specific definitions
if(WIN32)
    add_compile_definitions(
        PLATFORM_WINDOWS
        WIN32_LEAN_AND_MEAN
        UNICODE
        _UNICODE
    )
elseif(UNIX)
    add_compile_definitions(
        PLATFORM_LINUX
    )
endif()

# Define common definitions and flags
add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
)

# Multi-processor compilation
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

# For MinGW, add specific compiler options - FIXED: Only apply C flags to C files
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    add_compile_options(
        -Wall 
        $<$<COMPILE_LANGUAGE:C>:-Wno-declaration-after-statement>
    )
endif()

# Build GLFW first so it's available for ImGui
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Don't build the GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Don't build the GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Don't build the GLFW docs")
set(GLFW_INSTALL OFF CACHE BOOL "Don't create an GLFW install target")
add_subdirectory(vendor/glfw)

# Then build ImGui (which depends on GLFW)
add_subdirectory(vendor/imgui)

# Create your main application project
add_subdirectory(src)