#include "uisystem.h"
#include "tier1/utlvector.h"
#include <RmlUi/Core.h>
#include <RmlUi/Lua/Lua.h>
#include <tier1/interface.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <materialsystem/imaterialsystem.h>
#include <materialsystem/imaterial.h>
#include <RmlUi/Core.h>
#include <RmlUi/Lua/Lua.h>
#include "inputsystem/iinputsystem.h"
#include "uidocument_internal.h"
#include "uifileinterface.h"
#include "uirenderinterface.h"
#include "uisysteminterface.h"

class CUISystem : public IUISystem
{
public:
	virtual bool Start(CreateInterfaceFn* factorylist, int nFactories);
	virtual void Draw();
	virtual void Stop();
	virtual IUIPanel* GetTopPanel();
	virtual IUIPanel* CreatePanel(IUIPanel* parent);
	virtual void DestroyPanel(IUIPanel* parent);
private:
    void LoadFont(const char* name);
    int m_iLastWidth;
    int m_iLastHeight;
    int m_nLastInputPollCount = 0;

    Rml::Input::KeyIdentifier ButtonCodeToRmlKeyID(ButtonCode_t code);
    bool InputHandleInputEvent(const InputEvent_t& event);
    bool DispatchEventToContext(Rml::Context& context, const InputEvent_t& event);
};

Rml::Context* g_pGlobalContext;
Rml::Context* GetGlobalContext()
{
    return g_pGlobalContext;
}

CUISystem uisys;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CUISystem, IUISystem, UISYSTEM_INTERFACE_VERSION, uisys);

IMaterial* g_pDefaultUIMaterial = nullptr;

inline int MouseIndexFromCode(ButtonCode_t code)
{
    const int index = code - MOUSE_LEFT;
    return (index < 0 || index > 2) ? 0 : index;
}

inline bool FingerToPixels(const InputEvent_t& event, int& outX, int& outY)
{
    if (!g_pMaterialSystem)
        return false;

    int w = 0, h = 0;
    g_pMaterialSystem->GetBackBufferDimensions(w, h);

    float nx = 0.f, ny = 0.f;
    std::memcpy(&nx, &event.m_nData2, sizeof(nx));
    std::memcpy(&ny, &event.m_nData3, sizeof(ny));

    outX = static_cast<int>(w * nx);
    outY = static_cast<int>(h * ny);
    return true;
}

bool CUISystem::Start(CreateInterfaceFn* factorylist, int nFactories)
{
	ConnectTier1Libraries(factorylist, nFactories);
	ConnectTier2Libraries(factorylist, nFactories);

	if (!g_pMaterialSystem || !g_pFullFileSystem || !g_pInputSystem)
	{
		Error("RmlUI can't get some required interfaces to function\n");
		return false;
	}

	g_pMaterialSystem->GetBackBufferDimensions(m_iLastWidth, m_iLastHeight);

	g_pDefaultUIMaterial = g_pMaterialSystem->FindMaterial("vgui/white", TEXTURE_GROUP_VGUI);
	if (g_pDefaultUIMaterial)
		g_pDefaultUIMaterial->IncrementReferenceCount();

    Rml::SetFileInterface(&fileinterface);
    Rml::SetRenderInterface(&renderinterface);
    Rml::SetSystemInterface(&sysinterface);

    if (!Rml::Initialise())
        return false;

    Rml::Lua::Initialise();

    LoadFont("cstrike");
    LoadFont("LatoLatin");
    LoadFont("JetBrainsMono");

    float scaleX = m_iLastWidth / 1366.0f;
    float scaleY = m_iLastHeight / 768.0f;

    float scale = sqrt(scaleX * scaleY);

    g_pGlobalContext = Rml::CreateContext("UISystem", Rml::Vector2i(m_iLastWidth, m_iLastHeight));
    g_pGlobalContext->SetDensityIndependentPixelRatio(scale);
    g_pGlobalContext->SetDimensions(Rml::Vector2i(m_iLastWidth, m_iLastHeight));

    return true;
}

void CUISystem::Draw()
{
    int nw, nh;
    g_pMaterialSystem->GetBackBufferDimensions(nw, nh);

    if (nw != m_iLastWidth || nh != m_iLastHeight)
    {
        m_iLastWidth = nw;
        m_iLastHeight = nh;

        float scaleX = m_iLastWidth / 1366.0f;
        float scaleY = m_iLastHeight / 768.0f;

        float scale = sqrt(scaleX * scaleY);
        g_pGlobalContext->SetDensityIndependentPixelRatio(scale);
        g_pGlobalContext->SetDimensions(Rml::Vector2i(nw, nh));
    }

    int nPollCount = g_pInputSystem->GetPollCount();
    if (m_nLastInputPollCount != nPollCount) {
        if (m_nLastInputPollCount != nPollCount - 1)
        {
            Warning("RmlUi is losing input messages! Call brian!\n");
        }

        m_nLastInputPollCount = nPollCount;

        int nEventCount = g_pInputSystem->GetEventCount();
        const InputEvent_t* pEvents = g_pInputSystem->GetEventData();
        for (int i = 0; i < nEventCount; ++i) {
            InputHandleInputEvent(pEvents[i]);
        }
    }

    g_pGlobalContext->Update();
    g_pGlobalContext->Render();
}

