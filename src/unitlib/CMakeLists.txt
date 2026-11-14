# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(unitlib SHARED)

target_sources(unitlib PRIVATE
    unitlib.cpp
)


target_include_directories(unitlib PRIVATE
    .
    ../
    ../public
    ../public/tier1
    ../public/tier0
    ../common
)


