# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(havana_constraints STATIC)

target_sources(havana_constraints PRIVATE
    havok/hk_physics/constraint/constraint.cpp
    havok/hk_physics/constraint/ball_socket/ball_socket_constraint.cpp
    havok/hk_physics/constraint/limited_ball_socket/limited_ball_socket_constraint.cpp
    havok/hk_physics/constraint/ragdoll/ragdoll_constraint.cpp
    havok/hk_physics/constraint/ragdoll/ragdoll_constraint_bp_builder.cpp
    havok/hk_physics/constraint/local_constraint_system/local_constraint_system.cpp
    havok/hk_physics/constraint/hinge/hinge_bp_builder.cpp
    havok/hk_physics/constraint/hinge/hinge_constraint.cpp
    havok/hk_physics/constraint/breakable_constraint/breakable_constraint.cpp
    havok/hk_physics/constraint/fixed/fixed_constraint.cpp
    havok/hk_physics/constraint/prismatic/prismatic_constraint.cpp
    havok/hk_physics/constraint/pulley/pulley_constraint.cpp
    havok/hk_physics/constraint/stiff_spring/stiff_spring_constraint.cpp
    havok/hk_physics/core/rigid_body_core.cpp
    havok/hk_physics/effector/rigid_body_binary_effector.cpp
)


target_include_directories(havana_constraints PRIVATE
    .
    havok
    ../ivp_physics
    ../ivp_utility
    ../ivp_controller
)


