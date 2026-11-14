# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(tier2test SHARED)

target_sources(tier2test PRIVATE
    tier2test.cpp
)


target_include_directories(tier2test PRIVATE
    ../../public
    ../../public/tier0
)


target_link_libraries(tier2test PRIVATE
    tier0
    tier1
    tier2
    mathlib
    unitlib
)

if(NOT (WIN32))
target_link_libraries(tier2test PRIVATE
    DL
    LOG
)

endif()
if(NOT (NOT (WIN32)))
target_link_libraries(tier2test PRIVATE
    USER32
    SHELL32
)

endif()

