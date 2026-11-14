# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(stdshader_dx9 SHARED)

target_sources(stdshader_dx9 PRIVATE
    aftershock.cpp
    aftershock_helper.cpp
    AccumBuff4Sample.cpp
    accumbuff5sample.cpp
    BaseVSShader.cpp
    bik_dx90.cpp
    Bloom.cpp
    BlurFilterX.cpp
    BlurFilterY.cpp
    BufferClearObeyStencil_dx9.cpp
    cable_dx9.cpp
    cloak.cpp
    cloak_blended_pass_helper.cpp
    cloak_dx9_helper.cpp
    cloud_dx9.cpp
    colorcorrection.cpp
    compositor.cpp
    core_dx9.cpp
    color_projection.cpp
    debugdepth.cpp
    debugluxel.cpp
    debugnormalmap.cpp
    DebugDrawEnvmapMask.cpp
    debugmrttexture.cpp
    DebugTextureView.cpp
    DecalBaseTimesLightmapAlphaBlendSelfIllum_dx9.cpp
    DecalModulate_dx9.cpp
    depthwrite.cpp
    Downsample.cpp
    downsample_nohdr.cpp
    Engine_Post_dx9.cpp
    emissive_scroll_blended_pass_helper.cpp
    eye_refract.cpp
    eye_refract_helper.cpp
    eyes_dx8_dx9_helper.cpp
    eyes_dx9.cpp
    eyeglint_dx9.cpp
    fillrate.cpp
    filmdust_dx8_dx9.cpp
    filmgrain_dx8_dx9.cpp
    flesh_interior_blended_pass_helper.cpp
    floatcombine.cpp
    floatcombine_autoexpose.cpp
    floattoscreen.cpp
    floattoscreen_vanilla.cpp
    gooinglass.cpp
    HDRCombineTo16Bit.cpp
    HDRSelectRange.cpp
    hsl_filmgrain_pass1.cpp
    hsl_filmgrain_pass2.cpp
    hsv.cpp
    introscreenspaceeffect.cpp
    lightmappedgeneric_dx9.cpp
    lightmappedgeneric_dx9_helper.cpp
    lightmappedreflective.cpp
    modulate_dx9.cpp
    MonitorScreen_dx9.cpp
    motion_blur_dx9.cpp
    occlusion_dx9.cpp
    particlelitgeneric_dx9.cpp
    particlelitgeneric_dx9_helper.cpp
    particlesphere_dx9.cpp
    pbr_dx9.cpp
    portal.cpp
    portalstaticoverlay.cpp
    portal_refract.cpp
    portal_refract_helper.cpp
    pyro_vision.cpp
    refract.cpp
    refract_dx9_helper.cpp
    sample4x4.cpp
    sample4x4_blend.cpp
    screenspace_general.cpp
    sfm_blurfilterx.cpp
    sfm_blurfiltery.cpp
    sfm_downsample.cpp
    sfm_integercombine.cpp
    shadow.cpp
    shadowbuild_dx9.cpp
    shadowmodel_dx9.cpp
    shatteredglass.cpp
    showz.cpp
    skin_dx9_helper.cpp
    sky_dx9.cpp
    sky_hdr_dx9.cpp
    sprite_dx9.cpp
    spritecard.cpp
    teeth.cpp
    TreeLeaf.cpp
    unlitgeneric_dx9.cpp
    unlittwotexture_dx9.cpp
    vertexlitgeneric_dx9.cpp
    vertexlitgeneric_dx9_helper.cpp
    volume_clouds.cpp
    volume_clouds_helper.cpp
    vortwarp_dx9.cpp
    vr_distort_hud.cpp
    vr_distort_texture.cpp
    warp.cpp
    water.cpp
    weapon_sheen_pass_helper.cpp
    windowimposter_dx90.cpp
    wireframe_dx9.cpp
    worldtwotextureblend.cpp
    worldvertexalpha.cpp
    worldvertextransition.cpp
    worldvertextransition_dx8_helper.cpp
    writez_dx9.cpp
    writestencil_dx9.cpp
    yuv.cpp
    eyeball.cpp
    ../../public/tier0/memoverride.cpp
)


target_include_directories(stdshader_dx9 PRIVATE
    .
    ../../public
    ../../public/tier0
    ../../public/tier1
    fxctmp9
    vshtmp9
)

target_include_directories(stdshader_dx9 PRIVATE
    ${SDL2_INCLUDE_DIRS}
)


target_link_libraries(stdshader_dx9 PRIVATE
    tier0
    shaderlib
    tier1
    mathlib
)

if(ANDROID)
target_link_libraries(stdshader_dx9 PRIVATE
    ANDROID_SUPPORT
)

endif()

