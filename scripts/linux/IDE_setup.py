
import os
import sys
import glob
import platform
import subprocess
import stat
import json
import datetime

import scripts.utils as utils

def detect_rider():
    # Check common Rider installation paths on Linux
    rider_paths = [
        os.path.expanduser("~/.local/share/JetBrains/Toolbox/apps/Rider"),
        "/opt/JetBrains Rider",
        os.path.expanduser("~/.local/share/applications/jetbrains-rider.desktop")
    ]
    
    # Check if rider command exists in PATH
    try:
        subprocess.run(["which", "rider"], check=True, capture_output=True)
        return True
    except subprocess.CalledProcessError:
        pass

    # Check physical paths
    for path in rider_paths:
        if os.path.exists(path) or len(glob.glob(path)) > 0:
            return True
    return False


def detect_vscode():
    # Check if the `code` CLI is available
    try:
        subprocess.run(["which", "code"], check=True, capture_output=True)
        return True
    except subprocess.CalledProcessError:
        pass

    # Check common VSCode install paths (Linux)
    vscode_paths = [
        "/usr/share/code",
        "/opt/visual-studio-code",
        os.path.expanduser("~/.vscode"),
        os.path.expanduser("~/.var/app/com.visualstudio.code"),  # Flatpak
    ]
    for path in vscode_paths:
        if os.path.exists(path) or glob.glob(path):
            return True

    return False


def detect_IDEs():
    IDEs = []
    
    if detect_vscode():
        IDEs.append("VSCode")
    
    if detect_rider():                      # Detect available IDEs
        IDEs.append("JetBrains Rider")

    if not IDEs:
        print("No supported IDEs detected.")
        sys.exit(1)
    
    return IDEs

        
def prompt_ide_selection():
    
    IDEs = detect_IDEs()
    if (len(IDEs) == 1) and utils.get_silent():
        return IDEs[0]

    print("\nDetected IDEs:")
    for i, ide in enumerate(IDEs):
        print(f"{i}. {ide}")

    if len(IDEs) == 1:
        print("Only one IDE detected")
        return IDEs[0]

    while True:
        choice = input("Select an IDE to use (enter the number): ")
        try:
            choice_index = int(choice)
            if 0 <= choice_index < len(IDEs):
                return IDEs[choice_index]
            print("Invalid selection number.")
        except ValueError:
            print("Please enter a valid number.")


def prompt_build_config():
    configs = ["Debug", "RelWithDebInfo", "Release"]
    print("\nAvailable build configurations:")
    for i, cfg in enumerate(configs):
        print(f"{i}. {cfg}")
    
    while True:
        choice = input("Select a configuration (enter the number): ")
        try:
            index = int(choice)
            if 0 <= index < len(configs):
                if utils.SILENT:
                    print("")
                return configs[index]
            
            print("Invalid selection number.")
        except ValueError:
            print("Please enter a valid number.")


