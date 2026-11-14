# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(shaderlib STATIC)

target_sources(shaderlib PRIVATE
    BaseShader.cpp
    ShaderDLL.cpp
    shaderlib_cvar.cpp
)


target_include_directories(shaderlib PRIVATE
    .
    ../../public
    ../../public/tier0
    ../../public/tier1
    ../../common
    ../
)


