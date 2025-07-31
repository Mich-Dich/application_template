
include "dependencies.lua"

workspace "application"
	platforms "x64"
	startproject "application_template"

	configurations
	{
		"Debug",
		"RelWithDebInfo",
		"Release",
	}

	flags
	{
		"MultiProcessorCompile"
	}

	outputs  = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	local project_path = os.getcwd()
	defines
	{
		"ENGINE_INSTALL_DIR=\"" .. project_path .. "/bin/" .. outputs .. "\"",
		"OUTPUTS=\"" .. outputs .. "\"",
	}

	if os.target() == "linux" then
		print("---------- target platform is linux => manually compile GLFW ----------")
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
    project "application_template"

        location "%{wks.location}/gluttony"
        kind "WindowedApp"
        language "C++"
        cppdialect "C++20"
        staticruntime "on"

        targetdir ("%{wks.location}/bin/" .. outputs  .. "/%{prj.name}")
        objdir ("%{wks.location}/bin-int/" .. outputs  .. "/%{prj.name}")
        
        pchheader "src/util/pch.h"
        pchsource "src/util/pch.cpp"

        defines
        {
            "_CRT_SECURE_NO_WARNINGS",
            "GLFW_INCLUDE_NONE",
        }

        files
        {
            "src/**.h",
            "src/**.cpp",
            "src/**.embed",

            "%{vendor_path.ImGuizmo}/ImGuizmo.h",
            "%{vendor_path.ImGuizmo}/ImGuizmo.cpp",
        }

        includedirs
        {
            "src",
            "assets",
            "vendor",
            
            "%{IncludeDir.glew}",
            "%{IncludeDir.glm}",
            "%{IncludeDir.glfw}/include",
            "%{IncludeDir.ImGui}",
            "%{IncludeDir.ImGui}/backends/",
            "%{IncludeDir.stb_image}",
            "%{wks.location}/vendor/bvh/",
            "%{wks.location}/vendor/bvh/src",
            "%{wks.location}/vendor/bvh/src/bvh",
        }
        
        links
        {
            "ImGui",
            "assimp",
        }

        libdirs 
        {
            "vendor/imgui/bin/" .. outputs .. "/imgui",
        }

        filter "files:vendor/ImGuizmo/**.cpp"
            defines { "IMGUI_DEFINE_MATH_OPERATORS" }
            flags { "NoPCH" }
        
        filter "system:linux"
            systemversion "latest"
            defines "PLATFORM_LINUX"

            includedirs
            {
                "/usr/include",
                "%{IncludeDir.bvh}",
            }
            
            externalincludedirs											-- treat VMA as system headers (prevent warnings)
            {
                "/usr/include",
                "/usr/include/c++/13",
                "/usr/include/x86_64-linux-gnu/qt5", 				-- Base Qt include path
                "/usr/include/x86_64-linux-gnu/qt5/QtCore",
                "/usr/include/x86_64-linux-gnu/qt5/QtWidgets",
                "/usr/include/x86_64-linux-gnu/qt5/QtGui",
            }
            
            libdirs
            {
                "%{wks.location}/vendor/glfw/build/src",
                "/usr/lib/x86_64-linux-gnu",
                "/usr/lib/x86_64-linux-gnu/qt5",
            }
        
            links
            {
                "vulkan",
                "meshoptimizer",
                "GLEW",
                "GL",
                "glfw3",
                "Qt5Core",
                "Qt5Widgets",
                "Qt5Gui",
            }

            buildoptions
            {
                "-msse4.1",										  	-- include the SSE4.1 flag for Linux builds
                "-fPIC",
                "-Wall",											-- compiler options
                "-Wno-dangling-else",
            }

            postbuildcommands
            {
                -- '{COPYDIR} "%{wks.location}/gluttony/shaders" "%{wks.location}/bin/' .. outputs .. '"',
                -- '{COPYDIR} "%{wks.location}/gluttony/assets" "%{wks.location}/bin/' .. outputs .. '"',
            }
            
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
