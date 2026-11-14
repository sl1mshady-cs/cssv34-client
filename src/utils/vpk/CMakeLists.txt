# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_executable(vpk)

target_sources(vpk PRIVATE
    packtest.cpp
    ../../public/tier0/memoverride.cpp
)


target_include_directories(vpk PRIVATE
    .
    ../../public
    ../../public/vpklib
    ../../public/tier0
    ../../public/tier1
    ../../common
)


target_link_libraries(vpk PRIVATE
    tier0
    vpklib
    tier1
    tier2
    mathlib
    vstdlib
)

if(WIN32)
target_link_libraries(vpk PRIVATE
    SHELL32
)

endif()

