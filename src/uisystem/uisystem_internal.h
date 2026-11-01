#pragma once
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
    virtual IUIDocument* CreateDocument(UIState ctx);
    virtual IUIDocument* CreateDocument(Rml::Context* ctx);
    virtual void DestroyDocument(IUIDocument* doc);
    virtual Rml::DataModelConstructor CreateDataModel(UIState ctx, const char* name);

    virtual void ChangeUIState(UIState state);
    virtual UIState GetUIState();

    virtual void ShowDialog(const char* title, const char* name, DialogButtons btns);
    virtual DialogResult GetDialogResult();
    virtual void CloseDialog();

    virtual void OnLevelLoadingStarted(bool bShowProgressDialog);
    virtual void OnLevelLoadingFinished(bool bError, const char* failureReason, const char* extendedReason);
    virtual void UpdateProgressBar(float flProgress, const char* desc);
    virtual void OnDisconnectedFromServer();

    virtual Rml::Context* StateToContext(UIState state);
private:
    void LoadFont(const char* name);
    int m_iLastWidth;
    int m_iLastHeight;
    int m_nLastInputPollCount = 0;

    UIState m_CurrentUIState;

    IUIDocument* m_pDialogDoc = nullptr;
    IUIDocument* m_pLoadingDialogDoc = nullptr;

    Rml::DataModelHandle m_UISystemModel;
    Rml::DataModelHandle m_LoadingDialogModel;

    std::string m_TitleStr;
    std::string m_ContentsStr;
    DialogResult m_DialogResult = NoDialog;
    bool m_bDialogBtnOk = false;
    bool m_bDialogBtnCancel = false;
    bool m_bDialogBtnAbort = false;
    bool m_bDialogBtnRetry = false;

    float m_flProgress;
    std::string m_flProgressDesc;
    std::string m_strMapName;

    Rml::Input::KeyIdentifier ButtonCodeToRmlKeyID(ButtonCode_t code);
    bool InputHandleInputEvent(const InputEvent_t& event);
    bool DispatchEventToContext(Rml::Context& context, const InputEvent_t& event);
};

extern CUISystem uisys;