# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_executable(hl2_launcher)

if(NOT ANDROID)
    target_sources(hl2_launcher PRIVATE
        main.cpp
        standby_cleanup.cpp
    )
    if(WIN32)
        target_sources(hl2_launcher PRIVATE
            launcher_main.rc
        )
    endif()
endif()
if(NOT ANDROID)
    target_include_directories(hl2_launcher PRIVATE
        ../public
    )
endif()
if(NOT ANDROID)
    if(NOT WIN32)
        target_link_libraries(hl2_launcher PRIVATE
            ${DL_LIBRARY}
            LOG
        )
    endif()
    if(WIN32)
        target_link_libraries(hl2_launcher PRIVATE
            ${USER32_LIBRARY}
            ${SHELL32_LIBRARY}
            ${ADVAPI32_LIBRARY}
        )
    endif()
endif()
