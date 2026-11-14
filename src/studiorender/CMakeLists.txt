# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(studiorender SHARED)

target_sources(studiorender PRIVATE
    studiorender.cpp
    studiorendercontext.cpp
    flexrenderdata.cpp
    r_studio.cpp
    r_studiodecal.cpp
    r_studiodraw.cpp
    r_studiodraw_computeflexedvertex.cpp
    r_studioflex.cpp
    r_studiogettriangles.cpp
    r_studiolight.cpp
    r_studiostats.cpp
    ../public/tier0/memoverride.cpp
)


target_include_directories(studiorender PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
)


target_link_libraries(studiorender PRIVATE
    tier0
    tier1
    tier2
    tier3
    vstdlib
    mathlib
    bitmap
)


