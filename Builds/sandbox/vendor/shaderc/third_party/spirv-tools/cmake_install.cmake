# Install script for directory: D:/dev/vulkan-api-course/sandbox/vendor/shaderc/third_party/SPIRV-Tools

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/sandbox")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/external/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/source/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/tools/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/test/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/examples/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/spirv-tools" TYPE FILE FILES
    "D:/dev/vulkan-api-course/sandbox/vendor/shaderc/third_party/SPIRV-Tools/include/spirv-tools/libspirv.h"
    "D:/dev/vulkan-api-course/sandbox/vendor/shaderc/third_party/SPIRV-Tools/include/spirv-tools/libspirv.hpp"
    "D:/dev/vulkan-api-course/sandbox/vendor/shaderc/third_party/SPIRV-Tools/include/spirv-tools/optimizer.hpp"
    "D:/dev/vulkan-api-course/sandbox/vendor/shaderc/third_party/SPIRV-Tools/include/spirv-tools/linker.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES
    "D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/SPIRV-Tools.pc"
    "D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/SPIRV-Tools-shared.pc"
    )
endif()

