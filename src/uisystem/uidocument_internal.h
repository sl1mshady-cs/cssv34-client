#include "uisystem.h"
#include "tier1/utlvector.h"
#include <RmlUi/Core.h>

class CUIPanel : public IUIPanel
{
public:
	CUIPanel() { m_bIsVisible = false; m_pParent = nullptr; m_pDocument = nullptr; }
	virtual void SetVisible(bool state);
	virtual void ToggleVisiblity();
	virtual void SetDocument(const char* doc);
	virtual void RequestFocus();
	virtual IUIPanel* GetParent();
	virtual void SetParent(IUIPanel* parent);
	virtual int GetChildCount();
	virtual IUIPanel* GetChild(int i);
	virtual bool IsVisible();
	virtual void AddChild(IUIPanel* child);
	virtual void RemoveChild(IUIPanel* child);
	virtual void Close();
private:
	bool m_bIsVisible;
	IUIPanel* m_pParent;
	CUtlVector<IUIPanel*> m_Child;
	Rml::ElementDocument* m_pDocument;
};

extern Rml::Context* GetGlobalContext();