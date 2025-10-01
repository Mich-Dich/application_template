cmake_minimum_required(VERSION 3.16)
project(TestPlugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Get main project root
get_filename_component(MAIN_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

# Create shared library
add_library(test_plugin SHARED test_plugin.cpp)

# Set output properties
set_target_properties(test_plugin PROPERTIES
    OUTPUT_NAME "test_plugin"
    PREFIX "lib"
    SUFFIX ".so"
    POSITION_INDEPENDENT_CODE ON
)

# Include directories (without PCH)
target_include_directories(test_plugin PRIVATE
    "${MAIN_PROJECT_ROOT}/src"
    "${MAIN_PROJECT_ROOT}/vendor"
    "${MAIN_PROJECT_ROOT}/vendor/glm"
    # Add other specific includes as needed
)

# Compiler flags
target_compile_options(test_plugin PRIVATE
    -fPIC
    -shared
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O2>
    -Wall
    -Wextra
)

# Definitions
target_compile_definitions(test_plugin PRIVATE
    PLATFORM_LINUX
    "_CRT_SECURE_NO_WARNINGS"
    "GLFW_INCLUDE_NONE"
)
