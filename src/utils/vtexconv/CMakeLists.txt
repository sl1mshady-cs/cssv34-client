# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_executable(vtexconv)

target_sources(vtexconv PRIVATE
    vtex_launcher.cpp
)


target_include_directories(vtexconv PRIVATE
    .
    ../../public
    ../../public/tier0
    ../../public/tier1
    ../../common
)


target_link_libraries(vtexconv PRIVATE
    tier0
    appframework
    tier1
    tier2
    tier3
    vstdlib
)


