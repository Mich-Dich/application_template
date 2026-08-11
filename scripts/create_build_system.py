import os
import sys
import scripts.utils as utils



def prompt_build_sys_selection():
    build_sys = ["CMake", "Premake5"]
    
    for i, ide in enumerate(build_sys):
        print(f"{i}. {ide}")

    while (True):
        choice = input("Select a Build System to use (enter the number): ")
        try:
            choice_index = int(choice)
            if 0 <= choice_index < len(build_sys):
                return build_sys[choice_index]
            
            print("Invalid selection number.")
        except ValueError:
            print("Please enter a valid number.")



def write_file(target_dir, content):
    with open(target_dir, "w") as f:
        f.write(content)
    utils.print_info(f"Created {target_dir}")


def apply_cmake_build_system(project_name):

    root_cmake = """
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

# add the tests
add_subdirectory(testing)

# Create your main application project
add_subdirectory(src)
"""

    src_cmake = f"""
# Application [src/CMakeLists.txt]

project({project_name} LANGUAGES CXX)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)  # For Qt5
# find_package(Qt6 COMPONENTS Core Widgets REQUIRED)  # For Qt6     # not needed yet

# List your source files
file(GLOB_RECURSE SOURCES_CONAN "*.cpp" "*.h")
file(GLOB IMPLOT_SOURCES "../vendor/implot/*.h" "../vendor/implot/*.cpp")

# Add ImGui OpenGL3 backend files - THESE WERE MISSING
file(GLOB IMGUI_OPENGL3_SOURCES
    "../vendor/imgui/backends/imgui_impl_opengl3.h"
    "../vendor/imgui/backends/imgui_impl_opengl3.cpp"
)

# Add executable with ALL required sources
add_executable({project_name} 
    ${{SOURCES_CONAN}} 
    ${{IMPLOT_SOURCES}} 
    ${{IMGUI_OPENGL3_SOURCES}}
)

# Precompiled headers (requires CMake 3.16+)
target_precompile_headers({project_name} PRIVATE util/pch.h)

# Include directories
target_include_directories({project_name} PRIVATE
    ${{CMAKE_CURRENT_SOURCE_DIR}}
    ${{CMAKE_CURRENT_SOURCE_DIR}}/assets
    ${{CMAKE_CURRENT_SOURCE_DIR}}/../vendor
    ${{CMAKE_CURRENT_SOURCE_DIR}}/../vendor/glew/include
    ${{CMAKE_CURRENT_SOURCE_DIR}}/../vendor/glm
    ${{CMAKE_CURRENT_SOURCE_DIR}}/../vendor/glfw/include
    ${{CMAKE_CURRENT_SOURCE_DIR}}/../vendor/imgui
    ${{CMAKE_CURRENT_SOURCE_DIR}}/../vendor/imgui/backends
    ${{CMAKE_CURRENT_SOURCE_DIR}}/../vendor/implot
    ${{CMAKE_CURRENT_SOURCE_DIR}}/../vendor/stb_image
    ${{Qt5Core_INCLUDE_DIRS}}
    ${{Qt5Widgets_INCLUDE_DIRS}}
)

# Add definitions for application target
target_compile_definitions({project_name} PRIVATE 
    GLFW_INCLUDE_NONE
    GLEW_STATIC
)

# Configuration-specific definitions
target_compile_definitions({project_name} PRIVATE 
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:RelWithDebInfo>:RELEASE_WITH_DEBUG_INFO>
    $<$<CONFIG:Release>:RELEASE>
)

# Link libraries
target_link_libraries({project_name} PRIVATE
    imgui
    glfw
    OpenGL::GL
    Qt5::Core
    Qt5::Widgets
    pulse
    pulse-simple
)

# Platform-specific linking
if(WIN32)
    # Try different GLEW library paths for MinGW
    find_library(GLEW_LIBRARY
        NAMES glew32s glew32
        PATHS 
            ${{CMAKE_SOURCE_DIR}}/vendor/glew/lib/Release/x64
            ${{CMAKE_SOURCE_DIR}}/vendor/glew/lib
            ${{CMAKE_SOURCE_DIR}}/vendor/glew/lib/Release/Win32
        NO_DEFAULT_PATH
    )
    
    if(GLEW_LIBRARY)
        target_link_libraries({project_name} PRIVATE ${{GLEW_LIBRARY}})
    else()
        # Fallback: try to find GLEW system-wide or use a different approach
        find_package(GLEW)
        if(GLEW_FOUND)
            target_link_libraries({project_name} PRIVATE GLEW::GLEW)
        else()
            message(WARNING "GLEW library not found. Please check the path.")
        endif()
    endif()
    
    target_link_libraries({project_name} PRIVATE
        gdi32
        user32
        comdlg32
        shell32
        opengl32
    )
    
    # Copy assets post-build
    add_custom_command(TARGET {project_name} POST_BUILD
        COMMAND ${{CMAKE_COMMAND}} -E copy_directory
        ${{CMAKE_SOURCE_DIR}}/assets
        $<TARGET_FILE_DIR:{project_name}>/assets
        COMMAND ${{CMAKE_COMMAND}} -E copy_directory
        ${{CMAKE_SOURCE_DIR}}/config
        $<TARGET_FILE_DIR:{project_name}>/config
    )
else()
    # Linux-specific GLEW handling
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLEW REQUIRED glew)
    
    # Add GLEW include directories and link libraries for Linux
    target_include_directories({project_name} PRIVATE
        ${{GLEW_INCLUDE_DIRS}}
    )
    
    target_link_libraries({project_name} PRIVATE
        ${{GLEW_LIBRARIES}}
        GL
        X11
        pthread
        dl
    )
    
    # Alternative method if pkg-config doesn't work:
    # find_library(GLEW_LIBRARY NAMES GLEW glew)
    # if(GLEW_LIBRARY)
    #     target_link_libraries({project_name} PRIVATE ${{GLEW_LIBRARY}})
    # endif()
    
    # Copy assets for Linux (create the directories first)
    add_custom_command(TARGET {project_name} POST_BUILD
        COMMAND ${{CMAKE_COMMAND}} -E make_directory
        $<TARGET_FILE_DIR:{project_name}>/assets
        COMMAND ${{CMAKE_COMMAND}} -E make_directory
        $<TARGET_FILE_DIR:{project_name}>/config
        COMMAND ${{CMAKE_COMMAND}} -E copy_directory
        ${{CMAKE_SOURCE_DIR}}/assets
        $<TARGET_FILE_DIR:{project_name}>/assets
        COMMAND ${{CMAKE_COMMAND}} -E copy_directory
        ${{CMAKE_SOURCE_DIR}}/config
        $<TARGET_FILE_DIR:{project_name}>/config
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
"""
    
    tests_cmake = """

# Tests [testing/CMakeLists.txt]

project(tests LANGUAGES CXX)

# Use FetchContent for Catch2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# List test source files
file(GLOB_RECURSE TEST_SOURCES "*.cpp" "*.h")

# List utility source files from main application using file(GLOB)
file(GLOB UTIL_TIMING_SOURCES "../src/util/timing/*.cpp" "../src/util/timing/*.h")
file(GLOB UTIL_DATA_STRUCTURES_SOURCES 
    "../src/util/data_structures/data_types.h"
    "../src/util/data_structures/deletion_queue.h" 
    "../src/util/data_structures/deletion_queue.cpp"
    "../src/util/data_structures/type_deletion_queue.h"
    "../src/util/data_structures/type_deletion_queue.cpp"
    "../src/util/data_structures/string_manipulation.cpp"
)
file(GLOB UTIL_MATH_SOURCES
    "../src/util/math/random.cpp"
    "../src/util/math/math.cpp"
)
file(GLOB UTIL_IO_SOURCES
    "../src/util/io/io.cpp"
    "../src/util/io/config.cpp"
    "../src/util/io/logger.cpp"
    "../src/util/io/serializer_data.h"
    "../src/util/io/serializer_yaml.h"
    "../src/util/io/serializer_yaml.cpp"
    "../src/util/io/serializer_binary.h"
    "../src/util/io/serializer_binary.cpp"
)
file(GLOB UTIL_SYSTEM_SOURCES
    "../src/util/system.cpp"
)

# Combine all utility sources
set(UTIL_SOURCES
    ${UTIL_TIMING_SOURCES}
    ${UTIL_DATA_STRUCTURES_SOURCES}
    ${UTIL_MATH_SOURCES}
    ${UTIL_IO_SOURCES}
    ${UTIL_SYSTEM_SOURCES}
)

# Add executable
add_executable(tests ${TEST_SOURCES} ${UTIL_SOURCES})

# Set C++ standard
set_target_properties(tests PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

# Include directories (remove the local Catch2 src path)
target_include_directories(tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/glew/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/implot
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/stb_image
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
)

# Add definitions
target_compile_definitions(tests PRIVATE 
    _CRT_SECURE_NO_WARNINGS
    GLFW_INCLUDE_NONE
    NOMINMAX
)

# Configuration-specific definitions
target_compile_definitions(tests PRIVATE 
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:RelWithDebInfo>:RELEASE_WITH_DEBUG_INFO>
    $<$<CONFIG:Release>:RELEASE>
)

# Platform-specific definitions
if(WIN32)
    target_compile_definitions(tests PRIVATE
        PLATFORM_WINDOWS
        WIN32_LEAN_AND_MEAN
        UNICODE
        _UNICODE
        GLEW_STATIC
    )
else()
    target_compile_definitions(tests PRIVATE
        PLATFORM_LINUX
    )
endif()

# Link libraries
target_link_libraries(tests PRIVATE
    imgui
    glfw
    OpenGL::GL
    Qt5::Core
    Qt5::Widgets
    Catch2::Catch2WithMain
)

# Platform-specific linking
if(WIN32)
    # GLEW library for Windows
    find_library(GLEW_LIBRARY
        NAMES glew32s glew32
        PATHS 
            ${CMAKE_SOURCE_DIR}/vendor/glew/lib/Release/x64
            ${CMAKE_SOURCE_DIR}/vendor/glew/lib
            ${CMAKE_SOURCE_DIR}/vendor/glew/lib/Release/Win32
        NO_DEFAULT_PATH
    )
    
    if(GLEW_LIBRARY)
        target_link_libraries(tests PRIVATE ${GLEW_LIBRARY})
    else()
        find_package(GLEW)
        if(GLEW_FOUND)
            target_link_libraries(tests PRIVATE GLEW::GLEW)
        else()
            message(WARNING "GLEW library not found for tests.")
        endif()
    endif()
    
    target_link_libraries(tests PRIVATE
        gdi32
        user32
        comdlg32
        shell32
        opengl32
    )
    
else()
    # Linux-specific linking
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLEW REQUIRED glew)
    
    target_include_directories(tests PRIVATE
        ${GLEW_INCLUDE_DIRS}
    )
    
    target_link_libraries(tests PRIVATE
        ${GLEW_LIBRARIES}
        GL
        X11
        pthread
        dl
    )
endif()

# Use FetchContent for Catch2 instead of manual build
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4  # Use a specific stable version
)
FetchContent_MakeAvailable(Catch2)

# Link against Catch2 targets
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(tests PRIVATE
        -msse4.1
        -fPIC
        -Wall
        -Wno-dangling-else
    )
endif()

# Disable PCH for specific files if needed
set_source_files_properties(
    ../vendor/implot/implot.cpp
    ../vendor/implot/implot_items.cpp
    ../vendor/implot/implot_demo.cpp
    ../vendor/imgui/backends/imgui_impl_glfw.cpp
    ../vendor/imgui/backends/imgui_impl_opengl3.cpp
    PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON
)
"""

    imgui_cmake = """
# ImGui [vendor/imgui/CMakeLists.txt]

project(imgui LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# List source files
set(IMGUI_SOURCES
    imconfig.h
    imgui.h
    imgui.cpp
    imgui_draw.cpp
    imgui_internal.h
    imgui_tables.cpp
    imgui_widgets.cpp
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    imgui_demo.cpp
    backends/imgui_impl_glfw.h
    backends/imgui_impl_glfw.cpp
)

# Create static library
add_library(imgui STATIC ${IMGUI_SOURCES})

# Include directories
target_include_directories(imgui PUBLIC 
    . 
    backends
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
)

# Add GLFW_INCLUDE_NONE for ImGui if it uses GLFW
target_compile_definitions(imgui PRIVATE GLFW_INCLUDE_NONE)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
elseif(UNIX)
    # Linux-specific settings
    set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
"""


    # Write files to their respective locations
    try:

        write_file("./CMakeLists.txt", root_cmake)
        write_file("./src/CMakeLists.txt", src_cmake)
        write_file("./testing/CMakeLists.txt", tests_cmake)
        write_file("./vendor/imgui/CMakeLists.txt", imgui_cmake)
        utils.print_c(f"CMake build system successfully applied for project: {project_name}", "green")
        
    except Exception as e:
        print(f"Error creating CMake files: {e}")



