# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(mathlib STATIC)

target_sources(mathlib PRIVATE
    color_conversion.cpp
    halton.cpp
    lightdesc.cpp
    mathlib_base.cpp
    powsse.cpp
    sparse_convolution_noise.cpp
    sseconst.cpp
    sse.cpp
    ssenoise.cpp
    3dnow.cpp
    anorms.cpp
    bumpvects.cpp
    IceKey.cpp
    imagequant.cpp
    polyhedron.cpp
    quantize.cpp
    randsse.cpp
    spherical.cpp
    simdvectormatrix.cpp
    vector.cpp
    vmatrix.cpp
    almostequal.cpp
)


target_include_directories(mathlib PRIVATE
    .
    ../common
    ../public
    ../public/tier0
    ../public/mathlib
)


