# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(vphysics SHARED)

target_sources(vphysics PRIVATE
    convert.cpp
    ../public/filesystem_helpers.cpp
    ledgewriter.cpp
    main.cpp
    physics_airboat.cpp
    physics_collide.cpp
    physics_constraint.cpp
    physics_controller_raycast_vehicle.cpp
    physics_environment.cpp
    physics_fluid.cpp
    physics_friction.cpp
    physics_material.cpp
    physics_motioncontroller.cpp
    physics_object.cpp
    physics_shadow.cpp
    physics_spring.cpp
    physics_vehicle.cpp
    physics_virtualmesh.cpp
    trace.cpp
    vcollide_parse.cpp
    vphysics_saverestore.cpp
    ../public/tier0/memoverride.cpp
)


target_include_directories(vphysics PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../ivp/ivp_intern
    ../ivp/ivp_collision
    ../ivp/ivp_physics
    ../ivp/ivp_surface_manager
    ../ivp/ivp_utility
    ../ivp/ivp_controller
    ../ivp/ivp_compact_builder
    ../ivp/havana/havok
    ../ivp/havana
)


target_link_libraries(vphysics PRIVATE
    tier0
    havana_constraints
    hk_math
    hk_base
    ivp_compactbuilder
    ivp_physics
    tier1
    tier2
    vstdlib
    mathlib
)


