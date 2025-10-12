
import os
import sys
import json
import glob
import subprocess
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))
import scripts.utils as utils


def detect_visual_studio_versions():
    vs_version_map = {
        "17": "2022",  # VS 2022 (major version 17)
        "16": "2019",  # VS 2019 (major version 16)
        "15": "2017"   # VS 2017 (major version 15)
    }
    try:
        vswhere_path = os.path.join(os.environ["ProgramFiles(x86)"], "Microsoft Visual Studio", "Installer", "vswhere.exe")
        result = subprocess.run([vswhere_path, "-format", "json", "-products", "*", "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64"], capture_output=True, text=True)
        if result.returncode == 0:
            import json
            installations = json.loads(result.stdout)
            versions = []
            for install in installations:
                major_version = install["installationVersion"].split('.')[0]
                premake_id = vs_version_map.get(major_version, None)
                if premake_id:
                    versions.append(premake_id)
            return list(set(versions))  # Remove duplicates
    except Exception as e:
        print(f"Error detecting Visual Studio versions: {e}")
    return []


def prompt_build_config():
    print("Select build configuration:")
    print("0. Debug")
    print("1. Release")
    print("2. RelWithDebInfo")
    
    choice = input("Enter the number (default 0): ").strip()
    if choice == "1":
        return "Release"
    elif choice == "2":
        return "RelWithDebInfo"
    else:
        return "Debug"
    

def detect_rider():
    # Check common Rider installation paths
    rider_paths = [
        os.path.join(os.environ["ProgramFiles"], "JetBrains", "JetBrains Rider *"),
        os.path.join(os.environ["LocalAppData"], "JetBrains", "Toolbox", "apps", "Rider", "*")
    ]
    for path in rider_paths:
        if os.path.exists(path):
            return True
    return False


def detect_vscode():
    try:
        subprocess.run(["where", "code"], check=True, capture_output=True)
        return True
    except subprocess.CalledProcessError:
        pass

    # Fallback: check common installation paths
    paths = [
        os.path.join(os.environ.get("ProgramFiles", ""), "Microsoft VS Code", "Code.exe"),
        os.path.join(os.environ.get("LocalAppData", ""), "Programs", "Microsoft VS Code", "Code.exe")
    ]
    return any(os.path.exists(p) for p in paths)


def detect_IDEs():
    IDEs = []
    vs_versions = detect_visual_studio_versions()
    if vs_versions:
        IDEs.extend([f"Visual Studio {version}" for version in vs_versions])

    if detect_vscode():
        IDEs.append("VSCode")

    if detect_rider():
        IDEs.append("JetBrains Rider")

    if not IDEs:
        print("No supported IDEs detected.")
        sys.exit(1)
    
    return IDEs


def prompt_ide_selection():
    
    IDEs = detect_IDEs()
    print("Detected IDEs:")
    for i, ide in enumerate(IDEs):
        print(f"{i}. {ide}")

    if len(IDEs) == 1:
        print("only one IDE detected")
        return IDEs[0]

    while (True):
        choice = input("Select an IDE to use (enter the number): ")
        try:
            choice_index = int(choice)
            if 0 <= choice_index < len(IDEs):
            
                selected_ide = IDEs[choice_index]
                if "VSCode" in selected_ide:
                    utils.print_c("\nVSCode usage", "blue")
                    print("  Build the project:                 Ctrl+Shift+B (runs 'Clean & Build')")
                    print("  Launch the application:            F5 (runs the debugger with selected config)")
                return selected_ide
            
            print("Invalid selection number.")
        except ValueError:
            print("Please enter a valid number.")


