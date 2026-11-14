# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(datacache SHARED)

target_sources(datacache PRIVATE
    datacache.cpp
    mdlcache.cpp
    ../public/studio.cpp
    ../public/studio_virtualmodel.cpp
    ../common/studiobyteswap.cpp
    ../public/tier0/memoverride.cpp
)


target_include_directories(datacache PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)


target_link_libraries(datacache PRIVATE
    tier0
    tier1
    tier2
    tier3
)

if(ANDROID)
target_link_libraries(datacache PRIVATE
    ANDROID_SUPPORT
)

endif()

