#include "uisystem_internal.h"
#include "uihelpers.h"
#include <cdll_int.h>
#include "mainmenu.h"

CUISystem uisys;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CUISystem, IUISystem, UISYSTEM_INTERFACE_VERSION, uisys);

IMaterial* g_pDefaultUIMaterial = nullptr;

IVEngineClient* engine = nullptr;

Rml::Context* g_pMainMenuCtx = nullptr;
Rml::Context* g_pLoadingDialogCtx = nullptr;
Rml::Context* g_pPauseMenuCtx = nullptr;
Rml::Context* g_pInGameCtx = nullptr;
Rml::Context* g_pDialogCtx = nullptr;

Rml::Context* CUISystem::StateToContext(UIState ctx)
{
    if (ctx == InMainMenu) return g_pMainMenuCtx;
    else if (ctx == InLoadingDialog) return g_pLoadingDialogCtx;
    else if (ctx == InPauseMenu) return g_pPauseMenuCtx;
    else if (ctx == InGame) return g_pInGameCtx;
    else return nullptr;
}

bool CUISystem::Start(CreateInterfaceFn* factorylist, int nFactories)
{
    m_CurrentUIState = InMainMenu;
	ConnectTier1Libraries(factorylist, nFactories);
	ConnectTier2Libraries(factorylist, nFactories);

    engine = (IVEngineClient*)(*factorylist)(VENGINE_CLIENT_INTERFACE_VERSION, nullptr);

	if (!g_pMaterialSystem || !g_pFullFileSystem || !g_pInputSystem || !engine)
	{
		Error("RmlUI can't get some required interfaces to function\n");
		return false;
	}

	g_pMaterialSystem->GetBackBufferDimensions(m_iLastWidth, m_iLastHeight);

	g_pDefaultUIMaterial = g_pMaterialSystem->FindMaterial("vgui/white", TEXTURE_GROUP_VGUI);
	if (g_pDefaultUIMaterial)
		g_pDefaultUIMaterial->IncrementReferenceCount();

    renderinterface.Initialize();

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

    g_pMainMenuCtx = Rml::CreateContext("Main Menu", Rml::Vector2i(m_iLastWidth, m_iLastHeight));
    g_pMainMenuCtx->SetDensityIndependentPixelRatio(scale);
    g_pMainMenuCtx->SetDimensions(Rml::Vector2i(m_iLastWidth, m_iLastHeight));

    g_pLoadingDialogCtx = Rml::CreateContext("Loading Dialog", Rml::Vector2i(m_iLastWidth, m_iLastHeight));
    g_pLoadingDialogCtx->SetDensityIndependentPixelRatio(scale);
    g_pLoadingDialogCtx->SetDimensions(Rml::Vector2i(m_iLastWidth, m_iLastHeight));

    g_pPauseMenuCtx = Rml::CreateContext("Pause Menu", Rml::Vector2i(m_iLastWidth, m_iLastHeight));
    g_pPauseMenuCtx->SetDensityIndependentPixelRatio(scale);
    g_pPauseMenuCtx->SetDimensions(Rml::Vector2i(m_iLastWidth, m_iLastHeight));

    g_pInGameCtx = Rml::CreateContext("InGame Menu", Rml::Vector2i(m_iLastWidth, m_iLastHeight));
    g_pInGameCtx->SetDensityIndependentPixelRatio(scale);
    g_pInGameCtx->SetDimensions(Rml::Vector2i(m_iLastWidth, m_iLastHeight));

    g_pDialogCtx = Rml::CreateContext("UISystem Dialogs", Rml::Vector2i(m_iLastWidth, m_iLastHeight));
    g_pDialogCtx->SetDensityIndependentPixelRatio(scale);
    g_pDialogCtx->SetDimensions(Rml::Vector2i(m_iLastWidth, m_iLastHeight));

    Rml::DataModelConstructor cons = g_pDialogCtx->CreateDataModel("uisystem");

    cons.Bind("title", &m_TitleStr);
    cons.Bind("contents", &m_ContentsStr);
    cons.Bind("result", &m_DialogResult);
    cons.Bind("show_ok_btn", &m_bDialogBtnOk);
    cons.Bind("show_cancel_btn", &m_bDialogBtnCancel);
    cons.Bind("show_abort_btn", &m_bDialogBtnAbort);
    cons.Bind("show_retry_btn", &m_bDialogBtnRetry);

    Rml::DataModelConstructor cons2 = StateToContext(InLoadingDialog)->CreateDataModel("loading_dialog");

    cons2.Bind("progress", &m_flProgress);
    cons2.Bind("progress_desc", &m_flProgressDesc);
    cons2.Bind("map", &m_strMapName);

    m_pLoadingDialogDoc = CreateDocument(InLoadingDialog);
    m_pLoadingDialogDoc->SetDocument("loading_dialog.rml");
    m_pLoadingDialogDoc->SetVisible(false);

    m_UISystemModel = cons.GetModelHandle();
    m_LoadingDialogModel = cons2.GetModelHandle();

    mainmenu.Init();

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

        StateToContext(InGame)->SetDensityIndependentPixelRatio(scale);
        StateToContext(InPauseMenu)->SetDensityIndependentPixelRatio(scale);
        StateToContext(InLoadingDialog)->SetDensityIndependentPixelRatio(scale);
        StateToContext(InMainMenu)->SetDensityIndependentPixelRatio(scale);
        g_pDialogCtx->SetDensityIndependentPixelRatio(scale);

        StateToContext(InGame)->SetDimensions(Rml::Vector2i(nw, nh));
        StateToContext(InPauseMenu)->SetDimensions(Rml::Vector2i(nw, nh));
        StateToContext(InLoadingDialog)->SetDimensions(Rml::Vector2i(nw, nh));
        StateToContext(InMainMenu)->SetDimensions(Rml::Vector2i(nw, nh));
        g_pDialogCtx->SetDimensions(Rml::Vector2i(nw, nh));
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

    mainmenu.Think();

    StateToContext(m_CurrentUIState)->Update();
    StateToContext(m_CurrentUIState)->Render();

    if (m_pDialogDoc != nullptr)
    {
        g_pDialogCtx->Update();
        g_pDialogCtx->Render();

        if (m_DialogResult != NoDialog && m_DialogResult != NoAction)
        {
            Msg("Dialog Result: %i\n", m_DialogResult);
            CloseDialog();
        }
    }
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
    mainmenu.Shutdown();

    if (g_pDefaultUIMaterial)
    {
        g_pDefaultUIMaterial->DecrementReferenceCount();
        g_pDefaultUIMaterial = nullptr;
    }

    Rml::RemoveContext(StateToContext(InGame)->GetName());
    Rml::RemoveContext(StateToContext(InMainMenu)->GetName());
    Rml::RemoveContext(StateToContext(InPauseMenu)->GetName());
    Rml::RemoveContext(StateToContext(InLoadingDialog)->GetName());
    Rml::RemoveContext(g_pDialogCtx->GetName());

    Rml::Shutdown();
}

