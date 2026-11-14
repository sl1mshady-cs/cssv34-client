# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(bitmap STATIC)

target_sources(bitmap PRIVATE
    ImageByteSwap.cpp
    colorconversion.cpp
    float_bm.cpp
    float_bm2.cpp
    float_bm3.cpp
    float_bm_bilateral_filter.cpp
    float_cube.cpp
    imageformat.cpp
    psd.cpp
    resample.cpp
    tgaloader.cpp
    tgawriter.cpp
    bitmap.cpp
)

if(WIN32)
target_sources(bitmap PRIVATE
    float_bm4.cpp
)

endif()

target_include_directories(bitmap PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../thirdparty/stb
)


