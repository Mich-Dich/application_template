# Application [src/CMakeLists.txt]

project(application_template LANGUAGES CXX)

# Find required packages
find_package(OpenGL REQUIRED)

# List your source files
file(GLOB_RECURSE SOURCES_CONAN "*.cpp" "*.h")
file(GLOB IMPLOT_SOURCES "../vendor/implot/*.h" "../vendor/implot/*.cpp")

# Add ImGui OpenGL3 backend files - THESE WERE MISSING
file(GLOB IMGUI_OPENGL3_SOURCES
    "../vendor/imgui/backends/imgui_impl_opengl3.h"
    "../vendor/imgui/backends/imgui_impl_opengl3.cpp"
)

# Add executable with ALL required sources
add_executable(application_template 
    ${SOURCES_CONAN} 
    ${IMPLOT_SOURCES} 
    ${IMGUI_OPENGL3_SOURCES}
)

# Precompiled headers (requires CMake 3.16+)
target_precompile_headers(application_template PRIVATE util/pch.h)

# Include directories
target_include_directories(application_template PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/glew/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/implot
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/stb_image
)

# Add definitions for application target
target_compile_definitions(application_template PRIVATE 
    GLFW_INCLUDE_NONE
    GLEW_STATIC
)

# Configuration-specific definitions
target_compile_definitions(application_template PRIVATE 
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:RelWithDebInfo>:RELEASE_WITH_DEBUG_INFO>
    $<$<CONFIG:Release>:RELEASE>
)

# Link libraries
target_link_libraries(application_template PRIVATE
    imgui
    glfw
    OpenGL::GL
)

# Platform-specific linking
if(WIN32)
    # Try different GLEW library paths for MinGW
    find_library(GLEW_LIBRARY
        NAMES glew32s glew32
        PATHS 
            ${CMAKE_SOURCE_DIR}/vendor/glew/lib/Release/x64
            ${CMAKE_SOURCE_DIR}/vendor/glew/lib
            ${CMAKE_SOURCE_DIR}/vendor/glew/lib/Release/Win32
        NO_DEFAULT_PATH
    )
    
    if(GLEW_LIBRARY)
        target_link_libraries(application_template PRIVATE ${GLEW_LIBRARY})
    else()
        # Fallback: try to find GLEW system-wide or use a different approach
        find_package(GLEW)
        if(GLEW_FOUND)
            target_link_libraries(application_template PRIVATE GLEW::GLEW)
        else()
            message(WARNING "GLEW library not found. Please check the path.")
        endif()
    endif()
    
    target_link_libraries(application_template PRIVATE
        gdi32
        user32
        comdlg32
        shell32
        opengl32
    )
    
    # Copy assets post-build
    add_custom_command(TARGET application_template POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:application_template>/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/config
        $<TARGET_FILE_DIR:application_template>/config
    )
endif()

# Disable PCH for specific files (similar to your Premake flags)
set_source_files_properties(
    ../vendor/implot/implot.cpp
    ../vendor/implot/implot_items.cpp
    ../vendor/implot/implot_demo.cpp
    ../vendor/imgui/backends/imgui_impl_glfw.cpp
    ../vendor/imgui/backends/imgui_impl_opengl3.cpp  # ADD THIS
    PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON
)