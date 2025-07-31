import os
from pathlib import Path
import scripts.linux.lin_utils as lin_utils
from .. import utils

class premake_configuration:
    premake_version = "5.0.0-beta4"
    premake_zip_urls = f"https://github.com/premake/premake-core/releases/download/v{premake_version}/premake-{premake_version}-linux.tar.gz"
    premake_license_url = "https://raw.githubusercontent.com/premake/premake-core/master/LICENSE.txt"
    premake_directory = "./vendor/premake"

    @classmethod
    def validate(cls):
        if (not cls.check_premake()):
            utils.print_c("premake not installed correctly.", "red")
            return False
        else:
            lin_utils.ensure_executable(f"{cls.premake_directory}/premake5")
            return True

    @classmethod
    def check_premake(cls):
        while True:
            premake_path = Path(f"{cls.premake_directory}/premake5")
            if premake_path.exists():
                break                                                               # found preamke5

            utils.print_c("You don't have premake5 downloaded!", "orange")
            if (cls.__install_premake() == False):
                return False

        utils.print_c(f"Located premake", "green")
        return True

    @classmethod
    def __install_premake(cls):
        permission_granted = False
        while not permission_granted:
            reply = str(input("Would you like to download premake {0:s}? [Y/N]: ".format(cls.premake_version))).lower().strip()[:1]
            if reply == 'n':
                return False
            permission_granted = (reply == 'y')

        premake_path = f"{cls.premake_directory}/premake-{cls.premake_version}-linux.tar.gz"
        print("Downloading {0:s} to {1:s}".format(cls.premake_zip_urls, premake_path))
        utils.download_file(cls.premake_zip_urls, premake_path)
        print("Extracting", premake_path)
        lin_utils.un_targz_file(premake_path, deleteTarGzFile=True)
        print(f"Premake {cls.premake_version} has been downloaded to '{cls.premake_directory}'")

        premake_license_path = os.path.join(cls.premake_directory, 'LICENSE.txt')
        license_directory = os.path.dirname(premake_license_path)
        os.makedirs(license_directory, exist_ok=True)

        # premake_license_path = f"{cls.premake_directory}/LICENSE.txt"
        print("Downloading Premake License file{0:s} to {1:s}".format(cls.premake_license_url, premake_license_path))
        utils.download_file(cls.premake_license_url, premake_license_path)

        return True

if __name__ == "__main__":
    premake_configuration.validate()
