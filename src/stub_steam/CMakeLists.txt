# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(steam_api SHARED)

target_sources(steam_api PRIVATE
    steam_api.cpp
)


target_include_directories(steam_api PRIVATE
    .
    ../public
    ../public/tier0
)

target_include_directories(steam_api PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


