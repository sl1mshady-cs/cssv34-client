# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(launcher SHARED)

target_sources(launcher PRIVATE
    ../public/filesystem_init.cpp
    launcher.cpp
    reslistgenerator.cpp
    ../public/tier0/memoverride.cpp
)

if(ANDROID)
    target_sources(launcher PRIVATE
        android/main.cpp
        android/crashhandler.cpp
    )
endif()
target_include_directories(launcher PRIVATE
    .
    android/libunwind
    ../public
    ../public/tier0
    ../public/tier1
    ../common
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(launcher PRIVATE
    tier0
    tier1
    tier2
    tier3
    vstdlib
    steam_api
    appframework
    ${SDL2_LIBRARY}
    togl
    ${UNWIND_LIBRARY}
)

if(WIN32)
    target_link_libraries(launcher PRIVATE
        ${USER32_LIBRARY}
        ${OLE32_LIBRARY}
        ${SHELL32_LIBRARY}
    )
endif()
