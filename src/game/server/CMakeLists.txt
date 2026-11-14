# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(server SHARED)

target_sources(server PRIVATE
    ../../public/tier0/memoverride.cpp
)


target_include_directories(server PRIVATE
    .
    ../shared
    ../../utils/common
    ../shared/econ
    NextBot
    ../../common
    ../../public/tier0
    ../../public/tier1
    ../../public
)


target_link_libraries(server PRIVATE
    tier0
    particles
    dmxloader
    tier1
    tier2
    tier3
    mathlib
    vstdlib
    choreoobjects
    steam_api
)

if(WIN32)
target_link_libraries(server PRIVATE
    USER32
)

endif()

