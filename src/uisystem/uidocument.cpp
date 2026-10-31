#include "uidocument_internal.h"

void CUIPanel::SetVisible(bool state)
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

	for (int i = 0; i < GetChildCount(); ++i)
	{
		GetChild(i)->SetVisible(state);
	}

	m_bIsVisible = state;
}

bool CUIPanel::IsVisible()
{
	return m_bIsVisible;
}

void CUIPanel::ToggleVisiblity()
{
	SetVisible(!IsVisible());
}

void CUIPanel::SetDocument(const char* doc)
{
	if (m_pDocument)
	{
		m_pDocument->Close();
		m_pDocument = nullptr;
	}

	m_pDocument = GetGlobalContext()->LoadDocument(doc);

	if (!m_pDocument)
	{
		Warning("failed to load document from %s\n", doc);
		return;
	}
}

void CUIPanel::RequestFocus()
{
	m_pDocument->Focus();
}

IUIPanel* CUIPanel::GetParent()
{
	return m_pParent;
}

void CUIPanel::SetParent(IUIPanel* parent)
{
	m_pParent = parent;
	if (parent)
		parent->AddChild(this);
}

int CUIPanel::GetChildCount()
{
	return m_Child.Count();
}

IUIPanel* CUIPanel::GetChild(int i)
{
	if (i < 0 || i >= GetChildCount())
	{
		return nullptr;
	}

	return m_Child[i];
}

void CUIPanel::AddChild(IUIPanel* child)
{
	m_Child.AddToTail(child);
}

void CUIPanel::RemoveChild(IUIPanel* child)
{
	m_Child.FindAndFastRemove(child);
}

void CUIPanel::Close()
{
	if (m_pDocument)
	{
		m_pDocument->Close();
		m_pDocument = nullptr;
	}

	for (int i = 0; i < GetChildCount(); ++i)
	{
		GetChild(i)->Close();
	}
}