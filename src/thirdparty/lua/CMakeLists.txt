# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(lua STATIC)

target_sources(lua PRIVATE
    lapi.c
    lcode.c
    lctype.c
    ldebug.c
    ldo.c
    ldump.c
    lfunc.c
    lgc.c
    llex.c
    lmem.c
    lobject.c
    lopcodes.c
    lparser.c
    lstate.c
    lstring.c
    ltable.c
    ltm.c
    lundump.c
    lvm.c
    lzio.c
    lauxlib.c
    lbaselib.c
    lbitlib.c
    lcorolib.c
    ldblib.c
    liolib.c
    lmathlib.c
    loslib.c
    lstrlib.c
    ltablib.c
    lutf8lib.c
    loadlib.c
    linit.c
)


target_include_directories(lua PRIVATE
    .
)


target_compile_definitions(lua PRIVATE
    LUA_COMPAT_5_2
)


if(ANDROID)
target_link_libraries(lua PRIVATE
    ANDROID_SUPPORT
)

endif()