def setup_vscode_configs(project_root, build_config, application_name, clean_art_on_build, use_premake=False):
    vscode_dir = os.path.join(project_root, ".vscode")
    os.makedirs(vscode_dir, exist_ok=True)

    arch = "x86_64"
    system = "windows"
    output_dir = f"{build_config}-{system}"
    bin_dir = os.path.join(project_root, "bin", output_dir)
    exe_path = os.path.join(bin_dir, application_name, f"{application_name}.exe")

    # Conditionally include or skip cleaning artifacts
    clean_artifacts_block = f"""
echo ------ Clearing previous artifacts (trash at: %STAGE_DIR%) ------
mkdir "%STAGE_DIR%" 2>nul

move "{bin_dir}\\{application_name}\\{application_name}.exe" "%STAGE_DIR%\\" 2>nul
move "{bin_dir}\\{application_name}\\{application_name}.pdb" "%STAGE_DIR%\\" 2>nul

rd /s /q "{bin_dir}\\{application_name}" 2>nul
cd "{project_root}"
del /f /q Makefile 2>nul
del /f /q *.make 2>nul
""" if clean_art_on_build else """
REM To enable clearing of previous artifacts change [clean_build_artifacts_on_build] to true in file [config/app_settings.yml]
"""

    # Build system specific commands
    if use_premake:
        build_commands = """
echo ------ Regenerating Makefiles and rebuilding ------
vendor\\premake\\premake5.exe gmake2

echo ------ Building with MSBuild ------
msbuild {application_name}.sln /p:Configuration=%build_config% /p:Platform=x64 /t:Build /m
"""
    else:
        build_commands = """
echo ------ Generating build files ------
cmake -B build -DCMAKE_BUILD_TYPE=%build_config%

echo ------ Building main project ------
cmake --build build --config %build_config% --parallel
"""

    # Create build.bat script
    build_script_path = os.path.join(vscode_dir, "build.bat")
    build_script_content = f"""@echo off
setlocal enabledelayedexpansion

set build_config={build_config}
set timestamp=%date:~-4%-%date:~7,2%-%date:~4,2%-%time:~0,2%-%time:~3,2%-%time:~6,2%
set stage_name={application_name}_%build_config%_%timestamp%
set STAGE_DIR={bin_dir}\\%stage_name%

{clean_artifacts_block}

echo ------ Checking plugins ------

set plugins_rebuilt=0
set plugins_failed=0

if exist "plugins" (
    echo Scanning plugins directory...
    
    @REMfor /d %%i in (plugins\\*) do (
    @REM    if exist "%%i\\CMakeLists.txt" (
    @REM        set "plugin_dir=%%i"
    @REM        set "plugin_name=%%~nxi"
    @REM        
    @REM        echo   Checking plugin: !plugin_name!
    @REM        
    @REM        REM Check if plugin needs rebuilding
    @REM        set "needs_rebuild=0"
    @REM        set "build_dir=!plugin_dir!\\build"
    @REM        set "dll_file=!build_dir!\\!plugin_name!.dll"
    @REM        
    @REM        REM If no build directory exists
    @REM        if not exist "!build_dir!\" (
    @REM            echo     - No build directory found
    @REM            set needs_rebuild=1
    @REM        ) else (
    @REM            REM If no .dll file exists
    @REM            if not exist "!dll_file!" (
    @REM                echo     - No .dll file found
    @REM                set needs_rebuild=1
    @REM            ) else (
    @REM                REM Get timestamp of .dll file
    @REM                for %%f in ("!dll_file!") do set dll_time=%%~tf
    @REM                
    @REM                REM Find newest source file
    @REM                set newest_source_time=0
    @REM                set newest_source_file=
    @REM                
    @REM                for /r "!plugin_dir!" %%f in (*.cpp *.h *.hpp CMakeLists.txt) do (
    @REM                    for %%t in ("%%f") do (
    @REM                        REM Compare timestamps - this is a simplified check
    @REM                        if "%%~tf" gtr "!dll_time!" (
    @REM                            set needs_rebuild=1
    @REM                            echo     - Source files are newer than .dll file
    @REM                            goto :check_done
    @REM                        )
    @REM                    )
    @REM                )
    @REM            )
    @REM        )
    @REM        
    @REM        :check_done
    @REM        if !needs_rebuild! equ 1 (
    @REM            echo   Rebuilding required
    @REM            echo   Building plugin: !plugin_name!
    @REM            
    @REM            REM Create build directory
    @REM            if not exist "!build_dir!\" mkdir "!build_dir!"
    @REM            
    @REM            REM Build the plugin
    @REM            echo     Configuring CMake...
    @REM            cd "!build_dir!"
    @REM            cmake .. -DCMAKE_BUILD_TYPE=%build_config%
    @REM            if !errorlevel! neq 0 (
    @REM                echo     CMake configuration failed for !plugin_name!
    @REM                set /a plugins_failed+=1
    @REM                cd "!project_root!"
    @REM                goto :next_plugin
    @REM            )
    @REM            
    @REM            echo     Compiling...
    @REM            cmake --build . --config %build_config%
    @REM            if !errorlevel! neq 0 (
    @REM                echo     Compilation failed for !plugin_name!
    @REM                set /a plugins_failed+=1
    @REM                cd "!project_root!"
    @REM                goto :next_plugin
    @REM            )
    @REM            cd "!project_root!"
    @REM            
    @REM            echo     Successfully built: !plugin_name!
    @REM            set /a plugins_rebuilt+=1
    @REM        ) else (
    @REM            echo   Plugin is up to date
    @REM        )
    @REM        echo.
    @REM    )
    @REM    :next_plugin
    @REM)
) else (
    echo No plugins directory found
)

REM Build summary
echo ------ Build Summary ------
if !plugins_rebuilt! gtr 0 (
    echo Successfully rebuilt !plugins_rebuilt! plugin(s)
)
if !plugins_failed! gtr 0 (
    echo Failed to build !plugins_failed! plugin(s)
)
if !plugins_rebuilt! equ 0 if !plugins_failed! equ 0 (
    echo All plugins are up to date
)

echo.
{build_commands}

echo ------ Done ------
endlocal
"""
    with open(build_script_path, "w") as f:
        f.write(build_script_content)

    # Create tasks.json
    tasks_json_path = os.path.join(vscode_dir, "tasks.json")
    tasks_data = {
        "version": "2.0.0",
        "tasks": [
            {
                "label": "Clean & Build",
                "type": "shell",
                "command": "${workspaceFolder}\\.vscode\\build.bat",
                "problemMatcher": [],
                "group": { "kind": "build", "isDefault": True },
                "presentation": {
                    "reveal": "always",
                    "panel": "shared"
                }
            }
        ]
    }
    with open(tasks_json_path, "w") as f:
        json.dump(tasks_data, f, indent=4)

    # Create launch.json
    launch_json_path = os.path.join(vscode_dir, "launch.json")
    
    # Determine program path based on build system
    if use_premake:
        program_path = exe_path
        cwd_path = os.path.join("${workspaceFolder}", "bin", output_dir, application_name)
    else:
        program_path = os.path.join("${workspaceFolder}", "build", "bin", output_dir, application_name, f"{application_name}.exe")
        cwd_path = os.path.join("${workspaceFolder}", "build", "bin", output_dir, application_name)

    launch_data = {
        "version": "0.2.0",
        "configurations": [
            {
                "name": f"Launch {application_name} ({build_config})",
                "type": "cppvsdbg",
                "request": "launch",
                "program": program_path,
                "args": [],
                "cwd": cwd_path,
                "preLaunchTask": "Clean & Build",
                "stopAtEntry": False,
                "externalConsole": False,
                "environment": []
            }
        ]
    }
    with open(launch_json_path, "w") as f:
        json.dump(launch_data, f, indent=4)

    # Create c_cpp_properties.json
    config_define = {
        "debug": "DEBUG",
        "release_with_debug_info": "RELEASE_WITH_DEBUG_INFO",
        "release": "RELEASE"
    }.get(build_config.lower(), None)
    defines = ["PLATFORM_WINDOWS"]
    if config_define:
        defines.append(config_define)

    # Try to find MSVC compiler path
    compiler_path = "cl.exe"
    vs_versions = detect_visual_studio_versions()
    if vs_versions:
        vs_version = vs_versions[0]  # Use the first detected version
        compiler_path = f"C:/Program Files/Microsoft Visual Studio/{vs_version}/Community/VC/Tools/MSVC/*/bin/Hostx64/x64/cl.exe"
        compiler_path = glob.glob(compiler_path)[0] if glob.glob(compiler_path) else "cl.exe"

    cpp_props = {
        "version": 4,
        "configurations": [
            {
                "name": f"{build_config} ({system}-{arch})",
                "includePath": [
                    "${workspaceFolder}/**",
                    "${workspaceFolder}/vendor/**"
                ],
                "defines": defines,
                "compilerPath": compiler_path,
                "cStandard": "c11",
                "cppStandard": "c++20",
                "intelliSenseMode": "windows-msvc-x64"
            }
        ]
    }
    cpp_props_path = os.path.join(vscode_dir, "c_cpp_properties.json")
    with open(cpp_props_path, "w") as f:
        json.dump(cpp_props, f, indent=4)

    print("VSCode integration files generated in .vscode/")
