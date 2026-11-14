# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(mathlibtest SHARED)

target_sources(mathlibtest PRIVATE
    mathlib_performance_test.cpp
    mathlib_test.cpp
)


target_include_directories(mathlibtest PRIVATE
    ../../public
    ../../public/tier0
)


target_link_libraries(mathlibtest PRIVATE
    tier0
    tier1
    tier2
    mathlib
    unitlib
)

if(NOT (WIN32))
target_link_libraries(mathlibtest PRIVATE
    DL
    LOG
)

endif()
if(NOT (NOT (WIN32)))
target_link_libraries(mathlibtest PRIVATE
    USER32
    SHELL32
)

endif()

