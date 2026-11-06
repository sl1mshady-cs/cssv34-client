#ifndef _CLIENTMOD_MGR_H
#define _CLIENTMOD_MGR_H

#ifdef _WIN32
#pragma once
#endif

#include "netmessages.h"
#include "tier1/bitbuf.h"

class IClientModManager 
{
public:
	// Check for exploits
	virtual bool CheckFragment(uint8 cmd, bf_read& buf, bf_read& fallback) = 0;

	// Add clientmod cvars to ConVar Update Message
	virtual void FillConVarUpdateMsg(NET_SetConVar* cvarMsg) = 0;

	// Fill clc_RespondCvarValue
	virtual void FillRespondCvarValue(SVC_GetCvarValue* inMsg, CLC_RespondCvarValue& returnMsg) = 0;
};

extern IClientModManager *g_pClientModManager;

#endif // _CLIENTMOD_MGR_H