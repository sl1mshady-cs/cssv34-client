#include <rmlui_wrapper.h>
#include <rmlui_renderinterface.h>
#include <rmlui_fileinterface.h>
#include <rmlui_systeminterface.h>
#include <tier1/interface.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <materialsystem/imaterialsystem.h>
#include <materialsystem/imaterial.h>
#include <RmlUi/Core.h>
#include <RmlUi/Lua/Lua.h>
#include "inputsystem/iinputsystem.h"

struct RmlUiContext_t
{
    RmlUiContextSizing_e sizing;
    Rml::Context* context;
    int width;
    int order;
    int height;
};

class CRmlUiContext : public IRmlUiContext
{
public:
    CRmlUiContext(int order, const char* name, RmlUiContextSizing_e sizing, int width, int height)
    {
        m_Sizing = sizing;
        m_pName = name;
        m_Order = (RmlUiOrder_e)order;
        m_bVisible = true;

        if (sizing == RMLUI_SIZE_FIXED)
        {
            m_pContext = Rml::CreateContext(name, Rml::Vector2i(width, height));
        }
        else if (sizing == RMLUI_SIZE_FULLSCREEN)
        {
            g_pMaterialSystem->GetBackBufferDimensions(width, height);
            m_pContext = Rml::CreateContext(name, Rml::Vector2i(width, height));
        }
    }

    virtual void SetVisible(bool state)
    {
        m_bVisible = state;
    }

    virtual bool IsVisible()
    {
        return m_bVisible;
    }

    virtual RmlUiContextSizing_e GetSizing()
    {
        return m_Sizing;
    }

    virtual const char* GetName()
    {
        return m_pName;
    }

    virtual int GetOrder()
    {
        return m_Order;
    }

    virtual void SetSize(int w, int h)
    {
        m_pContext->SetDimensions(Rml::Vector2i(w, h));
    }

    virtual void GetSize(int& w, int& h)
    {
        Rml::Vector2i size = m_pContext->GetDimensions();
        w = size.x;
        h = size.y;
    }

    void Think()
    {
        m_pContext->Update();
    }

    void Render()
    {
        m_pContext->Render();
    }

    void SetDPRatio(float dp_ratio)
    {
        m_pContext->SetDensityIndependentPixelRatio(dp_ratio);
    }

    virtual Rml::Context* GetRmlContext()
    {
        return m_pContext;
    }
private:
    bool m_bVisible; // draw and process input
    RmlUiContextSizing_e m_Sizing;
    const char* m_pName;
    RmlUiOrder_e m_Order;
    Rml::Context* m_pContext;
};

class CRmlUI : public IRmlUI
{
public:
    virtual bool Initialize(CreateInterfaceFn* factorylist, int nFactories);
    virtual void Draw();
    virtual void Shutdown();
    virtual void AddInputEventListener(IRmlUIInputEventListener* listener);
    virtual void RemoveInputEventListener(IRmlUIInputEventListener* listener);

    virtual IRmlUiContext* CreateContext(int order, const char* name, RmlUiContextSizing_e sizing = RMLUI_SIZE_FIXED, int width = 0, int height = 0);
    virtual void LoadFont(const char* name);
private:
    int m_iLastWidth = 0;
    int m_iLastHeight = 0;
    int m_nLastInputPollCount = 0;
    CUtlVector<IRmlUIInputEventListener*> m_IEListeners;
    CUtlVector<IRmlUiContext*> m_Contexts;

    Rml::Input::KeyIdentifier ButtonCodeToRmlKeyID(ButtonCode_t code);
    bool InputHandleInputEvent(const InputEvent_t& event);
    bool DispatchEventToContext(Rml::Context& context, const InputEvent_t& event);
};

static bool s_bIsInitted = false;
IMaterial* g_pDefaultUIMaterial = nullptr;
CRmlUI rmlui;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CRmlUI, IRmlUI, RMLUI_INTERFACE_VERSION, rmlui);

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

bool CRmlUI::InputHandleInputEvent(const InputEvent_t& event)
{
    if (m_Contexts.Count() == 0)
        return false;

    for (IRmlUIInputEventListener* listener : m_IEListeners)
    {
        if (listener)
            listener->OnInputEvent(event);
    }

    FOR_EACH_VEC_BACK(m_Contexts, i)
    {
        Rml::Context* ctx = m_Contexts[i]->GetRmlContext();

        if (DispatchEventToContext(*ctx, event))
            return true;
    }

    return false;
}

bool CRmlUI::DispatchEventToContext(Rml::Context& context, const InputEvent_t& event)
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

bool CRmlUI::Initialize(CreateInterfaceFn* factorylist, int nFactories)
{
    if (s_bIsInitted)
    {
        return true;
    }
    s_bIsInitted = true;
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

    FOR_EACH_VEC(m_Contexts, i)
    {
        Rml::Context* ctx = m_Contexts[i]->GetRmlContext();
        ctx->SetDensityIndependentPixelRatio(scale);
    }

    return true;
}

void CRmlUI::Shutdown()
{
    s_bIsInitted = false;
    FOR_EACH_VEC(m_Contexts, i)
    {
        Rml::Context* ctx = m_Contexts[i]->GetRmlContext();
        Rml::RemoveContext(ctx->GetName());
    }
    m_Contexts.RemoveAll();

    if (g_pDefaultUIMaterial)
    {
        g_pDefaultUIMaterial->DecrementReferenceCount();
        g_pDefaultUIMaterial = nullptr;
    }

    Rml::Shutdown();
}

void CRmlUI::Draw()
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
        FOR_EACH_VEC(m_Contexts, i)
        {
            if (m_Contexts[i]->GetSizing() != RMLUI_SIZE_FULLSCREEN) continue;

            Rml::Context* ctx = m_Contexts[i]->GetRmlContext();
            ctx->SetDensityIndependentPixelRatio(scale);
            ctx->SetDimensions(Rml::Vector2i(nw, nh));
        }
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

    FOR_EACH_VEC(m_Contexts, i)
    {
        CRmlUiContext* ctx = (CRmlUiContext*)m_Contexts[i];
        if (ctx->IsVisible())
        {
            ctx->Think();
            ctx->Render();
        }
    }
}

Rml::Input::KeyIdentifier CRmlUI::ButtonCodeToRmlKeyID(ButtonCode_t code)
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

void CRmlUI::AddInputEventListener(IRmlUIInputEventListener* listener)
{
    if (m_IEListeners.Find(listener) != m_IEListeners.InvalidIndex())
    {
        return;
    }

    m_IEListeners.AddToTail(listener);
}

void CRmlUI::RemoveInputEventListener(IRmlUIInputEventListener* listener)
{
    m_IEListeners.FindAndFastRemove(listener);
}

IRmlUiContext* CRmlUI::CreateContext(int order, const char* name,
    RmlUiContextSizing_e sizing,
    int width, int height)
{
    CRmlUiContext* ctx = new CRmlUiContext(order, name, sizing, width, height);

    int insertAt = m_Contexts.Count();
    FOR_EACH_VEC(m_Contexts, i)
    {
        if (m_Contexts[i]->GetOrder() > ctx->GetOrder())
        {
            insertAt = i;
            break;
        }
    }

    m_Contexts.InsertBefore(insertAt, ctx);

    return ctx;
}

void CRmlUI::LoadFont(const char* name)
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