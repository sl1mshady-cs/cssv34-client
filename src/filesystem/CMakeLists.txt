# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(filesystem_stdio SHARED)

target_sources(filesystem_stdio PRIVATE
    basefilesystem.cpp
    packfile.cpp
    filetracker.cpp
    filesystem_async.cpp
    filesystem_stdio.cpp
    ../public/kevvaluescompiler.cpp
    ../public/zip_utils.cpp
    QueuedLoader.cpp
    ../public/tier0/memoverride.cpp
)

if(NOT (WIN32))
target_sources(filesystem_stdio PRIVATE
    linux_support.cpp
)

endif()

target_include_directories(filesystem_stdio PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)


target_link_libraries(filesystem_stdio PRIVATE
    tier0
    vpklib
    tier1
    tier2
    vstdlib
)

if(WIN32)
target_link_libraries(filesystem_stdio PRIVATE
    SHELL32
)

endif()

