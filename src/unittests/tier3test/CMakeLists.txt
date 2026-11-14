# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(tier3test SHARED)

target_sources(tier3test PRIVATE
    tier3test.cpp
)


target_include_directories(tier3test PRIVATE
    ../../public
    ../../public/tier0
)


target_link_libraries(tier3test PRIVATE
    tier0
    tier1
    tier2
    tier3
    mathlib
    unitlib
)

if(NOT (WIN32))
target_link_libraries(tier3test PRIVATE
    DL
    LOG
)

endif()
if(NOT (NOT (WIN32)))
target_link_libraries(tier3test PRIVATE
    USER32
    SHELL32
)

endif()

