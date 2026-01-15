cmake_minimum_required (VERSION 3.21)

project (testfirst
  DESCRIPTION "CMake project using the Microsoft GDK"
  LANGUAGES CXX
  VERSION 1.0.0.0)

option(BUILD_TEST_TEMPLATE "Ignore warnings related to TODOs" OFF)

option(ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(CompilerAndLinker.cmake)

# Add DirectXTK12 as subdirectory
# Configure DirectXTK12 options based on platform
if(VCPKG_TARGET_TRIPLET MATCHES "xbox")
    # Xbox platform - use GameInput and XAudio Win10
    set(BUILD_GAMEINPUT ON CACHE BOOL "Build for GameInput" FORCE)
    set(BUILD_WGI OFF CACHE BOOL "Build for Windows.Gaming.Input" FORCE)
    set(BUILD_XINPUT OFF CACHE BOOL "Build for XInput" FORCE)
    set(BUILD_XAUDIO_WIN10 ON CACHE BOOL "Build for XAudio 2.9" FORCE)
    set(BUILD_XAUDIO_REDIST OFF CACHE BOOL "Build for XAudio2Redist" FORCE)
else()
    # Desktop platform - use GameInput if available
    set(BUILD_GAMEINPUT ON CACHE BOOL "Build for GameInput" FORCE)
    set(BUILD_XAUDIO_WIN10 ON CACHE BOOL "Build for XAudio 2.9" FORCE)
    set(BUILD_XAUDIO_REDIST OFF CACHE BOOL "Build for XAudio2Redist" FORCE)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build DirectXTK as a shared library" FORCE)
set(BUILD_DXIL_SHADERS ON CACHE BOOL "Use DXC Shader Model 6 for shaders" FORCE)

# Find DXC tool from vcpkg and pass it to DirectXTK12
if(VCPKG_TOOLCHAIN)
    # Try to find DXC in vcpkg_installed directory
    find_program(DIRECTX_DXC_TOOL_FOR_DIRECTXTK DXC.EXE
        PATHS "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_HOST_TRIPLET}/tools/directx-dxc"
        NO_DEFAULT_PATH)
    if(DIRECTX_DXC_TOOL_FOR_DIRECTXTK)
        set(DIRECTX_DXC_PATH "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_HOST_TRIPLET}/tools/directx-dxc" CACHE PATH "Path to DirectX Shader Compiler")
        message(STATUS "Found DXC for DirectXTK12 at: ${DIRECTX_DXC_TOOL_FOR_DIRECTXTK}")
    endif()
endif()

add_subdirectory("External/DirectXTK12")

add_subdirectory("testfirst")
