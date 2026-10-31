#pragma once
#include "tier0/dbg.h"
#include "tier1/interface.h"

#define UISYSTEM_INTERFACE_VERSION "uimgr001"

class IUIPanel
{
public:
	virtual void SetVisible(bool state) = 0;
	virtual void ToggleVisiblity() = 0;
	virtual void SetDocument(const char* doc) = 0;
	virtual void RequestFocus() = 0;
	virtual IUIPanel* GetParent() = 0;
	virtual void SetParent(IUIPanel* parent) = 0;
	virtual int GetChildCount() = 0;
	virtual IUIPanel* GetChild(int i) = 0;
	virtual bool IsVisible() = 0;
	virtual void AddChild(IUIPanel* child) = 0;
	virtual void RemoveChild(IUIPanel* child) = 0;
	virtual void Close() = 0;
};

class IUISystem
{
public:
	virtual bool Start(CreateInterfaceFn* factorylist, int nFactories) = 0;
	virtual void Draw() = 0;
	virtual void Stop() = 0;
	virtual IUIPanel* GetTopPanel() = 0;
	virtual IUIPanel* CreatePanel(IUIPanel* parent) = 0;
	virtual void DestroyPanel(IUIPanel* parent) = 0;
};