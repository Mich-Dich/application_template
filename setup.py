
import os
import sys
import subprocess
import platform
import socket
import scripts.utils as utils

if platform.system() == "Linux":
    from scripts.linux.setup_env import env_configuration                  # <= create a file and use here
    from scripts.linux.setup_premake import premake_configuration
    import scripts.linux.IDE_setup as IDE_setup
else:
    raise Exception("Unsupported operating system")



def initialize_submodules():
    """Initialize and update all submodules"""
    try:
        utils.print_u("\nINITIALIZING SUBMODULES")
        subprocess.run(["git", "submodule", "update", "--init", "--recursive"], check=True)
        utils.print_c("Submodules initialized successfully.", "green")
        return True
    except subprocess.CalledProcessError as e:
        utils.print_c(f"Failed to initialize submodules: {e}", "red")
        return False

def update_submodule(submodule_path, branch="main"):
    print(f"\nUpdating submodule: {os.path.basename(submodule_path)}")
    try:
        # Ensure the submodule directory exists
        if not os.path.exists(submodule_path):
            utils.print_c(f"Submodule directory {submodule_path} does not exist", "red")
            return False
            
        # Fetch the latest changes
        subprocess.run(["git", "fetch", "origin"], cwd=submodule_path, check=True)
        
        # Check if branch exists in the repository
        branch_check = subprocess.run(["git", "show-ref", "--verify", "--quiet", f"refs/remotes/origin/{branch}"], cwd=submodule_path)
        
        # If branch doesn't exist, try with the default branch
        if branch_check.returncode != 0:
            utils.print_c(f"Branch '{branch}' not found, trying default branch", "yellow")
            default_branch = subprocess.run(["git", "symbolic-ref", "refs/remotes/origin/HEAD"], cwd=submodule_path, stdout=subprocess.PIPE, text=True, check=True).stdout.strip().split('/')[-1]
            
            if default_branch:
                utils.print_c(f"Using default branch: {default_branch}", "yellow")
                branch = default_branch
        
        # Checkout and update the branch
        subprocess.run(["git", "checkout", branch], cwd=submodule_path, check=True)
        subprocess.run(["git", "pull", "origin", branch], cwd=submodule_path, check=True)
        
        utils.print_c(f"Successfully updated {submodule_path} to branch '{branch}'.", "green")
        return True

    except subprocess.CalledProcessError as e:
        utils.print_c(f"Failed to update {submodule_path}: {e}", "red")
        return False


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
    
    # Initialize all submodules first
    if not initialize_submodules():
        utils.print_c("Submodule initialization failed - setup aborted", "red")
        sys.exit(1)
    
    update_submodule("vendor/glfw", "main")
    update_submodule("vendor/imgui", "docking")
    update_submodule("vendor/glm", "master")

    # utils.print_u("\nCHECKING PYTHON SETUP")
    # python_installed = python_requirements.validate()

    # if (True == python_installed == premake_installed):
    if (True == premake_installed):                         # DISABLE "python_installed" for now
        if platform.system() == "Windows":

            utils.print_c("Windows not supported yet", "red")

        else:               # because of [Load platform dependent script] only remaining option is Linux

            utils.print_u("\nCHECK WORKSPACE SETUP")
            
            selected_ide = IDE_setup.prompt_ide_selection()
            premake_action = "gmake2"  # Default action
            if selected_ide == "JetBrains Rider":
                premake_action = "rider"
            elif "VSCode" in selected_ide:
                premake_action = "gmake2"
            elif "Makefile" in selected_ide:
                premake_action = "gmake2"
            
            premake_result = subprocess.run(['vendor/premake/premake5', premake_action], text=True)
            if premake_result.returncode != 0:
                utils.print_c(f"BUILD FAILED! the premake script encountered [{premake_result.returncode}] errors", "red")
            else:
                utils.print_c("BUILD SUCCESSFUL!", "green")


            if "VSCode" in selected_ide:                                                # generate build/compile tasks
                premake_action = "gmake2"
                build_config = IDE_setup.prompt_build_config()
                IDE_setup.setup_vscode_configs(os.getcwd(), build_config)

            utils.print_c("\nhelpful hints", "blue")
            print("  apply changed premake scripts:     vendor/premake/premake5 gmake2")
            print("  cleanup all generated files:       make clean")
            print("  compile application:               make -j             (-j for multiprocessor compilation)")
            print("  for more help:                     vendor/premake/premake5 --help OR visit[https://premake.github.io/docs/Using-Premake/]")
        
    else:
        # Print error message if any requirements failed
        failed_requirements = []
        if not python_installed:
            failed_requirements.append("Python")
        if not premake_installed:
            failed_requirements.append("Premake5")

except KeyboardInterrupt:
    utils.print_c("\nProcess interrupted by user.", "red")
