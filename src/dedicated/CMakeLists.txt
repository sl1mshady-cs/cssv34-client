# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(dedicated SHARED)

target_sources(dedicated PRIVATE
    filesystem.cpp
    ../public/filesystem_init.cpp
    ../common/netapi.cpp
    ../common/SteamAppStartup.cpp
    sys_common.cpp
    sys_ded.cpp
    console/conproc.cpp
    console/textconsole.cpp
    ../filesystem/filetracker.cpp
    ../filesystem/basefilesystem.cpp
    ../filesystem/packfile.cpp
    ../filesystem/filesystem_async.cpp
    ../filesystem/filesystem_stdio.cpp
    ../filesystem/QueuedLoader.cpp
    ../public/zip_utils.cpp
    ../public/tier0/memoverride.cpp
)

if(WIN32)
target_sources(dedicated PRIVATE
    sys_windows.cpp
    console/TextConsoleWin32.cpp
)

endif()
if(NOT (WIN32))
target_sources(dedicated PRIVATE
    sys_linux.cpp
    console/TextConsoleUnix.cpp
    ../filesystem/linux_support.cpp
)

endif()

target_include_directories(dedicated PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)

target_include_directories(dedicated PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


target_link_libraries(dedicated PRIVATE
    tier0
    vpklib
    tier1
    tier2
    tier3
    vstdlib
    steam_api
    appframework
    mathlib
    EDIT
)

if(WIN32)
target_link_libraries(dedicated PRIVATE
    USER32
    SHELL32
)

endif()