def setup_vscode_configs(project_root, build_config, application_name, clean_art_on_build, use_premake=False):
    vscode_dir = os.path.join(project_root, ".vscode")
    os.makedirs(vscode_dir, exist_ok=True)

    arch = "x86_64"
    system = platform.system().lower()  # "linux" or "windows"
    output_dir = f"{build_config}-{system}-{arch}"
    bin_dir = os.path.join(project_root, "bin", output_dir)

    # Optional block for cleaning artifacts
    clean_artifacts_block = f"""
echo "------ Clearing previous artifacts (trash at: $STAGE_DIR) ------"
mkdir -p "$STAGE_DIR"

# move all previous artifacts into staging (ignore missing)
mv "{bin_dir}/{application_name}" "$STAGE_DIR/" 2>/dev/null || true

# trash the staging directory and Makefiles
gio trash "$STAGE_DIR" --force || true
cd "{project_root}"
find . -name "Makefile" -delete
""" if clean_art_on_build else """
# To enable clearing of previous artifacts change [clean_build_artifacts_on_build] to true in file [config/app_settings.yml]
"""

    # Build system specific commands
    if use_premake:
        build_commands = """
echo "------ Regenerating Makefiles and rebuilding ------"
./vendor/premake/premake5 gmake2

# Convert build config to lowercase for Makefile compatibility
make_config=$(echo "$build_config" | tr '[:upper:]' '[:lower:]')
gmake config=${make_config}_x64 -j -k
"""
    else:
        build_commands = """
echo -e "${BLUE}------ Generating build files ------${NC}"
cmake -B build -DCMAKE_BUILD_TYPE=$build_config

echo ""
echo -e "${BLUE}------ Building main project ------${NC}"
cmake --build build --config $build_config --parallel 32
"""

    # Create build.sh
    build_script_path = os.path.join(vscode_dir, "build.sh")
    build_script_content = f"""#!/usr/bin/env bash
set -e

# Color codes
GREEN='\\033[0;32m'
YELLOW='\\033[1;33m'
RED='\\033[0;31m'
BLUE='\\033[0;34m'
WHITE='\\033[1;37m'
CYAN='\\033[0;36m'
NC='\\033[0m' # No Color

build_config="{build_config}"
timestamp=$(date '+%Y-%m-%d-%H:%M:%S')
stage_name="{application_name}_${{build_config}}_${{timestamp}}"
STAGE_DIR="{bin_dir}/${{stage_name}}"
{clean_artifacts_block}

echo -e "${{BLUE}}------ Checking plugins ------${{NC}}"

# Function to check if a plugin needs rebuilding with detailed output
needs_rebuild() {{
    local plugin_dir="$1"
    local plugin_name=$(basename "$plugin_dir")
    local build_dir="$plugin_dir/build"
    local so_file="$build_dir/lib$plugin_name.so"
    
    echo -e "${{WHITE}}  Checking plugin: ${{CYAN}}$plugin_name${{NC}}"
    
    # If no build directory exists
    if [ ! -d "$build_dir" ]; then
        echo -e "    ${{YELLOW}}- No build directory found${{NC}}"
        return 0
    fi
    
    # If no .so file exists
    if [ ! -f "$so_file" ]; then
        echo -e "    ${{YELLOW}}- No .so file found${{NC}}"
        return 0
    fi
    
    # Get the timestamp of the .so file
    local so_timestamp=$(stat -c %Y "$so_file" 2>/dev/null || echo 0)
    echo -e "    ${{WHITE}}- .so file timestamp: ${{NC}}$(date -d @$so_timestamp)"
    
    # Find the newest source file timestamp
    local newest_source_timestamp=0
    local newest_source_file=""
    
    if find "$plugin_dir" -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "CMakeLists.txt" | read -r first_file; then
        newest_source_timestamp=$(find "$plugin_dir" \\( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "CMakeLists.txt" \\) -type f -exec stat -c "%Y %n" {{}} \\; | sort -nr | head -n 1)
        newest_source_file=$(echo "$newest_source_timestamp" | cut -d' ' -f2-)
        newest_source_timestamp=$(echo "$newest_source_timestamp" | cut -d' ' -f1)
        echo -e "    ${{WHITE}}- Newest source: ${{NC}}$(basename "$newest_source_file") ($(date -d @$newest_source_timestamp))"
    else
        echo -e "    ${{YELLOW}}- No source files found${{NC}}"
        return 1
    fi
    
    # If sources are newer than the .so file
    if [ "$newest_source_timestamp" -gt "$so_timestamp" ]; then
        echo -e "    ${{YELLOW}}- Source files are newer than .so file${{NC}}"
        return 0
    fi
    
    echo -e "    ${{GREEN}}- Plugin is up to date${{NC}}"
    return 1
}}

# Function to build a plugin
build_plugin() {{
    local plugin_dir="$1"
    local plugin_name=$(basename "$plugin_dir")
    local build_dir="$plugin_dir/build"
    
    echo -e "${{WHITE}}  Building plugin: ${{CYAN}}$plugin_name${{NC}}"
    
    # Create build directory if it doesn't exist
    mkdir -p "$build_dir"
    
    # Build the plugin
    echo -e "    ${{BLUE}}Configuring CMake...${{NC}}"
    cd "$build_dir"
    if ! cmake .. -DCMAKE_BUILD_TYPE=$build_config; then
        echo -e "    ${{RED}} CMake configuration failed for $plugin_name${{NC}}"
        cd - > /dev/null
        return 1
    fi
    
    echo -e "    ${{BLUE}}Compiling...${{NC}}"
    if ! make -j$(nproc); then
        echo -e "    ${{RED}} Compilation failed for $plugin_name${{NC}}"
        cd - > /dev/null
        return 1
    fi
    cd - > /dev/null
    
    echo -e "    ${{GREEN}} Successfully built: $plugin_name${{NC}}"
    return 0
}}

# Main plugin checking logic
plugins_rebuilt=0
plugins_failed=0

if [ -d "./plugins" ]; then
    echo -e "${{WHITE}}Scanning plugins directory...${{NC}}"
    
    for plugin_dir in ./plugins/*; do
        if [ -d "$plugin_dir" ] && [ -f "$plugin_dir/CMakeLists.txt" ]; then
            if needs_rebuild "$plugin_dir"; then
                echo -e "  ${{YELLOW}}Rebuilding required${{NC}}"
                if build_plugin "$plugin_dir"; then
                    ((plugins_rebuilt++))
                else
                    ((plugins_failed++))
                fi
            else
                echo -e "  ${{GREEN}} Up to date${{NC}}"
            fi
            echo ""  # Empty line for readability
        fi
    done
else
    echo -e "${{YELLOW}}No plugins directory found${{NC}}"
fi

# Summary
echo -e "${{BLUE}}------ Build Summary ------${{NC}}"
if [ $plugins_rebuilt -gt 0 ]; then
    echo -e "${{GREEN}} Successfully rebuilt $plugins_rebuilt plugin(s)${{NC}}"
fi
if [ $plugins_failed -gt 0 ]; then
    echo -e "${{RED}} Failed to build $plugins_failed plugin(s)${{NC}}"
fi
if [ $plugins_rebuilt -eq 0 ] && [ $plugins_failed -eq 0 ]; then
    echo -e "${{GREEN}} All plugins are up to date${{NC}}"
fi

echo ""
{build_commands}

echo -e "${{GREEN}}------ Done ------${{NC}}"
"""
    with open(build_script_path, "w") as f:
        f.write(build_script_content)
    os.chmod(build_script_path, os.stat(build_script_path).st_mode | stat.S_IEXEC)

   

    # create tasks.json
    tasks_json_path = os.path.join(vscode_dir, "tasks.json")
    tasks_data = {
        "version": "2.0.0",
        "tasks": [
            {
                "label": "Clean & Build",
                "type": "shell",
                "command": "${workspaceFolder}/.vscode/build.sh",
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

    # create launch.json
    launch_json_path = os.path.join(vscode_dir, "launch.json")
    program_path = os.path.join("${workspaceFolder}", "build", "bin", output_dir, f"{application_name}", f"{application_name}")
    cwd_path = os.path.join("${workspaceFolder}", "build", "bin", output_dir)

    launch_data = {
        "version": "0.2.0",
        "configurations": [
            {
                "name": f"Launch {application_name} ({build_config})",
                "type": "cppdbg",
                "request": "launch",
                "program": program_path,
                "args": [],
                "cwd": cwd_path,
                "preLaunchTask": "Clean & Build",
                "stopAtEntry": False,
                "environment": [],
                "externalConsole": False,
                "MIMode": "gdb",
                "miDebuggerPath": "/usr/bin/gdb",
                "setupCommands": [
                    {
                        "description": "Enable pretty-printing for gdb",
                        "text": "-enable-pretty-printing",
                        "ignoreFailures": True
                    }
                ]
            }
        ]
    }
    with open(launch_json_path, "w") as f:
        json.dump(launch_data, f, indent=4)

    # c_cpp_properties.json         Always define PLATFORM_LINUX, plus one based on build_config
    config_define = {
        "debug": "DEBUG",
        "release_with_debug_info": "RELEASE_WITH_DEBUG_INFO",
        "release": "RELEASE"
    }.get(build_config.lower(), None)
    defines = ["PLATFORM_LINUX"]
    if config_define:
        defines.append(config_define)

    cpp_props = {
        "version": 4,
        "configurations": [
            {
                "name": f"{build_config} ({system}-{arch})",
                "includePath": [
                    "${workspaceFolder}/**"
                ],
                "defines": defines,
                "compilerPath": "/usr/bin/gcc",
                "cStandard": "c11",
                "cppStandard": "c++20",
                "intelliSenseMode": "linux-gcc-x64"
            }
        ]
    }
    cpp_props_path = os.path.join(vscode_dir, "c_cpp_properties.json")
    with open(cpp_props_path, "w") as f:
        json.dump(cpp_props, f, indent=4)

    utils.print_info("VSCode integration files generated in .vscode/")
    utils.print_c("\nVSCode usage", "blue")
    utils.print_info("  Build the project:                 Ctrl+Shift+B (runs 'Clean & Build')")
    utils.print_info("  Launch the application:            F5 (runs the debugger with selected config)")
