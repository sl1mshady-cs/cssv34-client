# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(uisystem SHARED)

target_sources(uisystem PRIVATE
    uidocument.cpp
    uifileinterface.cpp
    uihelpers.cpp
    uirenderinterface.cpp
    uisystem.cpp
    mainmenu.cpp
    uiimage.cpp
    stb_image_Impl.cpp
    uisysteminterface.cpp
)


target_include_directories(uisystem PRIVATE
    .
    ../thirdparty
    ../public
    ../public/tier0
    ../public/tier1
    ../game/shared
)


target_link_libraries(uisystem PRIVATE
    tier0
    tier1
    tier2
    vstdlib
    vtf
    bitmap
    mathlib
    rmlui
    FT2
    LEGACY_STDIO_DEFINITIONS
)

if(ANDROID)
target_link_libraries(uisystem PRIVATE
    ANDROID_SUPPORT
)

endif()

