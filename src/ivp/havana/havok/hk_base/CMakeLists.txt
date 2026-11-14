# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(hk_base STATIC)

target_sources(hk_base PRIVATE
    memory/memory.cpp
    memory/memory_util.cpp
    array/array.cpp
    hash/hash.cpp
    stopwatch/stopwatch.cpp
    string/string.cpp
    id_server/id_server.cpp
    base_types.cpp
    console.cpp
)


target_include_directories(hk_base PRIVATE
    .
    ../
)


