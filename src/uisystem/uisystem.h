#pragma once
#include "tier0/dbg.h"
#include "tier1/interface.h"
#include <RmlUi/Core.h>

#define UISYSTEM_INTERFACE_VERSION "uimgr001"

class IUIDocument
{
public:
	virtual void SetVisible(bool state) = 0;
	virtual void ToggleVisiblity() = 0;
	virtual void SetDocument(const char* doc) = 0;
	virtual bool IsVisible() = 0;
	virtual void Close() = 0;
};

enum UIState
{
	None = 0,
	InMainMenu,
	InLoadingDialog,
	InPauseMenu,
	InGame
};

enum DialogResult
{
	NoDialog = 0,
	NoAction,
	OK,
	Cancel,
	Abort,
	Retry
};

typedef int DialogButtons;
#define DIALOG_BTN_OK 0x1
#define DIALOG_BTN_CANCEL 0x2
#define DIALOG_BTN_ABORT 0x4
#define DIALOG_BTN_RETRY 0x8

class IUISystem
{
public:
	virtual bool Start(CreateInterfaceFn* factorylist, int nFactories) = 0;
	virtual void Draw() = 0;
	virtual void Stop() = 0;
	virtual IUIDocument* CreateDocument(UIState ctx) = 0;
	virtual IUIDocument* CreateDocument(Rml::Context* ctx) = 0;
	virtual void DestroyDocument(IUIDocument* doc) = 0;
	virtual Rml::DataModelConstructor CreateDataModel(UIState ctx, const char* name) = 0;

	virtual void ChangeUIState(UIState state) = 0;
	virtual UIState GetUIState() = 0;

	virtual void ShowDialog(const char* title, const char* name, DialogButtons btns) = 0;
	virtual DialogResult GetDialogResult() = 0;
	virtual void CloseDialog() = 0;

	virtual void OnLevelLoadingStarted(bool bShowProgressDialog = false) = 0;
	virtual void OnLevelLoadingFinished(bool bError, const char* failureReason, const char* extendedReason) = 0;
	virtual void UpdateProgressBar(float flProgress, const char* desc) = 0;

	virtual void OnDisconnectedFromServer() = 0;
};