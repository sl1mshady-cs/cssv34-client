# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(libzstd STATIC)

target_sources(libzstd PRIVATE
    common/debug.c
    common/entropy_common.c
    common/error_private.c
    common/fse_decompress.c
    common/pool.c
    common/threading.c
    common/xxhash.c
    common/zstd_common.c
    compress/fse_compress.c
    compress/hist.c
    compress/huf_compress.c
    compress/zstdmt_compress.c
    compress/zstd_compress.c
    compress/zstd_compress_literals.c
    compress/zstd_compress_sequences.c
    compress/zstd_compress_superblock.c
    compress/zstd_double_fast.c
    compress/zstd_fast.c
    compress/zstd_lazy.c
    compress/zstd_ldm.c
    compress/zstd_opt.c
    compress/zstd_preSplit.c
    decompress/huf_decompress.c
    decompress/zstd_ddict.c
    decompress/zstd_decompress.c
    decompress/zstd_decompress_block.c
    dictBuilder/cover.c
    dictBuilder/divsufsort.c
    dictBuilder/fastcover.c
    dictBuilder/zdict.c
)

if(UNKNOWN)
target_sources(libzstd PRIVATE
    decompress/huf_decompress_amd64.S
)

endif()

target_include_directories(libzstd PRIVATE
    .
    ./common
    ./compress
    ./decompress
    ./dictBuilder
)


if(ANDROID)
target_link_libraries(libzstd PRIVATE
    ANDROID_SUPPORT
)

endif()

