#include "mainmenu.h"
#include <cdll_int.h>

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

static void SyncState()
{
    switch (menustate)
    {
    case Main:         m_strState = "main";         break;
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

    cons.BindEventCallback("goto",
        [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
        {
            if (args.empty())
                return;

            const Rml::String target = args[0].Get<Rml::String>();

            if (target == "main")          menustate = Main;
            else if (target == "create_server") menustate = CreateServer;
            else if (target == "find_servers")  menustate = FindServers;
            else if (target == "settings")      menustate = Settings;
            else if (target == "quit")          menustate = Quit;
            else
            {
                Warning("MainMenu: unknown goto target '%s'\n", target.c_str());
                return;
            }

            SyncState();
        });

    cons.BindEventCallback("start_server",
        [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
        {
            if (!engine)
                return;

            int maxPlayers = atoi(m_strMaxPlayers.c_str());
            if (maxPlayers < 1)  maxPlayers = 1;
            if (maxPlayers > 32) maxPlayers = 32;

            char cmd[512];
            snprintf(cmd, sizeof(cmd),
                "map %s\n",
                m_strServerName.c_str(),
                maxPlayers,
                m_strMapName.c_str());

            engine->ClientCmd_Unrestricted(cmd);

            menustate = Main;
            SyncState();
        });

    m_MainMenuHandle = cons.GetModelHandle();

    m_pMainMenuDoc = uisys.CreateDocument(InMainMenu);
    if (m_pMainMenuDoc)
        m_pMainMenuDoc->SetDocument("main_menu.rml");
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