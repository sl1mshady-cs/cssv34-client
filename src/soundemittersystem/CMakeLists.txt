# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(soundemittersystem SHARED)

target_sources(soundemittersystem PRIVATE
    ../game/shared/interval.cpp
    soundemittersystembase.cpp
    ../public/SoundParametersInternal.cpp
    ../public/tier0/memoverride.cpp
)


target_include_directories(soundemittersystem PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../game/shared
)


target_link_libraries(soundemittersystem PRIVATE
    tier0
    tier1
    vstdlib
)


