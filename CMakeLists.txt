cmake_minimum_required(VERSION 3.16)

project(engine LANGUAGES CXX)

# Set standard C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable experimental module support for GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fmodules-ts)
endif()

# Set up dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})

# Compile options
option(BUILD_DEBUG "Build in Debug mode" ON)
option(BUILD_RELEASE "Build in Release mode" OFF)

# Multi-processor compilation for MSVC
if(MSVC)
    add_compile_options(/MP)
    # Enable option to preserve folder structure in Visual Studio
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# Set Vulkan SDK
if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "Set VULKAN_SDK variable")
else()
    set(VULKAN_SDK $ENV{VULKAN_SDK})
    message(STATUS "Vulkan path is: ${VULKAN_SDK}")
endif()

# Set includes
set(GLFW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/GLFW/include")
set(GLM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/glm")
set(VULKAN_INCLUDE_DIR "${VULKAN_SDK}/Include")
set(SHADERC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/shaderc/libshaderc/include")
set(STB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/stb")
set(IMGUI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/imgui-docking")
set(ABSEIL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/abseil")

# Set GLFW
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/GLFW" EXCLUDE_FROM_ALL)
set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "Use DLL runtime (/MD)" FORCE)

# Shaderc configuration
set(SHADERC_SKIP_TESTS ON CACHE BOOL "Skip building tests for shaderc")
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "Skip building examples for shaderc" FORCE)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "Skip copyright check for shaderc" FORCE)
# Turn off precompiled headers shaderc
set(SHADERC_ENABLE_PCH OFF CACHE BOOL "Disable PCH for shaderc" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/shaderc" EXCLUDE_FROM_ALL)

# Abseil configuration
set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "")
set(ABSL_BUILD_TESTING OFF CACHE BOOL "")
set(ABSL_USE_EXTERNAL_GOOGLETEST OFF CACHE BOOL "")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/abseil" EXCLUDE_FROM_ALL)

# Main project
add_executable(engine
        engine/src/application/managers/virtual_fs.ixx)

# Find source files
file(GLOB_RECURSE engine_SOURCES "engine/src/**.cpp")
file(GLOB_RECURSE engine_MODULES "engine/src/**.ixx")
file(GLOB_RECURSE engine_HEADERS "engine/src/**.h")
# Find ImGui source files
file(GLOB imgui_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/imgui-docking/*.cpp")
file(GLOB imgui_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/engine/third_parties/imgui-docking/*.h")

# File structure setup for Visual Studio
# Function to set file structure as on disk
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if(IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

# Apply function to our files
assign_source_group(${engine_SOURCES} ${engine_MODULES} ${engine_HEADERS} ${imgui_SOURCES} ${imgui_HEADERS})

# Source files
target_sources(engine PRIVATE ${engine_SOURCES} ${engine_MODULES} ${engine_HEADERS} ${imgui_SOURCES} ${imgui_HEADERS})

# Include files
target_include_directories(engine PRIVATE
        "engine/src"
        ${GLFW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${VULKAN_INCLUDE_DIR}
        ${SHADERC_INCLUDE_DIR}
        ${STB_INCLUDE_DIR}
        ${IMGUI_INCLUDE_DIR}
        ${ABSEIL_INCLUDE_DIR}
)

# Vulkan path
set(VULKAN_LIB_DIR "${VULKAN_SDK}/Lib")
target_link_directories(engine PRIVATE ${VULKAN_LIB_DIR})

# Add libs
target_link_libraries(engine PRIVATE
        glfw
        vulkan-1
        shaderc
        absl::flat_hash_map
        absl::hash
        absl::strings
        absl::time
        absl::base
        absl::flags
)

# Win defines
if(WIN32)
    target_compile_definitions(engine PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Lib config
set_target_properties(engine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

# Debug config
target_compile_options(engine PRIVATE
        $<$<CONFIG:Debug>:
        $<$<CXX_COMPILER_ID:MSVC>:/Zi /Od /MDd>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g -O0>
        >
)

# Release config
target_compile_options(engine PRIVATE
        $<$<CONFIG:Release>:
        $<$<CXX_COMPILER_ID:MSVC>:/Zi /O2 /MD>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g -O2>
        >
)

# Set starting project
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT engine)
endif()