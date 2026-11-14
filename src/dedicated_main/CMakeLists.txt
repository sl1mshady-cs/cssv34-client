# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_executable(dedicated_launcher)

target_sources(dedicated_launcher PRIVATE
    main.cpp
)

if(NOT (NOT (WIN32)))
target_sources(dedicated_launcher PRIVATE
    dedicated_main.rc
)

endif()

target_include_directories(dedicated_launcher PRIVATE
    ../public
    ../public/tier0
)


if(NOT (WIN32))
target_link_libraries(dedicated_launcher PRIVATE
    DL
)

endif()
if(NOT (NOT (WIN32)))
target_link_libraries(dedicated_launcher PRIVATE
    USER32
    SHELL32
)

endif()