Rml::Input::KeyIdentifier CUISystem::ButtonCodeToRmlKeyID(ButtonCode_t code)
{
    switch (code)
    {
    case KEY_Q: return Rml::Input::KI_Q;
    case KEY_W: return Rml::Input::KI_W;
    case KEY_E: return Rml::Input::KI_E;
    case KEY_R: return Rml::Input::KI_R;
    case KEY_T: return Rml::Input::KI_T;
    case KEY_Y: return Rml::Input::KI_Y;
    case KEY_U: return Rml::Input::KI_U;
    case KEY_I: return Rml::Input::KI_I;
    case KEY_O: return Rml::Input::KI_O;
    case KEY_P: return Rml::Input::KI_P;
    case KEY_A: return Rml::Input::KI_A;
    case KEY_S: return Rml::Input::KI_S;
    case KEY_D: return Rml::Input::KI_D;
    case KEY_F: return Rml::Input::KI_F;
    case KEY_G: return Rml::Input::KI_G;
    case KEY_H: return Rml::Input::KI_H;
    case KEY_J: return Rml::Input::KI_J;
    case KEY_K: return Rml::Input::KI_K;
    case KEY_L: return Rml::Input::KI_L;
    case KEY_Z: return Rml::Input::KI_Z;
    case KEY_X: return Rml::Input::KI_X;
    case KEY_C: return Rml::Input::KI_C;
    case KEY_V: return Rml::Input::KI_V;
    case KEY_B: return Rml::Input::KI_B;
    case KEY_N: return Rml::Input::KI_N;
    case KEY_M: return Rml::Input::KI_M;

    case KEY_0: return Rml::Input::KI_0;
    case KEY_1: return Rml::Input::KI_1;
    case KEY_2: return Rml::Input::KI_2;
    case KEY_3: return Rml::Input::KI_3;
    case KEY_4: return Rml::Input::KI_4;
    case KEY_5: return Rml::Input::KI_5;
    case KEY_6: return Rml::Input::KI_6;
    case KEY_7: return Rml::Input::KI_7;
    case KEY_8: return Rml::Input::KI_8;
    case KEY_9: return Rml::Input::KI_9;

    case KEY_F1:  return Rml::Input::KI_F1;
    case KEY_F2:  return Rml::Input::KI_F2;
    case KEY_F3:  return Rml::Input::KI_F3;
    case KEY_F4:  return Rml::Input::KI_F4;
    case KEY_F5:  return Rml::Input::KI_F5;
    case KEY_F6:  return Rml::Input::KI_F6;
    case KEY_F7:  return Rml::Input::KI_F7;
    case KEY_F8:  return Rml::Input::KI_F8;
    case KEY_F9:  return Rml::Input::KI_F9;
    case KEY_F10: return Rml::Input::KI_F10;
    case KEY_F11: return Rml::Input::KI_F11;
    case KEY_F12: return Rml::Input::KI_F12;

    case KEY_ESCAPE:    return Rml::Input::KI_ESCAPE;
    case KEY_SPACE:     return Rml::Input::KI_SPACE;
    case KEY_ENTER:     return Rml::Input::KI_RETURN;
    case KEY_TAB:       return Rml::Input::KI_TAB;
    case KEY_BACKSPACE: return Rml::Input::KI_BACK;
    case KEY_DELETE:    return Rml::Input::KI_DELETE;
    case KEY_LEFT:      return Rml::Input::KI_LEFT;
    case KEY_RIGHT:     return Rml::Input::KI_RIGHT;
    case KEY_UP:        return Rml::Input::KI_UP;
    case KEY_DOWN:      return Rml::Input::KI_DOWN;

    case KEY_LSHIFT:   return Rml::Input::KI_LSHIFT;
    case KEY_RSHIFT:   return Rml::Input::KI_RSHIFT;
    case KEY_LCONTROL: return Rml::Input::KI_LCONTROL;
    case KEY_RCONTROL: return Rml::Input::KI_RCONTROL;
    case KEY_LALT:     return Rml::Input::KI_LMENU;
    case KEY_RALT:     return Rml::Input::KI_RMENU;

    case KEY_COMMA:      return Rml::Input::KI_OEM_COMMA;
    case KEY_PERIOD:     return Rml::Input::KI_OEM_PERIOD;
    case KEY_SLASH:      return Rml::Input::KI_OEM_2;
    case KEY_SEMICOLON:  return Rml::Input::KI_OEM_1;
    case KEY_APOSTROPHE: return Rml::Input::KI_OEM_7;
    case KEY_LBRACKET:   return Rml::Input::KI_OEM_4;
    case KEY_RBRACKET:   return Rml::Input::KI_OEM_6;
    case KEY_BACKSLASH:  return Rml::Input::KI_OEM_5;
    case KEY_MINUS:      return Rml::Input::KI_OEM_MINUS;
    case KEY_EQUAL:      return Rml::Input::KI_OEM_PLUS;

    default:
        return Rml::Input::KI_UNKNOWN;
    }
}

