#pragma once
#include "uisystem_internal.h"
#include "tier1/utlvector.h"
#include <RmlUi/Core.h>

class CUIDocument : public IUIDocument
{
public:
	CUIDocument(UIState ctx);
	CUIDocument(Rml::Context* ctx);
	virtual void SetVisible(bool state);
	virtual void ToggleVisiblity();
	virtual void SetDocument(const char* doc);
	virtual bool IsVisible();
	virtual void Close();
private:
	bool m_bIsVisible;
	Rml::ElementDocument* m_pDocument;
	Rml::Context* m_pContext;
};
