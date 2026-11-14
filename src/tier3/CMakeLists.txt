# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(tier3 STATIC)

target_sources(tier3 PRIVATE
    tier3.cpp
    mdlutils.cpp
    choreoutils.cpp
    scenetokenprocessor.cpp
    studiohdrstub.cpp
)


target_include_directories(tier3 PRIVATE
    .
    ../common
    ../public
    ../public/tier0
    ../public/tier1
    ../public/tier2
    ../public/tier3
)


