# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(bzip2 STATIC)

target_sources(bzip2 PRIVATE
    blocksort.c
    bzip2.c
    bzlib.c
    compress.c
    crctable.c
    decompress.c
    huffman.c
    randtable.c
)


target_include_directories(bzip2 PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)


