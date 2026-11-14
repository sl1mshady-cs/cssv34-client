# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(unicode SHARED)

target_sources(unicode PRIVATE
    unicode.cpp
    ../public/tier0/memoverride.cpp
)


target_include_directories(unicode PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
)


target_link_libraries(unicode PRIVATE
    tier0
    tier1
    tier2
    vstdlib
    mathlib
    USER32
)