def apply_premake_build_system(project_name):

    root_premake = f"""

------------------ dependencies ------------------

------------ path ------------
vendor_path = {{}}
vendor_path["glew"]				= "%{{wks.location}}/vendor/glew"
vendor_path["glfw"]          	= "%{{wks.location}}/vendor/glfw"
vendor_path["glm"]           	= "%{{wks.location}}/vendor/glm"
vendor_path["ImGui"]         	= "%{{wks.location}}/vendor/imgui"
vendor_path["implot"]         	= "%{{wks.location}}/vendor/implot"
vendor_path["stb_image"]     	= "%{{wks.location}}/vendor/stb_image"
vendor_path["catch2"]           = "%{{wks.location}}/vendor/Catch2"

------------ include ------------ 
IncludeDir = {{}}
IncludeDir["glew"]              = "%{{vendor_path.glew}}/include"
IncludeDir["glfw"]              = "%{{vendor_path.glfw}}"
IncludeDir["glm"]               = "%{{vendor_path.glm}}"
IncludeDir["ImGui"]             = "%{{vendor_path.ImGui}}"
IncludeDir["implot"]            = "%{{vendor_path.implot}}"
IncludeDir["stb_image"]         = "%{{vendor_path.stb_image}}"
IncludeDir["catch2"]            = "%{{vendor_path.catch2}}/src"

workspace "application"
	platforms "x64"
	startproject "{project_name}"

	configurations
	{{
		"Debug",
		"RelWithDebInfo",
		"Release",
	}}

	flags
	{{
		"MultiProcessorCompile"
	}}

	outputs  = "%{{cfg.buildcfg}}-%{{cfg.system}}-%{{cfg.architecture}}"
	local project_path = os.getcwd()
	defines
	{{
		"ENGINE_INSTALL_DIR=\\"" .. project_path .. "/bin/" .. outputs .. "\\"",
		"OUTPUTS=\\"" .. outputs .. "\\"",
	}}

    ---------- DISABLED FOR DEV ----------
	if os.target() == "linux" then
		print("---------- pre-compile GLFW ----------")
		os.execute("cmake -S ./vendor/glfw -B ./vendor/glfw/build")								-- manuel compilation
		os.execute("cmake --build ./vendor/glfw/build")											-- manuel compilation
		print("---------- Done compiling GLFW ----------")
	end

group "dependencies"
	include "vendor/imgui"
	if os.target() == "windows" then
		include "vendor/glfw"
	end
group ""


group "core"
    project "{project_name}"      -- generated by the setup.py script, modify "app_settings.yml" to change name

        location "%{{wks.location}}"
        kind "WindowedApp"
        language "C++"
        cppdialect "C++20"
        staticruntime "on"

        targetdir ("%{{wks.location}}/bin/" .. outputs  .. "/%{{prj.name}}")
        objdir ("%{{wks.location}}/bin-int/" .. outputs  .. "/%{{prj.name}}")
        
        pchheader "util/pch.h"
        pchsource "src/util/pch.cpp"

        defines
        {{
            "_CRT_SECURE_NO_WARNINGS",
            "GLFW_INCLUDE_NONE",
            "NOMINMAX",
        }}

        files
        {{
            "src/**.h",
            "src/**.cpp",
            "src/**.embed",

            "vendor/implot/*.h",              -- directly bake implot into application
            "vendor/implot/*.cpp",
		    "vendor/imgui/backends/imgui_impl_opengl3.h",
		    "vendor/imgui/backends/imgui_impl_opengl3.cpp",
        }}

        includedirs
        {{
            "src",
            "assets",
            "vendor",
            
            "%{{IncludeDir.glew}}",
            "%{{IncludeDir.glm}}",
            "%{{IncludeDir.glfw}}/include",
            "%{{IncludeDir.ImGui}}",
            "%{{IncludeDir.ImGui}}/backends/",
            "%{{IncludeDir.implot}}",
            "%{{IncludeDir.stb_image}}",
        }}
        
        links
        {{
            "ImGui",
        }}

        libdirs 
        {{
            "vendor/imgui/bin/" .. outputs .. "/imgui",
        }}

        filter "files:vendor/implot/**.cpp"
            flags {{ "NoPCH" }}
        
        filter "files:vendor/imgui/**.cpp"
            flags {{ "NoPCH" }}
        
        filter "system:linux"
            systemversion "latest"
            defines "PLATFORM_LINUX"

            includedirs
            {{
                "/usr/include",
            }}
            
            externalincludedirs											-- treat VMA as system headers (prevent warnings)
            {{
                "/usr/include/x86_64-linux-gnu/qt5", 				-- Base Qt include path
                "/usr/include/x86_64-linux-gnu/qt5/QtCore",
                "/usr/include/x86_64-linux-gnu/qt5/QtWidgets",
                "/usr/include/x86_64-linux-gnu/qt5/QtGui",
            }}
            
            libdirs
            {{
                "%{{wks.location}}/vendor/glfw/build/src",
                "/usr/lib/x86_64-linux-gnu",
                "/usr/lib/x86_64-linux-gnu/qt5",
            }}
        
            links
            {{
                "GLEW",
                "GL",
                "glfw3",
                "Qt5Core",
                "Qt5Widgets",
                "Qt5Gui",
            }}

            buildoptions
            {{
                "-msse4.1",										  	-- include the SSE4.1 flag for Linux builds
                "-fPIC",
                "-Wall",											-- compiler options
                "-Wno-dangling-else",
            }}

            postbuildcommands
            {{
                '{{COPYDIR}} -n "%{{wks.location}}/assets" "%{{wks.location}}/bin/' .. outputs .. '/%{{prj.name}}"',
                '{{COPYDIR}} -n "%{{wks.location}}/config" "%{{wks.location}}/bin/' .. outputs .. '/%{{prj.name}}"',
            }}

        filter "system:windows"
            systemversion "latest"
            defines
            {{
                "PLATFORM_WINDOWS",
                "WIN32_LEAN_AND_MEAN",
                "GLEW_STATIC",
                "UNICODE",
                "_UNICODE",
            }}

            links
            {{
                "ImGui",
                "glfw",
                "glew32s",
                "opengl32",
                "gdi32",
                "user32",
                "comdlg32",
                "shell32",
            }}

            libdirs
            {{
                "%{{wks.location}}\\\\vendor\\\\glfw\\\\bin\\\\" .. outputs  .. "\\\\glfw",
                "%{{wks.location}}\\\\vendor\\\\imgui\\\\bin\\\\" .. outputs  .. "\\\\imgui",
                "%{{vendor_path.glew}}\\\\lib\\\\Release\\\\x64",
            }}
            
            postbuildcommands
            {{
                '{{COPYDIR}} "%{{wks.location}}\\\\assets" "%{{wks.location}}\\\\bin\\\\' .. outputs .. '\\\\%{{prj.name}}\\\\assets"',
                '{{COPYDIR}} "%{{wks.location}}\\\\config" "%{{wks.location}}\\\\bin\\\\' .. outputs .. '\\\\%{{prj.name}}\\\\config"',
            }}

        
        filter {{ "system:windows", "action:gmake2" }}  -- MinGW specific settings
            links
            {{
                "ImGui",
                "glfw3",
                "glew32",
                "opengl32",
                "gdi32",
                "user32",
                "comdlg32",
                "shell32",
            }}

            libdirs
            {{
                "%{{wks.location}}\\\\vendor\\\\glfw\\\\build-mingw\\\\src",
                "%{{wks.location}}\\\\vendor\\\\glfw\\\\bin\\\\' .. outputs .. '\\\\glfw",
                "%{{vendor_path.glew}}\\\\lib\\\\Release\\\\Win32",
            }}


        filter "configurations:Debug"
            defines "DEBUG"
            runtime "Debug"
            symbols "on"

        filter "configurations:RelWithDebInfo"
            defines "RELEASE_WITH_DEBUG_INFO"
            runtime "Release"
            symbols "on"
            optimize "on"

        filter "configurations:Release"
            defines "RELEASE"
            runtime "Release"
            symbols "off"
            optimize "on"

group ""


group "tests"
    project "tests"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"
        staticruntime "on"

        targetdir ("%{{wks.location}}/bin/" .. outputs .. "/%{{prj.name}}")
        objdir ("%{{wks.location}}/bin-int/" .. outputs .. "/%{{prj.name}}")

        files
        {{
            "testing/**.h",
            "testing/**.cpp",

            "src/util/timing/**.h",
            "src/util/timing/**.cpp",
            
            "src/util/data_structures/data_types.h",
            "src/util/data_structures/deletion_queue.h",
            "src/util/data_structures/deletion_queue.cpp",
            "src/util/data_structures/type_deletion_queue.h",
            "src/util/data_structures/type_deletion_queue.cpp",

            "src/util/math/random.cpp",
            "src/util/math/math.cpp",
            "src/util/io/io.cpp",
            "src/util/io/config.cpp",
            "src/util/io/logger.cpp",
            "src/util/io/serializer_data.h",
            "src/util/io/serializer_yaml.h",
            "src/util/io/serializer_yaml.cpp",
            "src/util/io/serializer_binary.h",
            "src/util/io/serializer_binary.cpp",


            "src/util/data_structures/string_manipulation.cpp",
            "src/util/system.cpp",
        }}

        includedirs
        {{
            "src",
            "testing",
            "%{{IncludeDir.catch2}}",
            "%{{IncludeDir.glm}}",
            "%{{vendor_path.catch2}}/Build/generated-includes",

            "%{{IncludeDir.glew}}",
            "%{{IncludeDir.glm}}",
            "%{{IncludeDir.glfw}}/include",
            "%{{IncludeDir.ImGui}}",
            "%{{IncludeDir.ImGui}}/backends/",
            "%{{IncludeDir.implot}}",
            "%{{IncludeDir.stb_image}}",
        }}

        links
        {{
            "ImGui",
        }}

        libdirs 
        {{
            "%{{vendor_path.catch2}}/Build/src",
            "vendor/imgui/bin/" .. outputs .. "/imgui",
        }}

        prebuildcommands
        {{
            "cmake -S ./vendor/Catch2 -B ./vendor/Catch2/Build -DCMAKE_BUILD_TYPE=%{{cfg.buildcfg}} -DCATCH_INSTALL_DOCS=OFF -DCATCH_INSTALL_EXTRAS=OFF",
            "cmake --build ./vendor/Catch2/Build --config %{{cfg.buildcfg}}"
        }}

        filter "configurations:Debug"
            links {{ "Catch2Maind", "Catch2d" }}

        filter "configurations:RelWithDebInfo"
            links {{ "Catch2Main", "Catch2" }}

        filter "configurations:Release"
            links {{ "Catch2Main", "Catch2" }}

        filter "files:vendor/implot/**.cpp"
            flags {{ "NoPCH" }}
        
        filter "files:vendor/imgui/**.cpp"
            flags {{ "NoPCH" }}
        
        filter "system:linux"
            systemversion "latest"
            defines "PLATFORM_LINUX"
            links {{ 
                "pthread",      -- Catch2 requires pthread on Linux
                "Qt5Core",      -- Add Qt libraries if needed
                "Qt5Widgets",
                "Qt5Gui",
            }}

            buildoptions
            {{
                "-msse4.1",
                "-fPIC",
                "-Wall",
                "-Wno-dangling-else"
            }}
            
            externalincludedirs											-- treat VMA as system headers (prevent warnings)
            {{
                "/usr/include/x86_64-linux-gnu/qt5", 				-- Base Qt include path
                "/usr/include/x86_64-linux-gnu/qt5/QtCore",
                "/usr/include/x86_64-linux-gnu/qt5/QtWidgets",
                "/usr/include/x86_64-linux-gnu/qt5/QtGui",
            }}

        filter "system:windows"
            systemversion "latest"
            defines
            {{
                "PLATFORM_WINDOWS",
                "UNICODE",
                "_UNICODE",
            }}

            links
            {{
                "ImGui",
                "glfw",
                "glew32s",
                "opengl32",
                "gdi32",
                "user32",
                "comdlg32",   -- For GetOpenFileNameW
                "shell32",    -- For other Windows APIs
            }}

            libdirs
            {{
                "%{{wks.location}}/vendor/glfw/lib-vc2022",
                "%{{vendor_path.glew}}/lib/Release/x64",
            }}

        filter "configurations:Debug"
            defines "DEBUG"
            runtime "Debug"
            symbols "on"

        filter "configurations:RelWithDebInfo"
            defines "RELEASE_WITH_DEBUG_INFO"
            runtime "Release"
            symbols "on"
            optimize "on"

        filter "configurations:Release"
            defines "RELEASE"
            runtime "Release"
            symbols "off"
            optimize "on"
group ""
"""

    imgui_premake = """
project "imgui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
    staticruntime "on"

	targetdir ("bin/" .. outputs .. "/%{prj.name}")
	objdir ("bin-int/" .. outputs .. "/%{prj.name}")

	files
	{
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_tables.cpp",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp",
		"backends/imgui_impl_glfw.h",
		"backends/imgui_impl_glfw.cpp",
	}

	includedirs
	{
		"%{prj.name}",
		".",
		"backends",
		"%{IncludeDir.glfw}/include",
		-- "%{IncludeDir.Vulkan}",
	}

	filter "system:windows"
		systemversion "latest"

	filter "system:linux"
		pic "On"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		
	filter "configurations:RelWithDebInfo"
		runtime "Release"
		symbols "on"
		optimize "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
"""

    # Write files to their respective locations
    try:

        write_file("./premake5.lua", root_premake)
        write_file("./vendor/imgui/premake5.lua", imgui_premake)
        utils.print_c(f"Premake5 build system successfully applied for project: {project_name}", "green")
        
    except Exception as e:
        print(f"Error creating CMake files: {e}")
