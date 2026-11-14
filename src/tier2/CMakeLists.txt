# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(tier2 STATIC)

target_sources(tier2 PRIVATE
    beamsegdraw.cpp
    defaultfilesystem.cpp
    dmconnect.cpp
    fileutils.cpp
    keybindings.cpp
    ../public/map_utils.cpp
    ../public/materialsystem/MaterialSystemUtil.cpp
    camerautils.cpp
    meshutils.cpp
    p4helpers.cpp
    renderutils.cpp
    riff.cpp
    soundutils.cpp
    tier2.cpp
    tier2_logging.cpp
    util_init.cpp
    utlstreambuffer.cpp
    vconfig.cpp
    keyvaluesmacros.cpp
)


target_include_directories(tier2 PRIVATE
    .
    ../common
    ../public
    ../public/tier0
    ../public/tier1
    ../public/tier2
)

target_include_directories(tier2 PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


