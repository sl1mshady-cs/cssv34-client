# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vaudio_speex SHARED)

target_sources(vaudio_speex PRIVATE
    VoiceEncoder_Speex.cpp
    ../frame_encoder/voice_codec_frame.cpp
    ../../../tier1/interface.cpp
    ../../../public/tier0/memoverride.cpp
)


target_include_directories(vaudio_speex PRIVATE
    .
    ../frame_encoder
    ../../../public
    ../../../public/engine/audio
    ../../../public/tier1
    ../../../public/tier0
    ../../
    ../../../common
    ../../audio
)


target_link_libraries(vaudio_speex PRIVATE
    tier0
    tier1
    vstdlib
    SPEEX
)


