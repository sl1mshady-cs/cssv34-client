#include "uidocument_internal.h"
#include "uisystem_internal.h"

CUIDocument::CUIDocument(UIState ctx)
{
	m_bIsVisible = false;
	m_pDocument = nullptr;
	m_pContext = uisys.StateToContext(ctx);
}

CUIDocument::CUIDocument(Rml::Context* ctx)
{
	m_bIsVisible = false;
	m_pDocument = nullptr;
	m_pContext = ctx; 
}

void CUIDocument::SetVisible(bool state)
{
	if (m_pDocument == nullptr)
	{
		Warning("Tried to SetVisible() on uninitialized panel\n");
		return;
	}

	if (m_bIsVisible == state)
		return;

	if (state)
		m_pDocument->Show();
	else
		m_pDocument->Hide();

	m_bIsVisible = state;
}

bool CUIDocument::IsVisible()
{
	return m_bIsVisible;
}

void CUIDocument::ToggleVisiblity()
{
	SetVisible(!IsVisible());
}

void CUIDocument::SetDocument(const char* doc)
{
	if (m_pDocument)
	{
		m_pDocument->Close();
		m_pDocument = nullptr;
	}

	m_pDocument = m_pContext->LoadDocument(doc);

	if (!m_pDocument)
	{
		Warning("Failed to load document from %s\n", doc);
		return;
	}
}

void CUIDocument::Close()
{
	if (m_pDocument)
	{
		m_pDocument->Close();
		m_pDocument = nullptr;
	}
}