# Install script for directory: D:/dev/vulkan-api-course/sandbox/vendor/shaderc/third_party/SPIRV-Tools/source/lint

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

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/dev/vulkan-api-course/Builds/lib/Debug/SPIRV-Tools-lint.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/dev/vulkan-api-course/Builds/lib/Release/SPIRV-Tools-lint.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/dev/vulkan-api-course/Builds/lib/MinSizeRel/SPIRV-Tools-lint.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/dev/vulkan-api-course/Builds/lib/RelWithDebInfo/SPIRV-Tools-lint.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/SPIRV-Tools-lint/cmake/SPIRV-Tools-lintTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/SPIRV-Tools-lint/cmake/SPIRV-Tools-lintTargets.cmake"
         "D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/source/lint/CMakeFiles/Export/f32321bbfaf6fee93929cf8d0a0816ba/SPIRV-Tools-lintTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/SPIRV-Tools-lint/cmake/SPIRV-Tools-lintTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/SPIRV-Tools-lint/cmake/SPIRV-Tools-lintTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/SPIRV-Tools-lint/cmake" TYPE FILE FILES "D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/source/lint/CMakeFiles/Export/f32321bbfaf6fee93929cf8d0a0816ba/SPIRV-Tools-lintTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/SPIRV-Tools-lint/cmake" TYPE FILE FILES "D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/source/lint/CMakeFiles/Export/f32321bbfaf6fee93929cf8d0a0816ba/SPIRV-Tools-lintTargets-debug.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/SPIRV-Tools-lint/cmake" TYPE FILE FILES "D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/source/lint/CMakeFiles/Export/f32321bbfaf6fee93929cf8d0a0816ba/SPIRV-Tools-lintTargets-minsizerel.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/SPIRV-Tools-lint/cmake" TYPE FILE FILES "D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/source/lint/CMakeFiles/Export/f32321bbfaf6fee93929cf8d0a0816ba/SPIRV-Tools-lintTargets-relwithdebinfo.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/SPIRV-Tools-lint/cmake" TYPE FILE FILES "D:/dev/vulkan-api-course/Builds/sandbox/vendor/shaderc/third_party/spirv-tools/source/lint/CMakeFiles/Export/f32321bbfaf6fee93929cf8d0a0816ba/SPIRV-Tools-lintTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/SPIRV-Tools-lint/cmake" TYPE FILE FILES "D:/dev/vulkan-api-course/Builds/SPIRV-Tools-lintConfig.cmake")
endif()

