# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vgui_surfacelib STATIC)

target_sources(vgui_surfacelib PRIVATE
    BitmapFont.cpp
    FontAmalgam.cpp
    FontManager.cpp
    FontEffects.cpp
)

if(WIN32)
target_sources(vgui_surfacelib PRIVATE
    Win32Font.cpp
)

endif()
if(NOT (WIN32))
target_sources(vgui_surfacelib PRIVATE
    linuxfont.cpp
)

endif()

target_include_directories(vgui_surfacelib PRIVATE
    .
    ../../public
    ../../public/tier0
    ../../public/tier1
    ../../common
)


