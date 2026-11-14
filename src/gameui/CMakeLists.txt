# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(GameUI SHARED)

target_sources(GameUI PRIVATE
    BackgroundMenuButton.cpp
    BasePanel.cpp
    GameConsole.cpp
    GameUI_Interface.cpp
    ModInfo.cpp
    MouseMessageForwardingPanel.cpp
    ../common/GameUI/ObjectList.cpp
    PanelListPanel.cpp
    RunGameEngine.cpp
    ../common/GameUI/scriptobject.cpp
    Sys_Utils.cpp
    TextEntryBox.cpp
    TGAImagePanel.cpp
    ../public/vgui_controls/vgui_controls.cpp
    VGuiSystemModuleLoader.cpp
    BonusMapsDatabase.cpp
    ../common/language.cpp
    ../common/imageutils.cpp
    SaveGameBrowserDialog.cpp
    gameui_util.cpp
    BitmapImagePanel.cpp
    CommandCheckButton.cpp
    CvarNegateCheckButton.cpp
    ../common/GameUI/cvarslider.cpp
    CvarTextEntry.cpp
    CvarToggleCheckButton.cpp
    HapticControlBox.cpp
    KeyToggleCheckButton.cpp
    LabeledCommandComboBox.cpp
    URLButton.cpp
    vcontrolslistpanel.cpp
    BenchmarkDialog.cpp
    BonusMapsDialog.cpp
    CommentaryDialog.cpp
    CommentaryExplanationDialog.cpp
    ContentControlDialog.cpp
    CustomTabExplanationDialog.cpp
    GameConsoleDialog.cpp
    LoadGameDialog_Xbox.cpp
    LoadGameDialog.cpp
    MultiplayerAdvancedDialog.cpp
    NewGameDialog.cpp
    PlayerListDialog.cpp
    SaveGameDialog_Xbox.cpp
    SaveGameDialog.cpp
    LoadCommentaryDialog.cpp
    LoadingDialog.cpp
    BaseSaveGameDialog.cpp
    CreateMultiplayerGameBotPage.cpp
    CreateMultiplayerGameDialog.cpp
    CreateMultiplayerGameGameplayPage.cpp
    CreateMultiplayerGameServerPage.cpp
    OptionsDialog_Xbox.cpp
    ControllerDialog.cpp
    matchmaking/achievementsdialog.cpp
    matchmaking/basedialog.cpp
    matchmaking/dialogmenu.cpp
    matchmaking/leaderboarddialog.cpp
    matchmaking/matchmakingbasepanel.cpp
    matchmaking/pausedialog.cpp
    matchmaking/sessionlobbydialog.cpp
    matchmaking/sessionoptionsdialog.cpp
    matchmaking/sessionbrowserdialog.cpp
    matchmaking/welcomedialog.cpp
    OptionsDialog.cpp
    OptionsSubAudio.cpp
    OptionsSubDifficulty.cpp
    OptionsSubGame.cpp
    OptionsSubKeyboard.cpp
    OptionsSubMouse.cpp
    OptionsSubTouch.cpp
    OptionsSubMultiplayer.cpp
    OptionsSubPortal.cpp
    OptionsSubVideo.cpp
    OptionsSubVoice.cpp
    ../public/tier0/memoverride.cpp
)

if(WIN32)
target_sources(GameUI PRIVATE
    LogoFile.cpp
    ChangeGameDialog.cpp
    OptionsSubHaptics.cpp
)

endif()

target_include_directories(GameUI PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
    ../common/GameUI
    ../thirdparty
)


target_link_libraries(GameUI PRIVATE
    tier0
    vgui_controls
    tier1
    tier2
    tier3
    vstdlib
    vtf
    bitmap
    mathlib
    SDL2
    steam_api
    matsys_controls
    JPEG
    PNG
    ZLIB
)

if(WIN32)
target_link_libraries(GameUI PRIVATE
    USER32
    GDI32
)

endif()

