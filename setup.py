import os
import sys
import subprocess
import platform
import socket
import scripts.utils as utils
import scripts.git_util as git_util
IS_CI = os.getenv("CI") == "true"

if platform.system() == "Linux":
    from scripts.linux.setup_env import env_configuration
    from scripts.linux.setup_premake import premake_configuration
    import scripts.linux.IDE_setup as IDE_setup
elif platform.system() == "Windows":
    import scripts.windows.win_utils as win_util
    win_util.enable_ansi_support()
    from scripts.windows.setup_premake import premake_configuration
    import scripts.windows.IDE_setup as IDE_setup
else:
    raise Exception("Unsupported operating system")



def main():
    # Skip internet check in CI
    if not IS_CI:
        try:
            socket.setdefaulttimeout(3)
            socket.socket(socket.AF_INET, socket.SOCK_STREAM).connect(("8.8.8.8", 53))
        except:
            utils.print_c("\nNo Internet connection found\n", "red")
            sys.exit(1)

    try:
        if platform.system() == "Linux":
            if not env_configuration.validate():
                utils.print_c("Missing required packages - setup aborted", "red")
                sys.exit(1)

        utils.print_u("\nCHECK PREMAKE-5 SETUP")
        premake_installed = premake_configuration.validate()
        if not premake_installed:
            utils.print_c("Premake5 installation failed - setup aborted", "red")
            sys.exit(1)
    
        # Initialize submodule configuration
        if not git_util.initialize_submodules():
            utils.print_c("Submodule initialization failed - setup aborted", "red")
            sys.exit(1)
    
        # Update submodules to desired branches
        git_util.update_submodule("vendor/glfw", "main")
        git_util.update_submodule("vendor/glm", "master")
        git_util.update_submodule("vendor/imgui", "docking")
        git_util.update_submodule("vendor/implot", "master")

        # setup IDE
        utils.print_u("\nSETUP IDE")

        if IS_CI:                                       # select first detected IDE in CI
            IDEs = IDE_setup.detect_IDEs()
            selected_ide = IDEs[0]
            print(f"Selected IDE: {selected_ide}")
        else:
            selected_ide = IDE_setup.prompt_ide_selection()

        # VSCode spcific setup
        if "VSCode" in selected_ide:
            if IS_CI:
                build_config = "Debug"
            else:
                build_config = IDE_setup.prompt_build_config()
            IDE_setup.setup_vscode_configs(os.getcwd(), build_config)
        

        if platform.system() == "Linux":                # ---- LINUX VERSION ----
            # prepare premake command
            premake_action = "gmake2"  # Default to VSCode
            if selected_ide == "JetBrains Rider":
                premake_action = "rider"
            elif "VSCode" in selected_ide:
                premake_action = "gmake2"
            elif "Makefile" in selected_ide:
                premake_action = "gmake2"

            premake_result = subprocess.run(['vendor/premake/premake5', premake_action], text=True)

        else:                                           # ---- WINDOWS VERSION ----
            # prepare premake command
            premake_action = "vs2022"  # Default to VS2022
            if selected_ide == "VSCode":
                premake_action = "gmake2"  # For MinGW-based builds
            elif "Visual Studio" in selected_ide:       # Map IDE selection to premake action
                if "2022" in selected_ide:
                    premake_action = "vs2022"
                elif "2019" in selected_ide:
                    premake_action = "vs2019"
                elif "2017" in selected_ide:
                    premake_action = "vs2017"
            elif selected_ide == "JetBrains Rider":
                premake_action = "rider"
            
            premake_result = subprocess.run(['vendor/premake/premake5.exe', premake_action], text=True)

        if premake_result.returncode != 0:
            utils.print_c(f"BUILD FAILED! Premake script encountered errors [{premake_result.returncode}]", "red")
        else:
            utils.print_c("BUILD SUCCESSFUL!", "green")


        # print helpful hints
        if platform.system() == "Linux":                # ---- LINUX VERSION ----
            utils.print_c("\nHelpful hints", "blue")
            print("  Apply changed premake scripts:     vendor/premake/premake5 gmake2")
            print("  Cleanup generated files:           gmake clean")
            print("  Compile application:               gmake -j")
            print("  More help:                         vendor/premake/premake5 --help OR https://premake.github.io/docs/Using-Premake/")

        else:                                           # ---- WINDOWS VERSION ----
            # Print helpful hints
            utils.print_c("\nHelpful hints for Windows", "blue")
            if "Visual Studio" in selected_ide or "Rider" in selected_ide:
                print("  Open solution file:               build/*.sln")
                print("  Clean solution:                   msbuild /t:Clean")
                print("  Build solution:                   msbuild /t:Build")
            elif selected_ide == "VSCode":
                print("  Apply changed premake scripts:    vendor/premake/premake5 gmake2")
                print("  Clean solution:                   gmake clean")
                print("  Build solution:                   gmake -j")
            print("  More help:                        vendor/premake/premake5 --help")


    except KeyboardInterrupt:
        utils.print_c("\nProcess interrupted by user.", "red")


if __name__ == "__main__":
    main()