IUIDocument* CUISystem::CreateDocument(UIState ctx)
{
    IUIDocument* doc = new CUIDocument(ctx);
    return doc;
}


IUIDocument* CUISystem::CreateDocument(Rml::Context* ctx)
{
    IUIDocument* doc = new CUIDocument(ctx);
    return doc;
}

void CUISystem::DestroyDocument(IUIDocument* doc)
{
    doc->Close();
    delete doc;
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
    
    if (m_pDialogDoc != nullptr)
    {
        return DispatchEventToContext(*g_pDialogCtx, event);
    }

    if (event.m_nType == IE_ButtonPressed)
    {
        if (event.m_nData == KEY_ESCAPE && engine->IsInGame())
        {
            if (m_CurrentUIState == InGame)
            {
                ChangeUIState(InPauseMenu);
            }
            else
            {
                ChangeUIState(InGame);
            }
        }
        else if (event.m_nData == KEY_BACKQUOTE)
        {

        }
    }

    return DispatchEventToContext(*StateToContext(m_CurrentUIState), event);
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

Rml::DataModelConstructor CUISystem::CreateDataModel(UIState ctx, const char* name)
{
    return StateToContext(ctx)->CreateDataModel(name);
}

void CUISystem::ChangeUIState(UIState state)
{
    Msg("ChangeUIState: %s -> %s\n",
        StateToContext(m_CurrentUIState)->GetName().c_str(),
        StateToContext(state)->GetName().c_str()
    );
    m_CurrentUIState = state;
}

UIState CUISystem::GetUIState()
{
    return m_CurrentUIState;
}

void CUISystem::ShowDialog(const char* title, const char* name, DialogButtons btns)
{
    if (!m_pDialogDoc)
    {
        m_pDialogDoc = CreateDocument(g_pDialogCtx);
        m_pDialogDoc->SetDocument("uisystemdialog.rml");
    }

    m_TitleStr = title;
    m_ContentsStr = name;
    m_bDialogBtnOk = (btns & DIALOG_BTN_OK);
    m_bDialogBtnCancel = (btns & DIALOG_BTN_CANCEL);
    m_bDialogBtnAbort = (btns & DIALOG_BTN_ABORT);
    m_bDialogBtnRetry = (btns & DIALOG_BTN_RETRY);
    m_DialogResult = NoAction;
    m_UISystemModel.DirtyAllVariables();

    m_pDialogDoc->SetVisible(true);
}

DialogResult CUISystem::GetDialogResult()
{
    DialogResult prev_result = m_DialogResult;

    m_DialogResult = NoAction;

    return prev_result;
}

void CUISystem::CloseDialog()
{
    m_pDialogDoc->Close();
    m_pDialogDoc = nullptr;
}

void CUISystem::OnLevelLoadingStarted(bool bShowProgressDialog)
{
    ChangeUIState(InLoadingDialog);
    m_flProgress = 0.0f;
    m_flProgressDesc = "";
    m_strMapName = "de_dust2";
    m_LoadingDialogModel.DirtyAllVariables();
    m_pLoadingDialogDoc->SetVisible(true);
}

void CUISystem::OnLevelLoadingFinished(bool bError, const char* failureReason, const char* extendedReason)
{
    if (bError)
    {
        ChangeUIState(InMainMenu);
        ShowDialog("#RmlUi_Load_Error", failureReason, DIALOG_BTN_OK);
        m_LoadingDialogModel.DirtyAllVariables();
    }
    else
    {
        ChangeUIState(InGame);
    }
    m_pLoadingDialogDoc->SetVisible(false);
}

void CUISystem::UpdateProgressBar(float flProgress, const char* desc)
{
    m_flProgress = flProgress;
    if (desc != nullptr) {
        m_flProgressDesc = desc;
    }
    m_LoadingDialogModel.DirtyAllVariables();
}

void CUISystem::OnDisconnectedFromServer()
{
    ChangeUIState(InMainMenu);
}