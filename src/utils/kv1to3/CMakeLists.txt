# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_executable(kv1to3)

if(WIN32)
    target_sources(kv1to3 PRIVATE
        kv1to3.cpp
        ../../public/tier0/memoverride.cpp
    )
endif()
if(WIN32)
    target_include_directories(kv1to3 PRIVATE
        .
        ../../public
        ../../public/tier0
        ../../public/tier1
    )
endif()
target_compile_definitions(kv1to3 PRIVATE
    DONT_PROTECT_FILEIO_FUNCTIONS=1
)

if(WIN32)
    target_link_libraries(kv1to3 PRIVATE
        tier0
        tier1
        tier2
        mathlib
        vstdlib
        kv3lib
    )
endif()
