import os
from pathlib import Path
import scripts.linux.lin_utils as lin_utils
import subprocess
import requests
import tarfile
from .. import utils

class env_configuration:
    required_packages = [
        "build-essential",      # GCC, make, etc.
        "cmake",                # CMake build system
        "libgl1-mesa-dev",      # OpenGL development files
        "libglfw3-dev",         # GLFW development files
        "libglew-dev",          # GLEW development files
        "libassimp-dev",        # Assimp model loading library
        "libxinerama-dev",      # X11 input handling
        "libxcursor-dev",       # X11 cursor handling
        "libxi-dev",            # X11 input extension
        "xorg-dev",             # X11 development metapackage
        "pkg-config",           # Library configuration tool
        "qtbase5-dev",          # Qt core libraries
        "qttools5-dev",         # Qt tools
        "qttools5-dev-tools",   # Qt development tools
        "git"                   # Git version control
    ]
    
    @classmethod
    def validate(cls):
        """Check for required development packages on Linux systems"""
        utils.print_u("\nCHECKING SYSTEM DEPENDENCIES")
        missing_packages = cls.__check_missing_packages()
        
        # Install standard packages first
        if missing_packages:
            utils.print_c(f"Missing packages: {', '.join(missing_packages)}", "yellow")
            if not cls.__install_packages(missing_packages):
                return False

        utils.print_c("All required dependencies are installed", "green")
        return True

    @classmethod
    def __check_missing_packages(cls):
        """Check which required packages are not installed"""
        missing = []
        for package in cls.required_packages:
            result = subprocess.run(
                ["dpkg", "-s", package],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
            if result.returncode != 0:
                missing.append(package)
        return missing

    @classmethod
    def __install_packages(cls, packages):
        """Install missing packages with user confirmation"""
        utils.print_c("Packages required for engine development:", "blue")
        for pkg in packages:
            print(f"  - {pkg}")
            
        reply = input("\nInstall missing packages? [Y/n]: ").strip().lower()
        if reply and reply[0] != 'y':
            utils.print_c("Package installation aborted by user", "red")
            return False
            
        try:
            utils.print_c("Updating package lists...", "blue")
            subprocess.run(["sudo", "apt", "update"], check=True)
            
            utils.print_c(f"Installing packages: {', '.join(packages)}", "blue")
            subprocess.run(
                ["sudo", "apt", "install", "-y"] + packages,
                check=True
            )
            utils.print_c("Package installation completed", "green")
            return True
        except subprocess.CalledProcessError as e:
            utils.print_c(f"Package installation failed: {e}", "red")
            return False
