cmake_minimum_required(VERSION 3.20)
project(ombro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(vis_ombro SHARED
    src/plugin.cpp
    src/renderer.cpp
    src/visualization.cpp
    ombro.def
)

target_include_directories(vis_ombro PRIVATE src)

target_link_libraries(vis_ombro PRIVATE
    d3d11
    dxgi
    d3dcompiler
    user32
    gdi32
    shlwapi
)

set_target_properties(vis_ombro PROPERTIES
    OUTPUT_NAME "vis_ombro"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Per-configuration output directories
foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set_target_properties(vis_ombro PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()
