# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(dmxloader STATIC)

target_sources(dmxloader PRIVATE
    dmxattribute.cpp
    dmxelement.cpp
    dmxloader.cpp
    dmxloadertext.cpp
    dmxserializationdictionary.cpp
)


target_include_directories(dmxloader PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)


