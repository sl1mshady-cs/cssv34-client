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

target_compile_definitions(tier1test PRIVATE
    TIER1TEST_EXPORTS=1
)

target_link_libraries(tier1test PRIVATE
    tier0
    tier1
    mathlib
    unitlib
)

if(NOT WIN32)
    target_link_libraries(tier1test PRIVATE
        ${DL_LIBRARY}
        LOG
    )
endif()
if(WIN32)
    target_link_libraries(tier1test PRIVATE
        ${USER32_LIBRARY}
        ${SHELL32_LIBRARY}
    )
endif()
