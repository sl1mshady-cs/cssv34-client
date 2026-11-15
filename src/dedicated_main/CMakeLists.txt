# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_executable(dedicated_launcher)

target_sources(dedicated_launcher PRIVATE
    main.cpp
)

if(WIN32)
    target_sources(dedicated_launcher PRIVATE
        dedicated_main.rc
    )
endif()
target_include_directories(dedicated_launcher PRIVATE
    ../public
    ../public/tier0
)

if(NOT WIN32)
    target_link_libraries(dedicated_launcher PRIVATE
        ${DL_LIBRARY}
    )
endif()
if(WIN32)
    target_link_libraries(dedicated_launcher PRIVATE
        ${USER32_LIBRARY}
        ${SHELL32_LIBRARY}
    )
endif()
