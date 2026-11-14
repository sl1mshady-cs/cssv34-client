# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vaudio_minimp3 SHARED)

target_sources(vaudio_minimp3 PRIVATE
    mp3codecs.cpp
    ../../../public/tier0/memoverride.cpp
)


target_include_directories(vaudio_minimp3 PRIVATE
    .
    ../../../public/tier1
    ../../../public/
    ../../../thirdparty/minimp3
)


target_link_libraries(vaudio_minimp3 PRIVATE
    tier0
    tier1
    vstdlib
)


