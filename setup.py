
import os
import sys
import subprocess
import platform
import socket
import scripts.utils as utils

if platform.system() == "Linux":
    from scripts.linux.setup_premake import premake_configuration
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

    utils.print_u("\nCHECK PREMAKE-5 SETUP")
    premake_installed = premake_configuration.validate()
    
    
except KeyboardInterrupt:
    utils.print_c("\nProcess interrupted by user.", "red")
