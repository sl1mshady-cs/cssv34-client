# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(plutovg STATIC)

target_sources(plutovg PRIVATE
    plutovg-blend.c
    plutovg-canvas.c
    plutovg-font.c
    plutovg-ft-math.c
    plutovg-ft-raster.c
    plutovg-ft-stroker.c
    plutovg-matrix.c
    plutovg-paint.c
    plutovg-path.c
    plutovg-rasterize.c
    plutovg-surface.c
)


target_include_directories(plutovg PRIVATE
    ../include
)


if(ANDROID)
target_link_libraries(plutovg PRIVATE
    ANDROID_SUPPORT
)

endif()

