# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vstdlib SHARED)

target_sources(vstdlib PRIVATE
    coroutine.cpp
    cvar.cpp
    jobthread.cpp
    KeyValuesSystem.cpp
    random.cpp
    vcover.cpp
    ../public/tier0/memoverride.cpp
)

if(WIN32)
target_sources(vstdlib PRIVATE
    processutils.cpp
)

if(AMD64)
target_sources(vstdlib PRIVATE
    coroutine_win64.masm
    GetStackPtr64.masm
)

endif()
endif()

target_include_directories(vstdlib PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../public/vstdlib
)


target_link_libraries(vstdlib PRIVATE
    tier0
    tier1
)

if(ANDROID)
target_link_libraries(vstdlib PRIVATE
    ANDROID_SUPPORT
)

endif()

