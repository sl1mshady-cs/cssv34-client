# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vpklib STATIC)

target_sources(vpklib PRIVATE
    packedstore.cpp
    ../common/simplebitstring.cpp
)


target_include_directories(vpklib PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)


target_link_libraries(vpklib PRIVATE
    libzstd
)


