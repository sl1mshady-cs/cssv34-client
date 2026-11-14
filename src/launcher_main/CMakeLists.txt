# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_executable(hl2_launcher)

target_sources(hl2_launcher PRIVATE
    main.cpp
    standby_cleanup.cpp
)

if(NOT (NOT (WIN32)))
target_sources(hl2_launcher PRIVATE
    launcher_main.rc
)

endif()

target_include_directories(hl2_launcher PRIVATE
    ../public
)


if(NOT (WIN32))
target_link_libraries(hl2_launcher PRIVATE
    DL
    LOG
)

endif()
if(NOT (NOT (WIN32)))
target_link_libraries(hl2_launcher PRIVATE
    USER32
    SHELL32
    ADVAPI32
)

endif()

