import os
import sys
import subprocess
import platform
import socket
import scripts.utils as utils

if platform.system() == "Linux":
    from scripts.linux.setup_env import env_configuration
    from scripts.linux.setup_premake import premake_configuration
    import scripts.linux.IDE_setup as IDE_setup
else:
    raise Exception("Unsupported operating system")

def initialize_submodules():
    """Initialize and update all submodules with SSH/HTTPS fallback"""
    utils.print_u("\nINITIALIZING SUBMODULES")
    
    # Check if SSH authentication is available
    ssh_available = False
    try:
        result = subprocess.run(["ssh", "-T", "git@github.com"], stderr=subprocess.PIPE, stdout=subprocess.DEVNULL, text=True)
        ssh_available = "successfully authenticated" in result.stderr.lower()
    except:
        pass  # SSH command not available or failed

    # Convert SSH URLs to HTTPS if SSH not available
    if not ssh_available:
        utils.print_c("SSH not available, converting to HTTPS...", "yellow")
        try:
            with open('.gitmodules', 'r') as f:
                content = f.read()
            new_content = content.replace('git@github.com:', 'https://github.com/')
            new_content = new_content.replace('ssh://git@github.com/', 'https://github.com/')
            with open('.gitmodules', 'w') as f:
                f.write(new_content)
            subprocess.run(["git", "submodule", "sync"], check=True)
        except Exception as e:
            utils.print_c(f"Failed to convert URLs: {str(e)}", "red")
            return False

    # Initialize submodules (without fetching)
    try:
        subprocess.run(["git", "submodule", "init"], check=True)
        utils.print_c("Submodules initialized successfully.", "green")
        return True
    except subprocess.CalledProcessError as e:
        utils.print_c(f"Failed to initialize submodules: {e}", "red")
        return False

def update_submodule(submodule_path, branch="main"):
    print(f"\nUpdating submodule: {os.path.basename(submodule_path)}")
    try:
        # Initialize and fetch if directory doesn't exist
        if not os.path.exists(submodule_path):
            utils.print_c(f"Initializing and fetching {submodule_path}...", "yellow")
            subprocess.run(["git", "submodule", "update", "--init", "--", submodule_path], check=True)
        
        # Fetch the latest changes
        subprocess.run(["git", "fetch", "origin"], cwd=submodule_path, check=True)
        
        # Check if branch exists
        branch_check = subprocess.run(
            ["git", "show-ref", "--verify", "--quiet", f"refs/remotes/origin/{branch}"],
            cwd=submodule_path
        )
        
        # Use default branch if specified branch doesn't exist
        if branch_check.returncode != 0:
            utils.print_c(f"Branch '{branch}' not found, trying default branch", "yellow")
            result = subprocess.run(
                ["git", "symbolic-ref", "refs/remotes/origin/HEAD"],
                cwd=submodule_path,
                stdout=subprocess.PIPE,
                text=True,
                stderr=subprocess.DEVNULL
            )
            default_branch = result.stdout.strip().split('/')[-1] if result.returncode == 0 else None
            branch = default_branch or "main"
            utils.print_c(f"Using branch: {branch}", "yellow")
        
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
    
    # Initialize submodule configuration
    if not initialize_submodules():
        utils.print_c("Submodule initialization failed - setup aborted", "red")
        sys.exit(1)
    
    # Update submodules to desired branches
    update_submodule("vendor/glfw", "main")
    update_submodule("vendor/imgui", "docking")
    update_submodule("vendor/glm", "master")

    if premake_installed:
        if platform.system() == "Windows":
            utils.print_c("Windows not supported yet", "red")
        else:
            utils.print_u("\nCHECK WORKSPACE SETUP")
            selected_ide = IDE_setup.prompt_ide_selection()
            premake_action = "gmake2"
            if selected_ide == "JetBrains Rider":
                premake_action = "rider"
            elif "VSCode" in selected_ide:
                premake_action = "gmake2"
            elif "Makefile" in selected_ide:
                premake_action = "gmake2"
            
            premake_result = subprocess.run(['vendor/premake/premake5', premake_action], text=True)
            if premake_result.returncode != 0:
                utils.print_c(f"BUILD FAILED! Premake script encountered errors [{premake_result.returncode}]", "red")
            else:
                utils.print_c("BUILD SUCCESSFUL!", "green")

            if "VSCode" in selected_ide:
                build_config = IDE_setup.prompt_build_config()
                IDE_setup.setup_vscode_configs(os.getcwd(), build_config)

            utils.print_c("\nHelpful hints", "blue")
            print("  Apply changed premake scripts:     vendor/premake/premake5 gmake2")
            print("  Cleanup generated files:           make clean")
            print("  Compile application:               make -j")
            print("  More help:                         vendor/premake/premake5 --help OR https://premake.github.io/docs/Using-Premake/")
        
    else:
        utils.print_c("Premake5 installation failed - setup aborted", "red")
        sys.exit(1)

except KeyboardInterrupt:
    utils.print_c("\nProcess interrupted by user.", "red")