# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(kv3lib STATIC)

target_sources(kv3lib PRIVATE
    keyvalues3.cpp
    keyvalues3_oldschematext.cpp
    keyvalues3_textreader.cpp
    keyvalues3_textwriter.cpp
    keyvalues3_binaryreader.cpp
    keyvalues3_binarywriter.cpp
    keyvalues3_kv1textreader.cpp
    keyvalues3_kv1textwriter.cpp
    kv3transfer.cpp
    kv3schemahelpers.cpp
    kv3format_manager.cpp
    kv3formats.cpp
    blockcompress.cpp
    utltokenizer.cpp
)


target_include_directories(kv3lib PRIVATE
    .
    ../
    ../public
    ../public/tier1
    ../public/tier0
    ../common
)


