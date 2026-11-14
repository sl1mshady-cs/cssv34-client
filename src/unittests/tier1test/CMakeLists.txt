# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(tier1test SHARED)

target_sources(tier1test PRIVATE
    commandbuffertest.cpp
    utlstringtest.cpp
    tier1test.cpp
    lzsstest.cpp
)


target_include_directories(tier1test PRIVATE
    ../../public
    ../../public/tier0
)


target_link_libraries(tier1test PRIVATE
    tier0
    tier1
    mathlib
    unitlib
)

if(NOT (WIN32))
target_link_libraries(tier1test PRIVATE
    DL
    LOG
)

endif()
if(NOT (NOT (WIN32)))
target_link_libraries(tier1test PRIVATE
    USER32
    SHELL32
)

endif()

