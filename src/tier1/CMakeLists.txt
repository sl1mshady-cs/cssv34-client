# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(tier1 STATIC)

target_sources(tier1 PRIVATE
    ../utils/lzma/C/LzmaDec.c
    bitbuf.cpp
    byteswap.cpp
    characterset.cpp
    checksum_crc.cpp
    checksum_md5.cpp
    checksum_sha1.cpp
    commandbuffer.cpp
    convar.cpp
    datamanager.cpp
    diff.cpp
    floatprint.cpp
    generichash.cpp
    ilocalize.cpp
    interface.cpp
    KeyValues.cpp
    keyvaluesjson.cpp
    kvpacker.cpp
    lzmaDecoder.cpp
    lzss.cpp
    mempool.cpp
    memstack.cpp
    NetAdr.cpp
    newbitbuf.cpp
    rangecheckedvar.cpp
    reliabletimer.cpp
    snappy-sinksource.cpp
    snappy-stubs-internal.cpp
    snappy.cpp
    sparsematrix.cpp
    splitstring.cpp
    stringpool.cpp
    strtools.cpp
    strtools_unicode.cpp
    tier1.cpp
    tier1_logging.cpp
    tokenreader.cpp
    uniqueid.cpp
    utlbinaryblock.cpp
    utlbuffer.cpp
    utlbufferutil.cpp
    utlstring.cpp
    utlsymbol.cpp
)

if(WIN32)
target_sources(tier1 PRIVATE
    processor_detect.cpp
)

endif()
if(NOT (WIN32))
target_sources(tier1 PRIVATE
    processor_detect_linux.cpp
    qsort_s.cpp
)

endif()

target_include_directories(tier1 PRIVATE
    .
    ../
    ../public
    ../public/tier1
    ../public/tier0
    ../common
)


if(WIN32)
target_link_libraries(tier1 PRIVATE
    RPCRT4
)

endif()
if(DARWIN)
target_link_libraries(tier1 PRIVATE
    ICONV
)

endif()

