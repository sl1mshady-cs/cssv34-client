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

target_compile_definitions(vguimatsurface PRIVATE
    VGUIMATSURFACE_DLL_EXPORT=1
    GAMEUI_EXPORTS=1
    DONT_PROTECT_FILEIO_FUNCTIONS=1
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
    ${FT2_LIBRARY}
    ${FC_LIBRARY}
    ${SDL2_LIBRARY}
)

if(ANDROID)
    target_link_libraries(vguimatsurface PRIVATE
        EXPAT
    )
endif()
if(NOT ANDROID)
    if(WIN32)
        target_link_libraries(vguimatsurface PRIVATE
            ${USER32_LIBRARY}
            ${GDI32_LIBRARY}
        )
    endif()
endif()
