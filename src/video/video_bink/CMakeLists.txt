# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(video_bink SHARED)

target_sources(video_bink PRIVATE
    bink_material.cpp
    bink_video.cpp
    yuv_rgb.c
    ../../public/tier0/memoverride.cpp
)


target_include_directories(video_bink PRIVATE
    .
    ../../public
    ../../public/tier0
    ../../public/tier1
    ../../common
    ../
)

target_include_directories(video_bink PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


target_link_libraries(video_bink PRIVATE
    tier0
    tier1
    tier2
    tier3
    vstdlib
    AVCODEC
    AVUTIL
    AVFORMAT
)

if(WIN32)
target_link_libraries(video_bink PRIVATE
    USER32
)

endif()

