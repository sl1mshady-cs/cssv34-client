# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vtf STATIC)

target_sources(vtf PRIVATE
    convert_x360.cpp
    vtf.cpp
)

if(WIN32)
target_sources(vtf PRIVATE
    s3tc_decode.cpp
)

endif()

target_include_directories(vtf PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)


