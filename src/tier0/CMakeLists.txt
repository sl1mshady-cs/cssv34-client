# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(tier0 SHARED)

target_sources(tier0 PRIVATE
    assert_dialog.cpp
    commandline.cpp
    cpu.cpp
    cpumonitoring.cpp
    cpu_usage.cpp
    dbg.cpp
    dynfunction.cpp
    fasttimer.cpp
    logging.cpp
    mem.cpp
    mem_helpers.cpp
    memdbg.cpp
    memstd.cpp
    memvalidate.cpp
    minidump.cpp
    pch_tier0.cpp
    platform_independent.cpp
    PMELib.cpp
    progressbar.cpp
    security.cpp
    systeminformation.cpp
    stacktools.cpp
    thread.cpp
    threadtools.cpp
    tier0_strtools.cpp
    tslist.cpp
    vprof.cpp
)

if(WIN32)
target_sources(tier0 PRIVATE
    assert_dialog.rc
    platform.cpp
    pme.cpp
    vcrmode.cpp
    win32consoleio.cpp
)

if(AMD64)
target_sources(tier0 PRIVATE
    InterlockedCompareExchange128.masm
)

endif()
endif()
if(NOT (WIN32))
target_sources(tier0 PRIVATE
    cpu_posix.cpp
    platform_posix.cpp
    pme_posix.cpp
    vcrmode_posix.cpp
)

endif()

target_include_directories(tier0 PRIVATE
    .
    ../public
    ../public/tier0
)

target_include_directories(tier0 PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


if(WIN32)
target_link_libraries(tier0 PRIVATE
    ADVAPI32
    WS2_32
)

endif()
if(NOT (WIN32))
target_link_libraries(tier0 PRIVATE
    DL
    M
    LOG
)

endif()

