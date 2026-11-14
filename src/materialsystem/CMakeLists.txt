# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(materialsystem SHARED)

target_sources(materialsystem PRIVATE
    CColorCorrection.cpp
    cmaterial.cpp
    cmaterial_queuefriendly.cpp
    CMaterialSubRect.cpp
    cmaterialvar.cpp
    cmatnullrendercontext.cpp
    colorspace.cpp
    ctexture.cpp
    ../public/filesystem_helpers.cpp
    imagepacker.cpp
    mat_stub.cpp
    materialsystem_global.cpp
    morph.cpp
    occlusionquerymgr.cpp
    shadersystem.cpp
    texturemanager.cpp
    pch_materialsystem.cpp
    cmaterialdict.cpp
    cmaterialsystem.cpp
    cmatlightmaps.cpp
    cmatrendercontext.cpp
    cmatqueuedrendercontext.cpp
    ctexturecompositor.cpp
    ../public/tier0/memoverride.cpp
)


target_include_directories(materialsystem PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
    ../
)

target_include_directories(materialsystem PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


target_link_libraries(materialsystem PRIVATE
    tier0
    tier1
    tier2
    vstdlib
    mathlib
    bitmap
    shaderlib
    vtf
)


