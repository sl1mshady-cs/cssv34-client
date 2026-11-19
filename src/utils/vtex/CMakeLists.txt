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

target_compile_definitions(vtex_dll PRIVATE
    VTEX_DLL=1
    UTILS=1
    VTEX_DLL_EXPORTS=1
    PROTECTED_THINGS_DISABLE=1
)

target_link_libraries(vtex_dll PRIVATE
    tier0
    tier1
    bitmap
    tier2
    vstdlib
    mathlib
    vtf
    ${JPEG_LIBRARY}
    ${PNG_LIBRARY}
    ${ZLIB_LIBRARY}
)

if(WIN32)
    target_link_libraries(vtex_dll PRIVATE
        ${USER32_LIBRARY}
        ${GDI32_LIBRARY}
    )
endif()
