# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vguimatsurface SHARED)

target_sources(vguimatsurface PRIVATE
    Clip2D.cpp
    Cursor.cpp
    ../public/filesystem_helpers.cpp
    FontTextureCache.cpp
    Input.cpp
    MatSystemSurface.cpp
    asanstubs.cpp
    TextureDictionary.cpp
    ../vgui2/src/vgui_key_translation.cpp
    ../public/vgui_controls/vgui_controls.cpp
    ../public/tier0/memoverride.cpp
)

if(WIN32)
target_sources(vguimatsurface PRIVATE
    memorybitmap.cpp
)

endif()

target_include_directories(vguimatsurface PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)


target_link_libraries(vguimatsurface PRIVATE
    bitmap
    mathlib
    tier0
    vgui_controls
    tier1
    vstdlib
    tier2
    tier3
    vgui_surfacelib
    FT2
    FC
    SDL2
)

if(ANDROID)
target_link_libraries(vguimatsurface PRIVATE
    EXPAT
)

endif()
if(WIN32)
target_link_libraries(vguimatsurface PRIVATE
    USER32
    GDI32
)

endif()

