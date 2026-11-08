#include <cbase.h>
#include "hud_radar.h"
#include "c_plantedc4.h"

extern IVEngineClient* engine;
extern IUISystem* uisys;

Rml::String sRadarTransform = "rotate(-90deg)";
Rml::String sMapTexture = "";
Rml::String sPosName = "";

static float sOvPosX = 0.0f;
static float sOvPosY = 0.0f;
static float sOvScale = 1.0f;
static bool  sOvValid = false;

static constexpr float kOverviewTextureSize = 1024.0f;
static constexpr float kOverviewDisplaySize = 256.0f;

static Rml::String sLastLevel;
Rml::DataModelHandle handle;

static constexpr float kRadarSize = 200.0f;
static constexpr float kRadarRadius = kRadarSize * 0.5f;

struct RadarIcon
{
    Rml::String cls;
    float x = 0.0f;
    float y = 0.0f;
};

static Rml::Vector<RadarIcon> sIcons;
static bool sRadarHidden = false;

static const char* TeamClass(int team)
{
    if (team == 3) return "ct";
    if (team == 2) return "t";
    return "unknown";
}

static void AddIcon(const Rml::String& cls, float x, float y)
{
    RadarIcon icon;
    icon.cls = cls;
    icon.x = x;
    icon.y = y;
    sIcons.push_back(icon);
}

static bool LoadOverviewData(const std::string& mapname)
{
    sOvValid = false;

    char path[MAX_PATH];
    V_snprintf(path, sizeof(path), "resource/overviews/%s.txt", mapname.c_str());

    KeyValues* kv = new KeyValues(mapname.c_str());
    if (!kv->LoadFromFile(filesystem, path))
    {
        kv->deleteThis();
        return false;
    }

    sOvPosX = kv->GetFloat("pos_x", 0.0f);
    sOvPosY = kv->GetFloat("pos_y", 0.0f);
    sOvScale = kv->GetFloat("scale", 1.0f);
    kv->deleteThis();

    sOvValid = (sOvScale > 0.0f);
    return sOvValid;
}

void CHudRadar_Rml::Initialize()
{
    Rml::DataModelConstructor ctor = uisys->CreateDataModel(InGame, "radar");
    if (!ctor) return;

    Rml::StructHandle icon_ctor = ctor.RegisterStruct<RadarIcon>();
    icon_ctor.RegisterMember("cls", &RadarIcon::cls);
    icon_ctor.RegisterMember("x", &RadarIcon::x);
    icon_ctor.RegisterMember("y", &RadarIcon::y);

    ctor.RegisterArray<Rml::Vector<RadarIcon>>();

    ctor.Bind("sRadarTransform", &sRadarTransform);
    ctor.Bind("sMapTexture", &sMapTexture);
    ctor.Bind("pos_name", &sPosName);
    ctor.Bind("icons", &sIcons);
    ctor.Bind("radar_hidden", &sRadarHidden);

    handle = ctor.GetModelHandle();

    IUIDocument* doc = uisys->CreateDocument(InGame);
    doc->SetDocument("hud_radar.rml");
    doc->SetVisible(true);
}

