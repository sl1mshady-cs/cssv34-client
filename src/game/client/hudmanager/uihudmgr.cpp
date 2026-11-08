#include <cbase.h>
#include <cdll_int.h>
#include <hudmanager/uihudmgr.h>
#include <KeyValues.h>
#include <filesystem.h>
#include <string>
#include "hudmanager/hud_radar.h"
#include "hudmanager/hud_health.h"

IUISystem* uisys = nullptr;
CSysModule* uisysmod = nullptr;

extern IVEngineClient* engine;

CHudRadar_Rml hud_radar_rml;
CHudHealth hud_health_rml;

void CHudMgr::Init()
{
    uisysmod = Sys_LoadModule("uisystem");
    CreateInterfaceFn factory = Sys_GetFactory(uisysmod);
    if (!factory) return;

    uisys = (IUISystem*)factory(UISYSTEM_INTERFACE_VERSION, NULL);
    if (!uisys) return;

    hud_radar_rml.Initialize();
    hud_health_rml.Initialize();
}

void CHudMgr::Think()
{
    hud_radar_rml.Think();
    hud_health_rml.Think();
}

void CHudMgr::Shutdown()
{
    hud_radar_rml.Shutdown();
    hud_health_rml.Shutdown();

    if (uisysmod) { Sys_UnloadModule(uisysmod); uisysmod = nullptr; }
    uisys = nullptr;
}