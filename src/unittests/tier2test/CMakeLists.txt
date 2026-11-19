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

target_compile_definitions(tier2test PRIVATE
    TIER2TEST_EXPORTS=1
)

target_link_libraries(tier2test PRIVATE
    tier0
    tier1
    tier2
    mathlib
    unitlib
)

if(NOT WIN32)
    target_link_libraries(tier2test PRIVATE
        ${DL_LIBRARY}
        LOG
    )
endif()
if(WIN32)
    target_link_libraries(tier2test PRIVATE
        ${USER32_LIBRARY}
        ${SHELL32_LIBRARY}
    )
endif()
