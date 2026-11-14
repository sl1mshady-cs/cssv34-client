# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vgui2 SHARED)

target_sources(vgui2 PRIVATE
    Bitmap.cpp
    Border.cpp
    ScalableImageBorder.cpp
    ImageBorder.cpp
    fileimage.cpp
    ../../public/filesystem_helpers.cpp
    ../../public/filesystem_init.cpp
    InputWin32.cpp
    LocalizedStringTable.cpp
    MemoryBitmap.cpp
    MessageListener.cpp
    Scheme.cpp
    ../../public/UnicodeFileHelpers.cpp
    vgui.cpp
    vgui_internal.cpp
    vgui_key_translation.cpp
    VPanel.cpp
    VPanelWrapper.cpp
    keyrepeat.cpp
    ../../public/tier0/memoverride.cpp
)

if(WIN32)
target_sources(vgui2 PRIVATE
    Surface.cpp
    System.cpp
)

endif()
if(NOT (WIN32))
target_sources(vgui2 PRIVATE
    system_posix.cpp
)

endif()

target_include_directories(vgui2 PRIVATE
    .
    ../../public
    ../../public/tier0
    ../../public/tier1
    ../../common
)

target_include_directories(vgui2 PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


target_link_libraries(vgui2 PRIVATE
    tier0
    tier1
    tier2
    tier3
    vstdlib
    SDL2
)

if(WIN32)
target_link_libraries(vgui2 PRIVATE
    USER32
    IMM32
    GDI32
    SHELL32
    OLE32
    SHLWAPI
    vgui_surfacelib
    WINMM
)

endif()
if(DARWIN)
target_link_libraries(vgui2 PRIVATE
    APPLICATIONSERVICES
)

endif()

