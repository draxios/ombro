cmake_minimum_required(VERSION 3.20)
project(ombro LANGUAGES CXX)

# Windows-only project (Win32 DLL, Direct3D 11)
if(NOT WIN32)
    message(FATAL_ERROR "Ombro requires Windows (Win32 DLL + Direct3D 11)")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(vis_ombro SHARED
    src/plugin.cpp
    src/renderer.cpp
    src/visualization.cpp
    ombro.def
)

target_include_directories(vis_ombro PRIVATE src)

# Compiler warning flags for MSVC
if(MSVC)
    target_compile_options(vis_ombro PRIVATE
        /W4         # High warning level
        /sdl        # Security Development Lifecycle checks
        /guard:cf   # Control Flow Guard
        /wd4100     # Unreferenced formal parameter (common in callbacks)
    )
endif()

target_link_libraries(vis_ombro PRIVATE
    d3d11
    dxgi
    d3dcompiler
    user32
    gdi32
    shlwapi
)

# Separate output directories per architecture to avoid x86/x64 collisions
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OMBRO_ARCH "x64")
else()
    set(OMBRO_ARCH "x86")
endif()

set_target_properties(vis_ombro PROPERTIES
    OUTPUT_NAME "vis_ombro"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OMBRO_ARCH}"
)

# Per-configuration output directories
foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set_target_properties(vis_ombro PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/bin/${OMBRO_ARCH}"
    )
endforeach()
