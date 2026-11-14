# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_executable(unittest)

target_sources(unittest PRIVATE
    unittest.cpp
)


target_include_directories(unittest PRIVATE
    ../../public
)


target_link_libraries(unittest PRIVATE
    tier0
    appframework
    tier1
    tier2
    tier3
    vstdlib
    unitlib
)

if(NOT (WIN32))
target_link_libraries(unittest PRIVATE
    DL
    LOG
)

endif()
if(NOT (NOT (WIN32)))
target_link_libraries(unittest PRIVATE
    USER32
    SHELL32
)

endif()

