# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(client SHARED)

target_sources(client PRIVATE
    in_touch.cpp
    ../../public/tier0/memoverride.cpp
)

target_include_directories(client PRIVATE
    .
    game_controls
    ../../common
    ../../public
    ../../public/tier0
    ../../public/tier1
    ../../vgui2/include
    ../../vgui2/controls
    ../shared
)

target_link_libraries(client PRIVATE
    tier0
    particles
    dmxloader
    vgui_controls
    matsys_controls
    tier1
    tier2
    tier3
    mathlib
    vstdlib
    choreoobjects
    steam_api
    bitmap
    vtf
    rmlui
    ${RT_LIBRARY}
    ${ZLIB_LIBRARY}
)

if(WIN32)
    target_link_libraries(client PRIVATE
        ${USER32_LIBRARY}
    )
endif()
