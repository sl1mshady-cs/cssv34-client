# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vaudio_opus SHARED)

target_sources(vaudio_opus PRIVATE
    voiceencoder_opus.cpp
    ../frame_encoder/voice_codec_frame.cpp
    ../../../tier1/interface.cpp
    ../../../public/tier0/memoverride.cpp
)

target_include_directories(vaudio_opus PRIVATE
    .
    ../frame_encoder
    ../../../public
    ../../../public/tier1
    ../../../public/tier0
    ../../
    ../../../common
    ../../../public/engine/audio
    celt
)

target_compile_definitions(vaudio_opus PRIVATE
    OPUS_EXPORTS=1
)

target_link_libraries(vaudio_opus PRIVATE
    tier0
    tier1
    vstdlib
    ${OPUS_LIBRARY}
)

