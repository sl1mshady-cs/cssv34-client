# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(datamodel STATIC)

target_sources(datamodel PRIVATE
    clipboardmanager.cpp
    datamodel.cpp
    dependencygraph.cpp
    dmattribute.cpp
    dmelement.cpp
    dmelementdictionary.cpp
    dmelementfactoryhelper.cpp
    DmElementFramework.cpp
    dmserializerbinary.cpp
    dmserializerkeyvalues.cpp
    dmserializerkeyvalues2.cpp
    undomanager.cpp
)


target_include_directories(datamodel PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)


