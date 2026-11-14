# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(shaderapiempty SHARED)

target_sources(shaderapiempty PRIVATE
    shaderapiempty.cpp
    ../../public/tier0/memoverride.cpp
)


target_include_directories(shaderapiempty PRIVATE
    .
    ../../public
    ../../public/tier0
    ../../public/tier1
    ../../common
    ../
)

target_include_directories(shaderapiempty PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


target_link_libraries(shaderapiempty PRIVATE
    tier0
    tier1
)


