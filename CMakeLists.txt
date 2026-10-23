cmake_minimum_required(VERSION 3.22.1)
project(raylib_forest LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT ANDROID)
    message(FATAL_ERROR "This test project builds for Android arm64.")
endif()
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.deps/raylib/CMakeLists.txt")
    message(FATAL_ERROR "Run bash scripts/download-raylib.sh first.")
endif()
set(PLATFORM Android CACHE STRING "" FORCE)
set(OPENGL_VERSION "ES 2.0" CACHE STRING "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(WITH_PIC ON CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RAUDIO OFF CACHE BOOL "" FORCE)
add_subdirectory(.deps/raylib)
add_library(main SHARED src/main.cpp src/Geometry.cpp)
target_include_directories(main PRIVATE src)
target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic -fexceptions)
target_link_libraries(main PRIVATE raylib android log EGL GLESv2 OpenSLES)
# NDK 27 defaults to 4 KiB ELF pages. Explicit 16 KiB alignment is
# required for native APK installation on devices with 16 KiB pages.
target_link_options(main PRIVATE "-Wl,--gc-sections"
    "-Wl,-u,ANativeActivity_onCreate"
    "-Wl,-z,max-page-size=16384"
    "-Wl,-z,common-page-size=16384")
