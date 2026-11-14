# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(togl SHARED)

target_sources(togl PRIVATE
    linuxwin/dx9asmtogl2.cpp
    linuxwin/dxabstract.cpp
    linuxwin/glentrypoints.cpp
    linuxwin/glmgr.cpp
    linuxwin/glmgrbasics.cpp
    linuxwin/cglmtex.cpp
    linuxwin/cglmfbo.cpp
    linuxwin/cglmprogram.cpp
    linuxwin/cglmbuffer.cpp
    linuxwin/cglmquery.cpp
    linuxwin/asanstubs.cpp
)

if(DARWIN)
target_sources(togl PRIVATE
    linuxwin/glmgrcocoa.mm
)

endif()

target_include_directories(togl PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
)

target_include_directories(togl PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


target_link_libraries(togl PRIVATE
    tier0
    tier1
    tier2
    vstdlib
    mathlib
)

if(DARWIN)
target_link_libraries(togl PRIVATE
    OPENGL
    CARBON
)

endif()

