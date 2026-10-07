//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "vgui_int.h"
#include "ienginevgui.h"
#include "c_csrootpanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_CSRootPanel *g_pCSRootPanel = NULL;
extern bool g_bVGUIDisabled;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_CreateClientDLLRootPanel( void )
{
	if (g_bVGUIDisabled) return;
	g_pCSRootPanel = new C_CSRootPanel( enginevgui->GetPanel( PANEL_CLIENTDLL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_DestroyClientDLLRootPanel( void )
{
	if (g_bVGUIDisabled) return;
	delete g_pCSRootPanel;
	g_pCSRootPanel = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Game specific root panel
// Output : vgui::Panel
//-----------------------------------------------------------------------------
vgui::VPANEL VGui_GetClientDLLRootPanel( void )
{
	if (g_bVGUIDisabled) return 0;
	return g_pCSRootPanel->GetVPanel();
}