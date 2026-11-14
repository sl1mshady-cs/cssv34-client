# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vaudio_celt SHARED)

target_sources(vaudio_celt PRIVATE
    voiceencoder_celt.cpp
    ../frame_encoder/voice_codec_frame.cpp
    ../../../tier1/interface.cpp
    ../../../public/tier0/memoverride.cpp
)


target_include_directories(vaudio_celt PRIVATE
    .
    ../frame_encoder
    ../../../public
    ../../../public/tier1
    ../../../public/tier0
    ../../
    ../../../common
    ../../audio/public
)


target_link_libraries(vaudio_celt PRIVATE
    tier0
    tier1
    vstdlib
    CELT
)


