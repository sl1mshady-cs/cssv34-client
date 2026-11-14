# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(scenefilecache SHARED)

target_sources(scenefilecache PRIVATE
    SceneFileCache.cpp
    ../public/tier0/memoverride.cpp
)


target_include_directories(scenefilecache PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../game/shared
)


target_link_libraries(scenefilecache PRIVATE
    tier0
    tier1
)

if(ANDROID)
target_link_libraries(scenefilecache PRIVATE
    ANDROID_SUPPORT
)

endif()

