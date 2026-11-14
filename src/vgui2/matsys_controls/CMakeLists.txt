# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(matsys_controls STATIC)

target_sources(matsys_controls PRIVATE
    assetpicker.cpp
    baseassetpicker.cpp
    colorpickerpanel.cpp
    curveeditorpanel.cpp
    gamefiletreeview.cpp
    manipulator.cpp
    matsyscontrols.cpp
    mdlpanel.cpp
    mdlpicker.cpp
    mdlsequencepicker.cpp
    picker.cpp
    potterywheelpanel.cpp
    proceduraltexturepanel.cpp
    QCGenerator.cpp
    sequencepicker.cpp
    tgapreviewpanel.cpp
    vmtpicker.cpp
    vmtpreviewpanel.cpp
    vtfpicker.cpp
    vtfpreviewpanel.cpp
    vmtpanel.cpp
)


target_include_directories(matsys_controls PRIVATE
    .
    ../../public
    ../../public/tier0
    ../../public/tier1
)


