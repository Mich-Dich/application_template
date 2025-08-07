import os
import subprocess
import scripts.utils as utils



def initialize_submodules():
    """Initialize and update all submodules with SSH/HTTPS fallback"""
    # Check if we're in CI environment
    is_ci = os.getenv("CI") == "true"
    workspace = os.getenv("GITHUB_WORKSPACE")  # Get workspace from environment
    
    if is_ci:
        # Use HTTPS for submodules in CI
        utils.print_c("CI environment detected, using HTTPS for submodules", "blue")
        try:
            gitmodules_path = os.path.join(workspace, '.gitmodules') if workspace else '.gitmodules'
            with open(gitmodules_path, 'r') as f:
                content = f.read()
            new_content = content.replace('git@github.com:', 'https://github.com/')
            new_content = new_content.replace('ssh://git@github.com/', 'https://github.com/')
            with open(gitmodules_path, 'w') as f:
                f.write(new_content)
            utils.print_c("Converted .gitmodules URLs to HTTPS", "green")
        except Exception as e:
            utils.print_c(f"Failed to convert URLs: {str(e)}", "red")
            return False

    # Initialize submodules
    try:
        utils.print_c("Configuring Git safe.directory", "blue")
        
        # Use workspace path if available
        cwd_path = workspace if workspace else os.getcwd()
        
        # Add safe directory configuration
        subprocess.run(
            ["git", "config", "--global", "--add", "safe.directory", cwd_path],
            check=True
        )
        utils.print_c(f"Added safe.directory: {cwd_path}", "green")

        utils.print_c("Initializing submodules...", "blue")
        git_command = ["git", "submodule", "update", "--init", "--recursive"]
        
        # Run Git command in the correct directory
        result = subprocess.run(
            git_command,
            cwd=cwd_path,
            capture_output=True,
            text=True,
            check=True
        )
        
        utils.print_c("Submodule initialization output:", "blue")
        utils.print_c(result.stdout, "cyan")
        
        utils.print_c("Submodules initialized successfully.", "green")
        return True
        
    except subprocess.CalledProcessError as e:
        utils.print_c(f"Submodule init failed with code {e.returncode}", "red")
        utils.print_c("Possible causes:", "yellow")
        utils.print_c("1. Repository corruption (try recloning)", "yellow")
        utils.print_c("2. Network issues (check GitHub status)", "yellow")
        utils.print_c("3. Permission issues (verify workspace ownership)", "yellow")
        utils.print_c("4. Invalid submodule URLs (check .gitmodules)", "yellow")
        utils.print_c("5. Safe directory misconfiguration", "yellow")
        utils.print_c("6. Incorrect working directory", "yellow")
        
        utils.print_c("\nGit error output:", "red")
        utils.print_c(e.stderr, "red")
        
        utils.print_c("\nGit command:", "red")
        utils.print_c(" ".join(e.cmd), "red")
        
        utils.print_c(f"\nCurrent working directory: {os.getcwd()}", "red")
        utils.print_c(f"Workspace path: {workspace}", "red")
        
        return False


def update_submodule(submodule_path, branch="main"):
    print(f"\nUpdating submodule: {os.path.basename(submodule_path)}")
    try:
        # Fetch the latest changes
        subprocess.run(["git", "fetch", "origin"], cwd=submodule_path, check=True)
        
        # Check if branch exists
        branch_check = subprocess.run(["git", "show-ref", "--verify", "--quiet", f"refs/remotes/origin/{branch}"], cwd=submodule_path)
        
        # Use default branch if specified branch doesn't exist
        if branch_check.returncode != 0:
            utils.print_c(f"Branch '{branch}' not found, trying default branch", "yellow")
            result = subprocess.run(["git", "symbolic-ref", "refs/remotes/origin/HEAD"], cwd=submodule_path, stdout=subprocess.PIPE, text=True, stderr=subprocess.DEVNULL )
            default_branch = result.stdout.strip().split('/')[-1] if result.returncode == 0 else "main"
            branch = default_branch
            utils.print_c(f"Using branch: {branch}", "yellow")
        
        # Checkout and update the branch
        subprocess.run(["git", "checkout", branch], cwd=submodule_path, check=True)
        subprocess.run(["git", "pull", "origin", branch], cwd=submodule_path, check=True)
        utils.print_c(f"Successfully updated {submodule_path} to branch '{branch}'.", "green")
        return True

    except subprocess.CalledProcessError as e:
        utils.print_c(f"Failed to update {submodule_path}: {e}", "red")
        return False