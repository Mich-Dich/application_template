import os
from pathlib import Path
import scripts.windows.win_utils as win_utils
from .. import utils

class premake_configuration:
    premake_version = "5.0.0-beta4"
    premake_zip_url = f"https://github.com/premake/premake-core/releases/download/v{premake_version}/premake-{premake_version}-windows.zip"
    premake_license_url = "https://raw.githubusercontent.com/premake/premake-core/master/LICENSE.txt"
    premake_directory = "./vendor/premake"
    
    # CI detection
    is_ci = os.getenv("CI") == "true"

    @classmethod
    def validate(cls):
        if cls.is_ci:
            utils.print_c("CI environment detected - auto-installing premake", "blue")
            return cls.install_premake_ci()
            
        if not cls.check_premake():
            utils.print_c("premake not installed correctly.", "red")
            return False
            
        return True

    @classmethod
    def check_premake(cls):
        premake_path = Path(f"{cls.premake_directory}/premake5.exe")
        if premake_path.exists():
            utils.print_c(f"Located premake", "green")
            return True
            
        if cls.is_ci:
            return False  # Let install_premake_ci handle it
            
        utils.print_c("You don't have premake5 installed!", "orange")
        return cls.install_premake_interactive()

    @classmethod
    def install_premake_ci(cls):
        """Automatically install premake in CI environments"""
        # Create directory if needed
        os.makedirs(cls.premake_directory, exist_ok=True)
        
        # Download premake
        premake_path = f"{cls.premake_directory}/premake-{cls.premake_version}-windows.zip"
        utils.print_c(f"Downloading premake {cls.premake_version}", "blue")
        utils.download_file(cls.premake_zip_url, premake_path)
        
        # Extract
        utils.print_c("Extracting premake", "blue")
        win_utils.unzip_file(premake_path, deleteZipFile=True)
        
        # Download license
        premake_license_path = os.path.join(cls.premake_directory, 'LICENSE.txt')
        utils.download_file(cls.premake_license_url, premake_license_path)
        
        # Verify installation
        if Path(f"{cls.premake_directory}/premake5.exe").exists():
            utils.print_c("Premake installed successfully", "green")
            return True
            
        utils.print_c("Premake installation failed", "red")
        return False

    @classmethod
    def install_premake_interactive(cls):
        """Install premake with user prompts (for non-CI environments)"""
        permission_granted = False
        while not permission_granted:
            reply = str(input(f"Install premake {cls.premake_version}? [Y/N]: ")).lower().strip()[:1]
            if reply == 'n':
                return False
            permission_granted = (reply == 'y')

        # Create directory
        os.makedirs(cls.premake_directory, exist_ok=True)
        
        premake_path = f"{cls.premake_directory}/premake-{cls.premake_version}-windows.zip"
        print(f"Downloading {cls.premake_zip_url} to {premake_path}")
        utils.download_file(cls.premake_zip_url, premake_path)
        
        print("Extracting", premake_path)
        win_utils.unzip_file(premake_path, deleteZipFile=True)
        print(f"Premake {cls.premake_version} installed to '{cls.premake_directory}'")

        # Download license
        premake_license_path = os.path.join(cls.premake_directory, 'LICENSE.txt')
        print(f"Downloading license to {premake_license_path}")
        utils.download_file(cls.premake_license_url, premake_license_path)

        return True
        
if __name__ == "__main__":
    premake_configuration.validate()
