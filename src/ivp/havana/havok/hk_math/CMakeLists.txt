# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(hk_math STATIC)

target_sources(hk_math PRIVATE
    lcp/lcp_solver.cpp
    incr_lu/incr_lu.cpp
    gauss_elimination/gauss_elimination.cpp
    quaternion/quaternion.cpp
    quaternion/quaternion_util.cpp
    vector3/vector3.cpp
    vector3/vector3_util.cpp
    densematrix.cpp
    densematrix_util.cpp
    eulerangles.cpp
    math.cpp
    matrix3.cpp
    odesolve.cpp
    plane.cpp
    rotation.cpp
    spatial_matrix.cpp
    spatial_vector.cpp
    transform.cpp
)


target_include_directories(hk_math PRIVATE
    .
    ../
    ../../../ivp_utility/
)


