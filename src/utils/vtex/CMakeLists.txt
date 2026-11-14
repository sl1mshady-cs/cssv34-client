# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vtex_dll SHARED)

target_sources(vtex_dll PRIVATE
    ../common/cmdlib.cpp
    ../../public/filesystem_helpers.cpp
    ../../public/filesystem_init.cpp
    ../common/filesystem_tools.cpp
    vtex.cpp
    ../../common/imageutils.cpp
)


target_include_directories(vtex_dll PRIVATE
    .
    ../../public
    ../../public/tier0
    ../../public/tier1
    ../common
    ../../common
    ../../thirdparty
)


target_link_libraries(vtex_dll PRIVATE
    tier0
    tier1
    bitmap
    tier2
    vstdlib
    mathlib
    vtf
    JPEG
    PNG
    ZLIB
)

if(WIN32)
target_link_libraries(vtex_dll PRIVATE
    USER32
    GDI32
)

endif()

