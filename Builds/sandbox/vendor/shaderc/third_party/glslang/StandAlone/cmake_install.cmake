# Install script for directory: D:/dev/vulkan-api-course/sandbox/vendor/shaderc/third_party/glslang/StandAlone

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
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "D:/dev/vulkan-api-course/Builds/bin/Debug/glslang.exe")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "D:/dev/vulkan-api-course/Builds/bin/Release/glslang.exe")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "D:/dev/vulkan-api-course/Builds/bin/MinSizeRel/glslang.exe")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "D:/dev/vulkan-api-course/Builds/bin/RelWithDebInfo/glslang.exe")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
            message(STATUS "Installing (copy_if_different): glslang.exe -> $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different [=[glslang.exe]=] [=[glslangValidator.exe]=]
            WORKING_DIRECTORY "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin"
        )
    
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
            message(STATUS "Installing (copy_if_different): glslang.exe -> $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different [=[glslang.exe]=] [=[glslangValidator.exe]=]
            WORKING_DIRECTORY "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin"
        )
    
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
            message(STATUS "Installing (copy_if_different): glslang.exe -> $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different [=[glslang.exe]=] [=[glslangValidator.exe]=]
            WORKING_DIRECTORY "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin"
        )
    
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
            message(STATUS "Installing (copy_if_different): glslang.exe -> $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different [=[glslang.exe]=] [=[glslangValidator.exe]=]
            WORKING_DIRECTORY "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin"
        )
    
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "D:/dev/vulkan-api-course/Builds/bin/Debug/spirv-remap.exe")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "D:/dev/vulkan-api-course/Builds/bin/Release/spirv-remap.exe")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "D:/dev/vulkan-api-course/Builds/bin/MinSizeRel/spirv-remap.exe")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "D:/dev/vulkan-api-course/Builds/bin/RelWithDebInfo/spirv-remap.exe")
  endif()
endif()

