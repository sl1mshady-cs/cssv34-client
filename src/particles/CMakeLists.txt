# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(particles STATIC)

target_sources(particles PRIVATE
    builtin_constraints.cpp
    builtin_initializers.cpp
    builtin_particle_emitters.cpp
    builtin_particle_forces.cpp
    addbuiltin_ops.cpp
    builtin_particle_ops.cpp
    builtin_particle_render_ops.cpp
    particle_sort.cpp
    particles.cpp
    psheet.cpp
)


target_include_directories(particles PRIVATE
    .
    ../public
    ../public/tier1
    ../public/tier0
)