void CUISystem::Stop()
{
    if (g_pDefaultUIMaterial)
    {
        g_pDefaultUIMaterial->DecrementReferenceCount();
        g_pDefaultUIMaterial = nullptr;
    }

    for (int i = 0; i < GetTopPanel()->GetChildCount(); ++i)
    {
        // change it later, loading invalid document also closes previous
        GetTopPanel()->GetChild(i)->SetDocument("some non-existent file");
    }

    Rml::RemoveContext("UISystem");

    Rml::Shutdown();
}

IUIPanel* CUISystem::GetTopPanel()
{
    static CUIPanel topPanel;
    return (IUIPanel*)&topPanel;
}

IUIPanel* CUISystem::CreatePanel(IUIPanel* parent)
{
    IUIPanel* panel = new CUIPanel();
    panel->SetParent(parent);
    return panel;
}

void CUISystem::DestroyPanel(IUIPanel* parent)
{
    parent->SetDocument("some non-existent file");
    delete parent;
}

void CUISystem::LoadFont(const char* name)
{
    static const char* s_FontWeights[] = {
        "Thin",
        "ThinItalic",
        "ExtraLight",
        "ExtraLightItalic",
        "Light",
        "LightItalic",
        "Regular",
        "Italic",
        "Medium",
        "MediumItalic",
        "SemiBold",
        "SemiBoldItalic",
        "Bold",
        "BoldItalic",
        "ExtraBold",
        "ExtraBoldItalic"
    };

    char fontPathFull[256];

    for (int i = 0; i < ARRAYSIZE(s_FontWeights); ++i)
    {
        V_snprintf(fontPathFull, sizeof(fontPathFull), "fonts/%s-%s.ttf", name, s_FontWeights[i]);

        Rml::LoadFontFace(fontPathFull);
    }
}


bool CUISystem::InputHandleInputEvent(const InputEvent_t& event)
{
   //for (IRmlUIInputEventListener* listener : m_IEListeners)
    //{
    //    if (listener)
   //         listener->OnInputEvent(event);
   // }

    return DispatchEventToContext(*g_pGlobalContext, event);
}

bool CUISystem::DispatchEventToContext(Rml::Context& context, const InputEvent_t& event)
{
    switch (event.m_nType)
    {
    case IE_FirstVguiEvent + 3:
        return context.ProcessTextInput(static_cast<Rml::Character>(event.m_nData));

    case IE_ButtonPressed:
    {
        const ButtonCode_t code = static_cast<ButtonCode_t>(event.m_nData2);

        if (IsKeyCode(code))
            return context.ProcessKeyDown(ButtonCodeToRmlKeyID(code), 0);

        if (IsMouseCode(code))
            return context.ProcessMouseButtonDown(MouseIndexFromCode(code), 0);

        return false;
    }

    case IE_ButtonReleased:
    {
        const ButtonCode_t code = static_cast<ButtonCode_t>(event.m_nData2);

        if (IsKeyCode(code))
            return context.ProcessKeyUp(ButtonCodeToRmlKeyID(code), 0);

        if (IsMouseCode(code))
            return context.ProcessMouseButtonUp(MouseIndexFromCode(code), 0);

        return false;
    }

    case IE_FingerDown:
    {
        int x, y;
        if (!FingerToPixels(event, x, y))
            return false;

        context.ProcessMouseMove(x, y, 0);
        context.ProcessMouseButtonDown(0, 0);
        return true;
    }

    case IE_FingerUp:
    {
        int x, y;
        if (!FingerToPixels(event, x, y))
            return false;

        context.ProcessMouseMove(x, y, 0);
        context.ProcessMouseButtonUp(0, 0);
        return true;
    }

    case IE_FingerMotion:
    {
        int x, y;
        if (!FingerToPixels(event, x, y))
            return false;

        context.ProcessMouseMove(x, y, 0);
        return true;
    }

    case IE_AnalogValueChanged:
    {
        if (event.m_nData == MOUSE_WHEEL)
            return context.ProcessMouseWheel(static_cast<float>(-event.m_nData3), 0);

        if (event.m_nData == MOUSE_XY)
            return context.ProcessMouseMove(event.m_nData2, event.m_nData3, 0);

        return false;
    }

    default:
        return false;
    }
}
