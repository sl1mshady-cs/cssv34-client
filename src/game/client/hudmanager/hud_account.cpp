#include <cbase.h>
#include "hud_account.h"
#include <c_basecombatweapon.h>
#include <c_baseplayer.h>
#include <c_cs_player.h>

extern IVEngineClient* engine;
extern IUISystem* uisys;

int g_iPlayerAmmo = 0;
int g_iPlayerAmmo2 = 0;
int g_iMoney = 16000;
bool g_bUsingKnife = false;
Rml::DataModelHandle hud_acchandle;
IUIDocument* m_pAccDocument = nullptr;

void CHudAcc::Initialize()
{
    Rml::DataModelConstructor ctor = uisys->CreateDataModel(InGame, "account");
    if (!ctor) return;

    ctor.Bind("ammo", &g_iPlayerAmmo);
    ctor.Bind("ammo2", &g_iPlayerAmmo2);
    ctor.Bind("money", &g_iMoney);
    ctor.Bind("knife", &g_bUsingKnife);

    hud_acchandle = ctor.GetModelHandle();

    m_pAccDocument = uisys->CreateDocument(InGame);
    m_pAccDocument->SetDocument("hud_account.rml");
    m_pAccDocument->SetVisible(true);
}

void CHudAcc::Think()
{
    if (!hud_acchandle) return;

    if (!engine || !engine->IsInGame())
    {
        return;
    }

    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    if (!pPlayer || !pPlayer->IsAlive())
    {
        m_pAccDocument->SetVisible(false);
        return;
    }

    m_pAccDocument->SetVisible(true);

    C_CSPlayer* pLocal = ToCSPlayer(C_BasePlayer::GetLocalPlayer());
    if (!pLocal) return;

    g_iMoney = pLocal->GetAccount();

    C_BaseCombatWeapon* pWeapon = pLocal->GetActiveWeapon();
    if (pWeapon)
    {
        const char* szClass = pWeapon->GetClassname();
        g_bUsingKnife = szClass && (V_strstr(szClass, "knife") != nullptr ||
            V_strstr(szClass, "bayonet") != nullptr);

        if (!g_bUsingKnife)
        {
            g_iPlayerAmmo = pWeapon->Clip1();

            int nAmmoType = pWeapon->GetPrimaryAmmoType();
            g_iPlayerAmmo2 = (nAmmoType >= 0) ? pLocal->GetAmmoCount(nAmmoType) : 0;
        }
    }
    else
    {
        g_bUsingKnife = false;
    }

    hud_acchandle.DirtyAllVariables();
}

void CHudAcc::Shutdown()
{
}