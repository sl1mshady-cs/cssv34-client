# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(appframework STATIC)

target_sources(appframework PRIVATE
    AppSystemGroup.cpp
    ../public/filesystem_init.cpp
)

if(SDL)
    target_sources(appframework PRIVATE
        sdlmgr.cpp
    )
endif()
if(APPLE AND GL)
    target_sources(appframework PRIVATE
        glmrendererinfo_osx.mm
    )
endif()
if(WIN32)
    target_sources(appframework PRIVATE
        vguimatsysapp.cpp
        winapp.cpp
    )
endif()
if(NOT WIN32)
    target_sources(appframework PRIVATE
        posixapp.cpp
    )
endif()
target_include_directories(appframework PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ${SDL2_INCLUDE_DIRS}
)

if(APPLE)
    target_link_libraries(appframework PRIVATE
        ${FOUNDATION_LIBRARY}
        ${APPKIT_LIBRARY}
        ${IOKIT_LIBRARY}
    )
endif()
