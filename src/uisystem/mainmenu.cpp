#include "mainmenu.h"
#include <cdll_int.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <filesystem.h>

MainMenu mainmenu;
Rml::DataModelHandle m_MainMenuHandle;
IUIDocument* m_pMainMenuDoc = nullptr;

extern IVEngineClient* engine;

enum MainMenuState
{
    Main = 0,
    CreateServer,
    FindServers,
    Settings,
    Quit,
    InQuitDialog
};

MainMenuState menustate = Main;

Rml::String m_strState = "main";
Rml::String m_strServerName = "Counter-Strike: Source";
Rml::String m_strMapName = "de_dust2";
Rml::String m_strMaxPlayers = "32";

static Rml::Vector<std::string> m_Maps =
{
    "de_dust2",
    "de_aztec",
    "de_inferno",
    "cs_office",
    "cs_italy",
    "de_cbble",
};

static Rml::String m_strVisibility = "local";
static Rml::String m_strServerPassword = "";
static Rml::String m_strMaxBots = "0";
static Rml::String m_strDifficulty = "normal";
static Rml::String m_strSelectedMap = "";          
static void SyncState()
{
    switch (menustate)
    {
    case Main:         m_strState = "main";          break;
    case CreateServer: m_strState = "create_server"; break;
    case FindServers:  m_strState = "find_servers";  break;
    case Settings:     m_strState = "settings";      break;
    case Quit:         m_strState = "quit";          break;
    case InQuitDialog: m_strState = "quit_dialog";   break;
    }

    if (m_MainMenuHandle)
    {
        m_MainMenuHandle.DirtyVariable("menustate");
        m_MainMenuHandle.DirtyVariable("state");
    }
}

