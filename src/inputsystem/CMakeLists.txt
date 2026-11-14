# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(inputsystem SHARED)

target_sources(inputsystem PRIVATE
    inputsystem.cpp
    joystick_sdl.cpp
    touch_sdl.cpp
    key_translation.cpp
    steamcontroller.cpp
    ../public/tier0/memoverride.cpp
)

if(WIN32)
target_sources(inputsystem PRIVATE
    novint.cpp
)

endif()

target_include_directories(inputsystem PRIVATE
    .
    ../common
    ../public
    ../public/tier0
)


target_link_libraries(inputsystem PRIVATE
    tier0
    tier1
    tier2
    vstdlib
    SDL2
    steam_api
)

if(WIN32)
target_link_libraries(inputsystem PRIVATE
    USER32
)

endif()

