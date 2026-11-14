# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(tier0test SHARED)

target_sources(tier0test PRIVATE
    tier0test.cpp
    tslisttests.cpp
)


target_include_directories(tier0test PRIVATE
    ../../public
    ../../public/tier0
)


target_link_libraries(tier0test PRIVATE
    tier0
    tier1
    unitlib
)

if(NOT (WIN32))
target_link_libraries(tier0test PRIVATE
    DL
    LOG
)

endif()
if(NOT (NOT (WIN32)))
target_link_libraries(tier0test PRIVATE
    USER32
    SHELL32
)

endif()

