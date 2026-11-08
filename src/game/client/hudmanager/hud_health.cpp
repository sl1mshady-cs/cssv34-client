#include <cbase.h>
#include "hud_health.h"
#include "c_plantedc4.h"

extern IVEngineClient* engine;
extern IUISystem* uisys;

int g_iPlayerHealth = 0;
int g_iPlayerArmor = 0;
int g_flPlayerHealthPer = 0.0f;
int g_flPlayerArmorPer = 0.0f;
Rml::DataModelHandle hud_healthhandle;
IUIDocument* m_pHealthDocument = nullptr;

void CHudHealth::Initialize()
{
    Rml::DataModelConstructor ctor = uisys->CreateDataModel(InGame, "health");
    if (!ctor) return;

    ctor.Bind("health", &g_iPlayerHealth);
    ctor.Bind("armor", &g_iPlayerArmor);

    ctor.Bind("health_percent", &g_flPlayerHealthPer);
    ctor.Bind("armor_percent", &g_flPlayerArmorPer);

    hud_healthhandle = ctor.GetModelHandle();

    m_pHealthDocument = uisys->CreateDocument(InGame);
    m_pHealthDocument->SetDocument("hud_health.rml");
    m_pHealthDocument->SetVisible(true);
}

void CHudHealth::Think()
{
    if (!hud_healthhandle) return;

    if (!engine || !engine->IsInGame())
    {
        return;
    }

    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    if (!pPlayer || !pPlayer->IsAlive())
    {
        m_pHealthDocument->SetVisible(false);
        return;
    }

    m_pHealthDocument->SetVisible(true);

    C_CSPlayer* pLocal = ToCSPlayer(C_BasePlayer::GetLocalPlayer());
    if (!pLocal) return;

    g_iPlayerHealth = pLocal->GetHealth();
    g_iPlayerArmor = pLocal->ArmorValue();

    g_flPlayerArmorPer = g_iPlayerArmor;
    if (g_flPlayerArmorPer > 100.0f) g_flPlayerArmorPer = 100.0f;
    if (g_flPlayerArmorPer < 0.0f) g_flPlayerArmorPer = 0.0f;

    g_flPlayerHealthPer = g_iPlayerHealth;
    if (g_flPlayerHealthPer > 100.0f) g_flPlayerHealthPer = 100.0f;
    if (g_flPlayerHealthPer < 0.0f) g_flPlayerHealthPer = 0.0f;

    hud_healthhandle.DirtyAllVariables();
}

void CHudHealth::Shutdown()
{
}