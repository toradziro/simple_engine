#!/usr/bin/env python3
import os
import platform
import subprocess
import sys
from pathlib import Path

def check_vulkan_sdk():
    vulkan_sdk_path = os.environ.get('VULKAN_SDK_PATH')
    if not vulkan_sdk_path:
        print("Error: The environment variable VULKAN_SDK_PATH is not set.")
        print("Please set VULKAN_SDK_PATH to the path of the Vulkan SDK.")
        sys.exit(1)
    print(f"Vulkan SDK detected: {vulkan_sdk_path}")
    return vulkan_sdk_path

def create_build_directory():
    """Creates the build directory if it does not exist."""
    build_dir = Path('build')
    if not build_dir.exists():
        print("Создание директории build...")
        build_dir.mkdir()
    return build_dir

def generate_project_files(build_dir):
    """"""
    os_name = platform.system()
    
    os.chdir(build_dir)
    
    if os_name == "Windows":
        print("Project file generation for... Visual Studio...")
        generator = "Visual Studio 17 2022"
        cmd = ["cmake", "..", "-G", generator, "-A", "x64"]
        subprocess.run(cmd, check=True)
        return True
    elif os_name == "Linux":
        print("Generating Makefile for Linux...")
        subprocess.run(["cmake", ".."], check=True)
        
    else:
        subprocess.run(["cmake", ".."], check=True)
    
    return True

def main():
    # Check Vulkan SDK
    check_vulkan_sdk()
    
    # Create build dir
    build_dir = create_build_directory()
    
    # Generate project files
    if generate_project_files(build_dir):
        print("\n=== SUCCESS ===")
    else:
        print("\n=== FAIL ===")

if __name__ == "__main__":
    main()