# ------------------------------------------------- #
# waf2cmake automatically generated CMakeLists.txt,
# Fix if needed before using.
# ------------------------------------------------- #

add_library(ServerBrowser SHARED)

target_sources(ServerBrowser PRIVATE
    BaseGamesPage.cpp
    DialogAddServer.cpp
    DialogGameInfo.cpp
    DialogServerPassword.cpp
    FavoriteGames.cpp
    HistoryGames.cpp
    InternetGames.cpp
    LanGames.cpp
    ModList.cpp
    ServerBrowser.cpp
    ServerBrowserDialog.cpp
    ServerContextMenu.cpp
    ServerListCompare.cpp
    VACBannedConnRefusedDialog.cpp
    QuickListPanel.cpp
    ../public/vgui_controls/vgui_controls.cpp
    ../common/ServerBrowser/blacklisted_server_manager.cpp
    ../public/tier0/memoverride.cpp
)


target_include_directories(ServerBrowser PRIVATE
    .
    ../public
    ../public/tier0
    ../public/tier1
    ../common
)


target_link_libraries(ServerBrowser PRIVATE
    tier0
    vgui_controls
    mathlib
    tier1
    tier2
    tier3
    vstdlib
    steam_api
)


