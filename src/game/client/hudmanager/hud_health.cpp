#include <cbase.h>
#include "hud_health.h"
#include "c_plantedc4.h"

extern IVEngineClient* engine;
extern IUISystem* uisys;

int g_iPlayerHealth = 0;
int g_iPlayerArmor = 0;
Rml::DataModelHandle hud_healthhandle;

void CHudHealth::Initialize()
{
    Rml::DataModelConstructor ctor = uisys->CreateDataModel(InGame, "health");
    if (!ctor) return;

    ctor.Bind("health", &g_iPlayerHealth);
    ctor.Bind("armor", &g_iPlayerArmor);

    hud_healthhandle = ctor.GetModelHandle();

    IUIDocument* doc = uisys->CreateDocument(InGame);
    doc->SetDocument("hud_health.rml");
    doc->SetVisible(true);
}

void CHudHealth::Think()
{
    if (!hud_healthhandle) return;

    if (!engine || !engine->IsInGame())
    {
        return;
    }

    C_CSPlayer* pLocal = ToCSPlayer(C_BasePlayer::GetLocalPlayer());
    if (!pLocal) return;

    g_iPlayerHealth = pLocal->GetHealth();
    g_iPlayerArmor = pLocal->ArmorValue();
    hud_healthhandle.DirtyAllVariables();
}

void CHudHealth::Shutdown()
{
}