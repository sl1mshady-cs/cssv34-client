#include <cbase.h>
#include "hud_scoreboard.h"

#include "c_cs_playerresource.h"
#include "c_cs_player.h"
#include "c_team.h"
#include "cs_gamerules.h"

extern IVEngineClient* engine;
extern IUISystem* uisys;
extern C_PlayerResource* g_PR;

static std::vector<ScoreboardTeam> g_ScoreboardTeams;
static Rml::DataModelHandle g_ScoreboardHandle;
static IUIDocument* m_pScoreboardDocument = nullptr;

bool g_bScoreboardVis = false;
void prmlscore_f(const CCommand& args) { g_bScoreboardVis = true; }
void mrmlscore_f(const CCommand& args) { g_bScoreboardVis = false; }

ConCommand prmlscore("+rml_scoreboard", prmlscore_f);
ConCommand mrmlscore("-rml_scoreboard", mrmlscore_f);

bool SortPlayers(const ScoreboardPlayer& a, const ScoreboardPlayer& b)
{
    if (a.score != b.score)
        return a.score > b.score;
    return a.deaths < b.deaths;
}

void CHudScoreboard::Initialize()
{
    Rml::DataModelConstructor ctor = uisys->CreateDataModel(InGame, "scoreboard");
    if (!ctor) return;

    Rml::StructHandle hStruct = ctor.RegisterStruct<ScoreboardPlayer>();
    hStruct.RegisterMember("name", &ScoreboardPlayer::name);
    hStruct.RegisterMember("score", &ScoreboardPlayer::score);
    hStruct.RegisterMember("deaths", &ScoreboardPlayer::deaths);
    hStruct.RegisterMember("latency", &ScoreboardPlayer::latency);
    hStruct.RegisterMember("isBot", &ScoreboardPlayer::isBot);

    ctor.RegisterArray<std::vector<ScoreboardPlayer>>();

    Rml::StructHandle hStruct2 = ctor.RegisterStruct<ScoreboardTeam>();
    hStruct2.RegisterMember("name", &ScoreboardTeam::name);
    hStruct2.RegisterMember("score", &ScoreboardTeam::score);
    hStruct2.RegisterMember("players", &ScoreboardTeam::players);

    ctor.RegisterArray<std::vector<ScoreboardTeam>>();

    ctor.Bind("teams", &g_ScoreboardTeams);

    g_ScoreboardHandle = ctor.GetModelHandle();

    m_pScoreboardDocument = uisys->CreateDocument(InGame);
    m_pScoreboardDocument->SetDocument("hud_scoreboard.rml");
    m_pScoreboardDocument->SetVisible(false);
}

void CHudScoreboard::UpdatePlayerList()
{
    g_ScoreboardTeams.clear();

    if (!g_PR) return;

    ScoreboardTeam ctTeam;
    ctTeam.name = "Counter-Terrorists";
    C_Team* pCT = GetGlobalTeam(TEAM_CT);
    ctTeam.score = pCT ? pCT->Get_Score() : 0;

    ScoreboardTeam tTeam;
    tTeam.name = "Terrorists";
    C_Team* pT = GetGlobalTeam(TEAM_TERRORIST);
    tTeam.score = pT ? pT->Get_Score() : 0;

    for (int i = 1; i <= MAX_PLAYERS; ++i)
    {
        if (!g_PR->IsConnected(i))
            continue;

        int teamNum = g_PR->GetTeam(i);

        if (teamNum != TEAM_CT && teamNum != TEAM_TERRORIST)
            continue;

        ScoreboardPlayer player;

        const char* playerName = g_PR->GetPlayerName(i);
        player.name = playerName ? playerName : "";
        player.score = g_PR->GetPlayerScore(i);
        player.deaths = g_PR->GetDeaths(i);
        player.isBot = g_PR->IsFakePlayer(i);

        int ping = g_PR->GetPing(i);
        if (ping < 1)
        {
            player.latency = player.isBot ? -1 : 0;
        }
        else
        {
            player.latency = ping;
        }

        if (teamNum == TEAM_CT)
        {
            ctTeam.players.push_back(player);
        }
        else if (teamNum == TEAM_TERRORIST)
        {
            tTeam.players.push_back(player);
        }
    }

    std::sort(ctTeam.players.begin(), ctTeam.players.end(), SortPlayers);
    std::sort(tTeam.players.begin(), tTeam.players.end(), SortPlayers);

    g_ScoreboardTeams.push_back(ctTeam);
    g_ScoreboardTeams.push_back(tTeam);
}

void CHudScoreboard::Think()
{
    if (!g_ScoreboardHandle) return;
    if (!engine || !engine->IsInGame())
    {
        if (m_pScoreboardDocument) m_pScoreboardDocument->SetVisible(false);
        return;
    }

    if (m_pScoreboardDocument)
    {
        m_pScoreboardDocument->SetVisible(g_bScoreboardVis);
    }

    if (!g_bScoreboardVis) return;

    UpdatePlayerList();

    g_ScoreboardHandle.DirtyAllVariables();
}

void CHudScoreboard::Shutdown()
{
    g_ScoreboardTeams.clear();
    m_pScoreboardDocument = nullptr;
    g_ScoreboardHandle = Rml::DataModelHandle();
}