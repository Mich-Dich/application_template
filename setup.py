import sys
import subprocess

# Check for required Python packages
required_modules = {
    'yaml': 'PyYAML'
}

for module, package in required_modules.items():
    try:
        __import__(module)
    except ImportError:
        # Check if pip is available
        try:
            subprocess.run([sys.executable, "-m", "pip", "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
        except (subprocess.CalledProcessError, FileNotFoundError):
            print(f"Error: pip is not installed. Please install pip to proceed with installing {package}.")
            sys.exit(1)

        print(f"Installing required package: {package}")
        try:
            subprocess.check_call([sys.executable, "-m", "pip", "install", package])
        except subprocess.CalledProcessError:
            print(f"Failed to install {package}. Please install it manually by running: pip install {package}")
            sys.exit(1)

import os
import re
import yaml
import shutil
import socket
import argparse
import platform
import scripts.utils as utils
import scripts.git_util as git_util
IS_CI = os.getenv("CI") == "true"

import scripts.create_build_system as CBS

if platform.system() == "Linux":
    from scripts.linux.setup_env import env_configuration
    import scripts.linux.IDE_setup as IDE_setup
elif platform.system() == "Windows":
    from scripts.windows.setup_env import env_configuration
    import scripts.windows.win_utils as win_util
    win_util.enable_ansi_support()
    import scripts.windows.IDE_setup as IDE_setup
else:
    raise Exception("Unsupported operating system")


def get_application_name(file_path="./config/app_settings.yml"):
    try:
        with open(file_path, 'r') as stream:
            data = yaml.safe_load(stream)
            return data['general_settings']['name']
    except Exception as e:
        utils.print_c(f"Error while reading settings file: {str(e)}", "red")
        sys.exit(1)


def get_clean_build_artifacts_on_build(file_path="./config/app_settings.yml"):
    try:
        with open(file_path, 'r') as stream:
            data = yaml.safe_load(stream)
            return data['general_settings']['clean_build_artifacts_on_build']
    except Exception as e:
        utils.print_c(f"Error while reading settings file: {str(e)}", "red")
        sys.exit(1)


def clean_build_artifacts():
    """Clean build artifacts and generated files"""
    folders_to_clean = [
        "build",
        "bin",
        "bin-int", 
        "vendor/glfw/build",
        "vendor/Catch2/Build",
        ".vscode"  # Optional: remove if you don't want to clean VSCode configs
    ]
    
    files_to_clean = [
        "Makefile",
        "*.workspace",
        "*.sln",
        "*.vcxproj*"
    ]
    
    for folder in folders_to_clean:
        if os.path.exists(folder):
            utils.print_c(f"Removing {folder}", "yellow")
            shutil.rmtree(folder)
    
    for pattern in files_to_clean:
        for file in glob.glob(pattern):
            utils.print_c(f"Removing {file}", "yellow")
            os.remove(file)


def main():

    parser = argparse.ArgumentParser(
        description='Application Template Setup Script',
        formatter_class=argparse.RawDescriptionHelpFormatter, 
        epilog='''
Examples:
  python3 setup.py                    # Interactive setup with all prompts
  python3 setup.py -k                 # Use CMake build system (skip selection)
  python3 setup.py -p                 # Use Premake build system (skip selection)
  python3 setup.py -V                 # Setup for VSCode (skip IDE selection)
  python3 setup.py -n                 # Generate project files without building
  python3 setup.py -c                 # Clean build artifacts before setup
  python3 setup.py -C Release         # Set build configuration
  python3 setup.py -i -N              # Offline setup (ignore submodules, no internet check)

Common workflows:
  Development:  python3 setup.py -k -V -C Debug
  Release:      python3 setup.py -k -C Release -n
  Quick setup:  python3 setup.py -k -V -n
        '''
    )
    
    # Build system group
    build_group = parser.add_argument_group('Build System Selection')
    build_exclusive = build_group.add_mutually_exclusive_group()
    build_exclusive.add_argument('-p', '--premake', action='store_true', help='Use Premake build system (skip selection prompt)')
    build_exclusive.add_argument('-k', '--cmake', action='store_true', help='Use CMake build system (skip selection prompt)')
    
    # IDE selection group
    ide_group = parser.add_argument_group('IDE Selection')
    ide_exclusive = ide_group.add_mutually_exclusive_group()
    ide_exclusive.add_argument('-V', '--vscode', action='store_true', help='Setup for VSCode (skip IDE selection)')
    ide_exclusive.add_argument('-S', '--visual-studio', action='store_true', help='Setup for Visual Studio (Windows only)')
    ide_exclusive.add_argument('-r', '--rider', action='store_true', help='Setup for JetBrains Rider')
    ide_exclusive.add_argument('-m', '--makefile', action='store_true', help='Setup for Makefile (Linux only)')
    
    # Build configuration group
    config_group = parser.add_argument_group('Build Configuration')
    config_group.add_argument('-C', '--config', choices=['Debug', 'Release', 'RelWithDebInfo'], default='Debug', help='Build configuration (default: Debug)')
    
    # Behavior flags group
    behavior_group = parser.add_argument_group('Behavior Flags')
    behavior_group.add_argument('-s', '--skip-submods', action='store_true', help='Skip submodule updating')
    behavior_group.add_argument('-N', '--no-net-check', action='store_true', help='Skip internet connectivity check')
    behavior_group.add_argument('-n', '--no-build', action='store_true', help='Generate project files without building')
    behavior_group.add_argument('-c', '--clean', action='store_true', help='Clean build artifacts before setup')
    behavior_group.add_argument('-f', '--force', action='store_true', help='Force overwrite existing configurations')
    behavior_group.add_argument('-q', '--quiet', action='store_true', help='Enable silent mode (suppress non-essential output)')
    
    args = parser.parse_args()
    
    # Set verbose mode early
    if args.quiet:
        utils.SILENT = True
            

    # Skip internet check in CI or if --no-internet-check is passed
    if not IS_CI and not args.no_net_check:
        try:
            socket.setdefaulttimeout(3)
            socket.socket(socket.AF_INET, socket.SOCK_STREAM).connect(("8.8.8.8", 53))
        except:
            utils.print_c("\nNo Internet connection found\n", "red")
            sys.exit(1)

    try:

        # Clean build artifacts if requested
        if args.clean:
            utils.print_u("\nCLEANING BUILD ARTIFACTS")
            clean_build_artifacts()

        utils.print_u("\nCHECKING SYSTEM DEPENDENCIES")
        if not env_configuration.validate():
            utils.print_c("Missing required packages - setup aborted", "red")
            sys.exit(1)
    
        utils.print_u("\nINITIALIZING SUBMODULES")              # Initialize submodule configuration
        if not git_util.initialize_submodules():
            utils.print_c("Submodule initialization failed - setup aborted", "red")
            sys.exit(1)
    
        if not args.skip_submods:
            utils.print_u("\nUPDATING SUBMODULES")                  # Update submodules to desired branches
            git_util.update_submodule("vendor/glfw", "main")
            git_util.update_submodule("vendor/glm", "master")
            git_util.update_submodule("vendor/imgui", "docking")
            git_util.update_submodule("vendor/implot", "master")
            git_util.update_submodule("vendor/Catch2", "devel")
        else:
            utils.print_c("ignoring update routine for submodules", "orange")

        utils.print_u("\nAPPLY SETTINGS")
        utils.print_c("Settings are defined at [./config/app_settings.yml]. after changing the settings, it is recommended to re-execute the setup script", "blue")
        # apply_premake_settings()
        application_name = get_application_name()
        clean_art_on_build = get_clean_build_artifacts_on_build()
        utils.print_info(f"name: {application_name}")
        shutil.copy2("./scripts/imgui_CMakeLists.txt", "./vendor/imgui/CMakeLists.txt")         # copy template CMake file to correct directory

        # select build system
        utils.print_u("\nSETUP A BUILD SYSTEM")              # Initialize submodule configuration

        
        if args.premake:
            build_sys = "Premake5"
        elif args.cmake:
            build_sys = "CMake"
        else:
            print("Available build systems:")
            build_sys = CBS.prompt_build_sys_selection()

        if build_sys == "Premake5":
            CBS.apply_premake_build_system(application_name)
        else:   # currently only supporting 2 systems -> CMake is default at error
            CBS.apply_cmake_build_system(application_name)

        # setup IDE
        utils.print_u("\nSETUP IDE")

        if IS_CI:                                       # select first detected IDE in CI
            IDEs = IDE_setup.detect_IDEs()
            selected_ide = IDEs[0]
            print(f"Selected IDE: {selected_ide}")
        else:
            # Use command line selection if provided
            if args.vscode:
                selected_ide = "VSCode"
            elif args.visual_studio:
                selected_ide = "Visual Studio 2022"  # Adjust based on detection
            elif args.rider:
                selected_ide = "JetBrains Rider"
            elif args.makefile:
                selected_ide = "Makefile"
            else:
                selected_ide = IDE_setup.prompt_ide_selection()

        # VSCode spcific setup
        build_config = args.config
        if "VSCode" in selected_ide and not (args.vscode or IS_CI):
            # Only prompt if VSCode was not explicitly chosen via command line
            # AND config was not explicitly set via -C/--config
            if not any(arg in sys.argv for arg in ['-C', '--config']):
                build_config = IDE_setup.prompt_build_config()
            
            IDE_setup.setup_vscode_configs(os.getcwd(), build_config, application_name, clean_art_on_build)
        
        if build_sys == "Premake5":
            if platform.system() == "Linux":                # ---- LINUX VERSION ----
                # prepare premake command
                premake_action = "gmake2"                   # Default to VSCode
                if selected_ide == "JetBrains Rider":
                    premake_action = "rider"
                elif "VSCode" in selected_ide:
                    premake_action = "gmake2"
                elif "Makefile" in selected_ide:
                    premake_action = "gmake2"

                utils.print_c(f"\nCreating Build files using [vendor/premake/premake5 {premake_action}]", "blue")
                premake_result = subprocess.run(['vendor/premake/premake5', premake_action], text=True)

            else:                                           # ---- WINDOWS VERSION ----
                # prepare premake command
                premake_action = "vs2022"                   # Default to VS2022
                if selected_ide == "VSCode":
                    premake_action = "gmake2"               # For MinGW-based builds
                elif "Visual Studio" in selected_ide:       # Map IDE selection to premake action
                    if "2022" in selected_ide:
                        premake_action = "vs2022"
                    elif "2019" in selected_ide:
                        premake_action = "vs2019"
                    elif "2017" in selected_ide:
                        premake_action = "vs2017"
                elif selected_ide == "JetBrains Rider":
                    premake_action = "rider"
                
                utils.print_c(f"\nCreating Build files using [vendor\\premake\\premake5.exe {premake_action}]", "blue")
                premake_result = subprocess.run(['vendor\\premake\\premake5.exe', premake_action], text=True)

            if premake_result.returncode != 0:
                utils.print_c(f"BUILD FAILED! Premake script encountered errors [{premake_result.returncode}]", "red")
            else:
                utils.print_c("Successfully generated build files and precompiled some libs", "green")

                # continue to compile only if generation was successful
                if not args.no_build:
                    utils.print_c("\nBuilding project using [gmake -j]", "blue")
                    premake_result = subprocess.run(["gmake", "-j"])
                    if premake_result.returncode == 0:
                        utils.print_c("BUILD SUCCESSFUL!", "green")
                    else:
                        utils.print_c(f"BUILD FAILED!", "red")

            # print helpful hints
            if platform.system() == "Linux":                # ---- LINUX VERSION ----
                utils.print_c("\nHelpful hints", "blue")
                utils.print_info("  Apply changed premake scripts:     vendor/premake/premake5 gmake2")
                utils.print_info("  Cleanup generated files:           gmake clean")
                utils.print_info("  Compile application:               gmake -j")
                utils.print_info("  More help:                         vendor/premake/premake5 --help OR https://premake.github.io/docs/Using-Premake/")

            else:                                           # ---- WINDOWS VERSION ----
                # Print helpful hints
                utils.print_c("\nHelpful hints for Windows", "blue")
                if "Visual Studio" in selected_ide or "Rider" in selected_ide:
                    utils.print_info("  Open solution file:               *.sln")
                    utils.print_info("  Clean solution:                   msbuild /t:Clean")
                    utils.print_info("  Build solution:                   msbuild /t:Build")
                elif selected_ide == "VSCode":
                    utils.print_info("  Apply changed premake scripts:    vendor\\premake\\premake5.exe gmake2")
                    utils.print_info("  Clean solution:                   gmake clean")
                    utils.print_info("  Build solution:                   gmake -j")
                utils.print_info("  More help:                        vendor\\premake\\premake5.exe --help")


        else:       # Default is CMake
            utils.print_c(f"\nCreating Build files using [cmake -B build {build_config}]", "blue")
            build_config = "-DCMAKE_BUILD_TYPE=" + build_config
            cmake_result = subprocess.run(["cmake", "-B", "build", build_config])
            if cmake_result.returncode != 0:
                utils.print_c("CMake configuration failed!", "red")
            else:
                utils.print_c("CMake configuration successful!", "green")

                # continue to compile only if generation was successful
                if not args.no_build:
                    utils.print_c("\nBuilding project using [cmake --build build]", "blue")
                    cmake_result = subprocess.run(["cmake", "--build", "build"])
                    if cmake_result.returncode == 0:
                        utils.print_c("BUILD SUCCESSFUL!", "green")
                    else:
                        utils.print_c("BUILD FAILED!", "red")

            # dont need platform if
            utils.print_c("\nHelpful hints", "blue")
            utils.print_info("  Build from the project root directory:")
            utils.print_info(f"    Generate build files :              cmake -B build {build_config}")
            utils.print_info("    Build project:                      cmake --build build --parallel 16")

            utils.print_info("  Build with manually creating 'build':")
            utils.print_info("    create build dir:                   mkdir build && cd build")
            utils.print_info(f"    Generate build files :              cmake .. {build_config}")
            utils.print_info("    Build project:                      make -j")

        utils.print_u("\nQUICK REFERENCE")
        utils.print_info("Common commands:")
        utils.print_info("  Setup with different options:")
        utils.print_info("    Development:      python3 setup.py -k -V -C Debug")
        utils.print_info("    Release:          python3 setup.py -k -C Release -n")
        utils.print_info("    Quick setup:      python3 setup.py -k -V -n")
        utils.print_info("\n  Re-run setup:")
        utils.print_info("    python3 setup.py -c -k -V             # Full clean setup")
        utils.print_info("    python3 setup.py -n -C Release        # Change config only")
        utils.print_info("\nFor more options:      python3 setup.py -h")

    except KeyboardInterrupt:
        utils.print_c("\nProcess interrupted by user.", "red")


if __name__ == "__main__":
    main()
