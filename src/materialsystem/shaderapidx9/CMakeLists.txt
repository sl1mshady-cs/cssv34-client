# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(shaderapidx9 SHARED)

target_sources(shaderapidx9 PRIVATE
    colorformatdx8.cpp
    ../../public/filesystem_helpers.cpp
    hardwareconfig.cpp
    meshbase.cpp
    meshdx8.cpp
    shaderapidx8.cpp
    shaderdevicebase.cpp
    shaderapibase.cpp
    shaderdevicedx8.cpp
    shadershadowdx8.cpp
    texturedx8.cpp
    TransitionTable.cpp
    cvballoctracker.cpp
    vertexdecl.cpp
    vertexshaderdx8.cpp
    ../../public/tier0/memoverride.cpp
)

if((WIN32) AND (NOT (GL)))
target_sources(shaderapidx9 PRIVATE
    recording.cpp
    wmi.cpp
)

endif()
if(NOT (WIN32))
target_sources(shaderapidx9 PRIVATE
    winutils.cpp
)

endif()

target_include_directories(shaderapidx9 PRIVATE
    .
    ../../public
    ../../public/tier0
    ../../public/tier1
    ../../common
    ../
)

target_include_directories(shaderapidx9 PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


target_link_libraries(shaderapidx9 PRIVATE
    tier0
    tier1
    tier2
    vstdlib
    togl
    bitmap
    mathlib
)

if(ANDROID)
target_link_libraries(shaderapidx9 PRIVATE
    ANDROID_SUPPORT
)

endif()
if((WIN32) AND (NOT (GL)))
target_link_libraries(shaderapidx9 PRIVATE
    D3D9
    D3DX9
    bzip2
    LEGACY_STDIO_DEFINITIONS
    GDI32
)

endif()