void MainMenu::Init()
{
    Rml::DataModelConstructor cons =
        uisys.StateToContext(uisys.GetUIState())->CreateDataModel("main_menu");

    cons.Bind("menustate", &menustate);
    cons.Bind("state", &m_strState);

    cons.Bind("server_name", &m_strServerName);
    cons.Bind("map", &m_strMapName);
    cons.Bind("max_players", &m_strMaxPlayers);
    cons.RegisterArray<Rml::Vector<std::string>>();
    cons.Bind("maps", &m_Maps);

    cons.Bind("visibility", &m_strVisibility);
    cons.Bind("server_password", &m_strServerPassword);
    cons.Bind("max_bots", &m_strMaxBots);
    cons.Bind("difficulty", &m_strDifficulty);
    cons.Bind("selected_map", &m_strSelectedMap);
    
    cons.BindEventCallback("goto",
        [](Rml::DataModelHandle handle, Rml::Event&, const Rml::VariantList& args)
        {
            if (args.empty())
                return;

            const Rml::String target = args[0].Get<Rml::String>();

            if (target == "main")               menustate = Main;
            else if (target == "create_server") menustate = CreateServer;
            else if (target == "find_servers")  menustate = FindServers;
            else if (target == "settings")      menustate = Settings;
            else if (target == "quit")          menustate = Quit;
            else
            {
                Warning("MainMenu: unknown goto target '%s'\n", target.c_str());
                return;
            }

            if (target == "create_server")
            {
                m_Maps.clear();

                FileFindHandle_t findHandle = NULL;

                const char* pszFilename = g_pFullFileSystem->FindFirstEx("maps/*.bsp", "GAME", &findHandle);
                while (pszFilename)
                {
                    char mapname[256];
                    char* ext, * str;

                    _snprintf(mapname, sizeof(mapname), "maps/%s", pszFilename);
                    if (!g_pFullFileSystem->FileExists(mapname, "GAME"))
                    {
                        goto nextFile;
                    }

                    
                    str = Q_strstr(pszFilename, "maps");
                    if (str)
                    {
                        strncpy(mapname, str + 5, sizeof(mapname) - 1);
                    }
                    else
                    {
                        strncpy(mapname, pszFilename, sizeof(mapname) - 1);
                    }
                    ext = Q_strstr(mapname, ".bsp");
                    if (ext)
                    {
                        *ext = 0;
                    }

                    m_Maps.push_back(mapname);

                nextFile:
                    pszFilename = g_pFullFileSystem->FindNext(findHandle);
                }
                g_pFullFileSystem->FindClose(findHandle);

                if (handle) handle.DirtyVariable("maps");

                m_strSelectedMap = "";
                if (handle) handle.DirtyVariable("selected_map");
            }

            SyncState();
        });

        cons.BindEventCallback("set_visibility",
        [](Rml::DataModelHandle handle, Rml::Event&, const Rml::VariantList& args)
        {
            if (args.empty()) return;
            const Rml::String v = args[0].Get<Rml::String>();
            if (v != "local" && v != "public_unlisted" && v != "public_listed")
                return;
            m_strVisibility = v;
            if (handle) handle.DirtyVariable("visibility");
        });

        cons.BindEventCallback("set_difficulty",
        [](Rml::DataModelHandle handle, Rml::Event&, const Rml::VariantList& args)
        {
            if (args.empty()) return;
            const Rml::String d = args[0].Get<Rml::String>();
            if (d != "easy" && d != "normal" && d != "hard" && d != "expert")
                return;
            m_strDifficulty = d;
            if (handle) handle.DirtyVariable("difficulty");
        });

        cons.BindEventCallback("select_map",
        [](Rml::DataModelHandle handle, Rml::Event&, const Rml::VariantList& args)
        {
            if (args.empty()) return;
            m_strSelectedMap = args[0].Get<Rml::String>();
            m_strMapName = m_strSelectedMap;
            if (handle)
            {
                handle.DirtyVariable("selected_map");
                handle.DirtyVariable("map");
            }
        });

        cons.BindEventCallback("start_server",
        [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
        {
            if (!engine) return;

            if (m_strSelectedMap.empty())
            {
                Warning("MainMenu: no map selected\n");
                return;
            }

            int maxPlayers = atoi(m_strMaxPlayers.c_str());
            if (maxPlayers < 1)  maxPlayers = 1;
            if (maxPlayers > 32) maxPlayers = 32;

            int maxBots = atoi(m_strMaxBots.c_str());
            if (maxBots < 0) maxBots = 0;

            int botDifficulty = 1;
            if (m_strDifficulty == "easy")   botDifficulty = 0;
            else if (m_strDifficulty == "normal") botDifficulty = 1;
            else if (m_strDifficulty == "hard")   botDifficulty = 2;
            else if (m_strDifficulty == "expert") botDifficulty = 3;

            const char* sv_lan = (m_strVisibility == "local") ? "1" : "0";

            char cmd[1024];
            snprintf(cmd, sizeof(cmd),
                "sv_lan %s\n"
                "hostname \"%s\"\n"
                "sv_password \"%s\"\n"
                "maxplayers %d\n"
                "bot_quota %d\n"
                "bot_difficulty %d\n"
                "map %s\n",
                sv_lan,
                m_strServerName.c_str(),
                m_strServerPassword.c_str(),
                maxPlayers,
                maxBots,
                botDifficulty,
                m_strSelectedMap.c_str());

            engine->ClientCmd_Unrestricted(cmd);

            menustate = Main;
            SyncState();
        });

    m_MainMenuHandle = cons.GetModelHandle();

    m_pMainMenuDoc = uisys.CreateDocument(InMainMenu);
    if (m_pMainMenuDoc)
        m_pMainMenuDoc->SetDocument("main_menu.rml");

    m_pMainMenuDoc->SetVisible(true);
}

void MainMenu::Think()
{
    if (menustate == Quit)
    {
        uisys.ShowDialog("#RmlUi_Quit", "#RmlUi_AreYouSureToQuit",
            DIALOG_BTN_OK | DIALOG_BTN_CANCEL);
        menustate = InQuitDialog;
        SyncState();
    }

    if (menustate == InQuitDialog)
    {
        DialogResult res = uisys.GetDialogResult();
        if (res == OK)
        {
            engine->ClientCmd_Unrestricted("quit");
            menustate = Main;
        }
        else if (res == Cancel)
        {
            uisys.CloseDialog();
            menustate = Main;
        }
        else
        {
            Warning("Unexpected DialogResult\n");
            menustate = Main;
        }

        SyncState();
    }
}

void MainMenu::Shutdown()
{
    m_MainMenuHandle = nullptr;
    m_pMainMenuDoc = nullptr;
}