void CHudRadar_Rml::Think()
{
    if (!handle) return;

    if (!engine || !engine->IsInGame())
    {
        if (!sRadarHidden) { sRadarHidden = true; handle.DirtyVariable("radar_hidden"); }
        return;
    }

    const char* levelName = engine->GetLevelName();
    if (levelName && sLastLevel != levelName)
    {
        sLastLevel = levelName;

        std::string map = levelName;
        size_t slash = map.find_last_of("/\\");
        if (slash != std::string::npos) map = map.substr(slash + 1);
        size_t dot = map.find_last_of('.');
        if (dot != std::string::npos) map = map.substr(0, dot);

        sMapTexture = "textures/map_overviews/" + map + ".png";
        handle.DirtyVariable("sMapTexture");

        LoadOverviewData(map);
    }

    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    if (!pPlayer || !pPlayer->IsAlive())
    {
        if (!sRadarHidden) { sRadarHidden = true; handle.DirtyVariable("radar_hidden"); }
        if (!sIcons.empty()) { sIcons.clear(); handle.DirtyVariable("icons"); }
        return;
    }

    if (sRadarHidden) { sRadarHidden = false; handle.DirtyVariable("radar_hidden"); }

    QAngle va;
    engine->GetViewAngles(va);
    const float theta = va[1] - 90.0f;

    if (sOvValid)
    {
        const Vector origin = pPlayer->GetAbsOrigin();
        const float ratio = kOverviewTextureSize / kOverviewDisplaySize;

        const float u = (origin.x - sOvPosX) / (sOvScale * ratio);
        const float v = (sOvPosY - origin.y) / (sOvScale * ratio);

        const float ox = u - kOverviewDisplaySize * 0.5f;
        const float oy = v - kOverviewDisplaySize * 0.5f;

        const float rad = theta * float(M_PI) / 180.0f;
        const float cs = cosf(rad);
        const float sn = sinf(rad);
        const float rx = ox * cs - oy * sn;
        const float ry = ox * sn + oy * cs;

        char buf[128];
        V_snprintf(buf, sizeof(buf),
            "translate(%.2fpx, %.2fpx) rotate(%.2fdeg)", -rx, -ry, theta);

        if (sRadarTransform != buf)
        {
            sRadarTransform = buf;
            handle.DirtyVariable("sRadarTransform");
        }
    }
    else
    {
        char buf[64];
        V_snprintf(buf, sizeof(buf), "rotate(%.2fdeg)", theta);
        if (sRadarTransform != buf)
        {
            sRadarTransform = buf;
            handle.DirtyVariable("sRadarTransform");
        }
    }

    sIcons.clear();

    const Vector origin = pPlayer->GetAbsOrigin();
    const int    localTeam = pPlayer->GetTeamNumber();

    AddIcon(Rml::String("radar_icon local ") + TeamClass(localTeam),
        kRadarRadius, kRadarRadius);

    if (sOvValid)
    {
        for (int i = 1; i <= gpGlobals->maxClients; ++i)
        {
            C_BasePlayer* pOther = UTIL_PlayerByIndex(i);
            if (!pOther || pOther == pPlayer) continue;
            if (!pOther->IsAlive()) continue;
            if (pOther->GetTeamNumber() != localTeam) continue;

            float rx, ry, rz;

            WorldToRadar(pOther->GetAbsOrigin(), origin, va, rx, ry, rz);

            AddIcon(Rml::String("radar_icon player ") + TeamClass(pOther->GetTeamNumber()),
                rx, ry);
        }

        for (int i = 0; i < g_PlantedC4s.Count(); ++i)
        {
            C_PlantedC4* c4 = g_PlantedC4s[i];

            float rx, ry, rz;
            WorldToRadar(c4->GetAbsOrigin(), origin, va, rx, ry, rz);

            AddIcon(Rml::String("radar_icon c4"), rx, ry);
        }
    }

    handle.DirtyVariable("icons");

    const char* place = pPlayer->GetLastKnownPlaceName();
    if (place && *place && sPosName != place)
    {
        sPosName = place;
        handle.DirtyVariable("pos_name");
    }
}

void CHudRadar_Rml::WorldToRadar(const Vector& location, const Vector& origin,
    const QAngle& angles,
    float& x, float& y, float& z_delta)
{
    if (!sOvValid)
    {
        x = kRadarRadius;
        y = kRadarRadius;
        z_delta = 0.0f;
        return;
    }

    const float ratio = kOverviewTextureSize / kOverviewDisplaySize;

    const float u = (location.x - sOvPosX) / (sOvScale * ratio);
    const float v = (sOvPosY - location.y) / (sOvScale * ratio);
    const float pu = (origin.x - sOvPosX) / (sOvScale * ratio);
    const float pv = (sOvPosY - origin.y) / (sOvScale * ratio);

    const float ou = u - pu;
    const float ov = v - pv;

    const float theta = angles.y - 90.0f;
    const float rad = theta * float(M_PI) / 180.0f;
    const float cs = cosf(rad);
    const float sn = sinf(rad);

    const float rx = ou * cs - ov * sn;
    const float ry = ou * sn + ov * cs;

    x = kRadarRadius + rx;
    y = kRadarRadius + ry;
    z_delta = location.z - origin.z;
}

void CHudRadar_Rml::Shutdown()
{
    sIcons.clear();
    sOvValid = false;
    sLastLevel.clear();
}