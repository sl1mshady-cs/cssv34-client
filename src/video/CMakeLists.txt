# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(video_services SHARED)

target_sources(video_services PRIVATE
    videoservices.cpp
    ../public/tier0/memoverride.cpp
)


target_include_directories(video_services PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)

target_include_directories(video_services PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


target_link_libraries(video_services PRIVATE
    tier0
    tier1
    tier2
    tier3
    vstdlib
)

if(WIN32)
target_link_libraries(video_services PRIVATE
    USER32
)

endif()
if(DARWIN)
target_link_libraries(video_services PRIVATE
    COREGRAPHICS
)

endif()

