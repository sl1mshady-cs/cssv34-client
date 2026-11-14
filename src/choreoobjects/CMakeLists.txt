# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(choreoobjects STATIC)

target_sources(choreoobjects PRIVATE
    ../game/shared/choreoactor.cpp
    ../game/shared/choreochannel.cpp
    ../game/shared/choreoevent.cpp
    ../game/shared/choreoscene.cpp
    ../game/shared/sceneimage.cpp
)


target_include_directories(choreoobjects PRIVATE
    .
    ../game/shared
    ../utils/common
    ../public/tier0
    ../public/tier1
    ../public
    ../common
)


