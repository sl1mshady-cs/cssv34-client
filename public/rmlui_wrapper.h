#ifndef RMLUI_WRAPPER_H
#define RMLUI_WRAPPER_H

#if defined(_WIN32)
#pragma once
#endif

#include <RmlUi/Core.h>
#include <tier1/interface.h>
#include <inputsystem/InputEnums.h>

#define RMLUI_INTERFACE_VERSION "RmlUi002"

class IRmlUIInputEventListener
{
public:
	virtual void OnInputEvent(const InputEvent_t& event) = 0;
};

enum RmlUiContextSizing_e
{
	RMLUI_SIZE_FIXED = 0,
	RMLUI_SIZE_FULLSCREEN
};

enum RmlUiOrder_e
{
	RMLUI_ORDER_LOADINGDIALOG = 0,
	RMLUI_ORDER_MAINMENU = 50,
	RMLUI_ORDER_GAMEUI = 100,
	RMLUI_ORDER_HUD = 200/*,
	RMLUI_ORDER_TOUCH*/
};

class IRmlUiContext
{
public:
	virtual Rml::Context* GetRmlContext() = 0;
	virtual void SetVisible(bool state) = 0;
	virtual bool IsVisible() = 0;
	virtual RmlUiContextSizing_e GetSizing() = 0;
	virtual const char* GetName() = 0;
	virtual int GetOrder() = 0;
	virtual void SetSize(int w, int h) = 0;
	virtual void GetSize(int& w, int& h) = 0;
};

class IRmlUI
{
public:
	virtual bool Initialize(CreateInterfaceFn* factorylist, int nFactories) = 0;
	virtual void Draw() = 0;
	virtual void Shutdown() = 0;
	virtual void AddInputEventListener(IRmlUIInputEventListener* listener) = 0;
	virtual void RemoveInputEventListener(IRmlUIInputEventListener* listener) = 0;
	
	virtual IRmlUiContext* CreateContext(int order, const char* name, RmlUiContextSizing_e sizing = RMLUI_SIZE_FIXED, int width = 0, int height = 0) = 0;
	virtual void LoadFont(const char* name) = 0;
};

#endif