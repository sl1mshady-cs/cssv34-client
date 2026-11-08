#include <cbase.h>
#include "hud_roundtimer.h"
#include <cs_gamerules.h>
#include <c_baseplayer.h>

extern IVEngineClient* engine;
extern IUISystem* uisys;

std::string g_strRoundTime;

Rml::DataModelHandle hud_rthandle;
IUIDocument* m_pRTDocument = nullptr;

void CHudRT::Initialize()
{
    Rml::DataModelConstructor ctor = uisys->CreateDataModel(InGame, "rt");
    if (!ctor) return;

    ctor.Bind("time", &g_strRoundTime);

    hud_rthandle = ctor.GetModelHandle();

    m_pRTDocument = uisys->CreateDocument(InGame);
    m_pRTDocument->SetDocument("hud_roundtimer.rml");
    m_pRTDocument->SetVisible(true);
}

void CHudRT::Think()
{
    if (!hud_rthandle) return;

    if (!engine || !engine->IsInGame())
    {
        return;
    }

    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    if (!pPlayer || !pPlayer->IsAlive())
    {
        m_pRTDocument->SetVisible(false);
        return;
    }

    m_pRTDocument->SetVisible(true);
    
    CCSGameRules* rules = CSGameRules();
    if (!rules) return;

    const int nTotalSeconds = static_cast<int>(rules->GetRoundRemainingTime());

    int nSeconds = nTotalSeconds % 60;
    int nMinutes = nTotalSeconds / 60;

    g_strRoundTime = std::to_string(nMinutes) + ":" + std::to_string(nSeconds);

    hud_rthandle.DirtyAllVariables();
}

void CHudRT::Shutdown()
{
}