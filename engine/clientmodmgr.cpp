#include "GameEventManager.h"
#include "net_ws_headers.h"
#include "clientmodmgr.h"
#include "server.h"

class CClientModManager : public IClientModManager
{
public:
	CClientModManager();
	~CClientModManager();

	// Check for exploits
	virtual bool CheckFragment(uint8 cmd, bf_read& buf, bf_read& fallback);

	// Add clientmod cvars to ConVar Update Message
	virtual void FillConVarUpdateMsg(NET_SetConVar* cvarMsg);

	// Fill clc_RespondCvarValue
	virtual void FillRespondCvarValue(SVC_GetCvarValue* inMsg, CLC_RespondCvarValue& returnMsg);

private:
	const char* client_major_version = "2.0";
	const char* client_version = "3.0.1.1943";
	const char* connect_method = "8";
};

ConVar cm_enabled("cm_enabled", "1", FCVAR_HIDDEN_FROM_SERVER, "Enable/Disable clientmod emulation");

static CClientModManager s_mgr;
IClientModManager* g_pClientModManager = (IClientModManager*)&s_mgr;

CClientModManager::CClientModManager() {
	// nothing to do here
}

CClientModManager::~CClientModManager() {
	// nothing to do here
}

// Check for exploits
bool CClientModManager::CheckFragment(uint8 cmd, bf_read& buf, bf_read& fallback) 
{
	if (cmd == svc_GameEvent)
	{
		int length = buf.ReadUBitLong(11);
		int eventid = buf.ReadUBitLong(MAX_EVENT_BITS);
		CGameEventDescriptor* descriptor = g_GameEventManager.GetEventDescriptor(eventid);
		const char* name = descriptor->name;

		DevMsg("svc_GameEvent: %s (%d)\n", name, eventid);

		if (name && !strcmp(name, "player_disconnect"))
		{
			short userid = buf.ReadWord();
			char reason[1024];
			buf.ReadString(reason, sizeof(reason));
			char name[1024];
			buf.ReadString(name, sizeof(name));
			char networkid[1024];
			buf.ReadString(networkid, sizeof(networkid));
			//DevMsg("player_disconnect %d name %s reason %s networkid %s\n", userid, name, reason, networkid);

			char name_low[1024];
			V_strcpy_safe(name_low, name);
			V_strlower(name_low);

			if (userid < 1 || strstr(name_low, "unconnected"))
				return false;
		}

		//if (name && !strcmp(name, "player_info"))
		//{
		//	char databuf[1024];
		//	buf.ReadString(databuf, sizeof(databuf));
//
		//	if (strstr(databuf, "{}") && strstr(databuf, "?"))
			{
		//		//DevMsg("player_info buffer %s\n", databuf);
		//		buf.ReadString(databuf, sizeof(databuf));
		//		return false;
		//	}
		//}

		buf = fallback;
	}

	if (cmd == svc_UserMessage)
	{
		auto msgType = buf.ReadByte();
		auto dataLengthInBits = buf.ReadUBitLong(11);
		assert(math::BitsToBytes(data->dataLengthInBits) <= MAX_USER_MSG_DATA);
		char databuf[1024];
		buf.ReadBits(databuf, dataLengthInBits);

		if (msgType < 0)
		{
			DevMsg("UserMsg Rejected: type %d dataLengthInBits %d\n", msgType, dataLengthInBits);
			return false;
		}

		buf = fallback;
	}

	if (!sv.IsActive())
	{
		if (cmd == svc_Menu)
		{
			short Type = (short)buf.ReadUBitLong(16);
			auto dataLength = buf.ReadUBitLong(16);
			char databuf[4096];
			buf.ReadBytes(databuf, dataLength);
			DevMsg("svc_Menu Rejected: type %d dataLength %d\n", Type, dataLength);
			return false;
		}
	}

	return true;
}

// Add clientmod cvars to ConVar Update Message
void CClientModManager::FillConVarUpdateMsg(NET_SetConVar* cvarMsg) 
{
	if (!cm_enabled.GetBool())
		return;

	// Hardcoded clientmod cvars
	NET_SetConVar::cvar_t cmcvar;

	Q_strncpy(cmcvar.name, "~clientmod", MAX_OSPATH);
	Q_strncpy(cmcvar.value, client_major_version, MAX_OSPATH);
	cvarMsg->m_ConVars.AddToTail(cmcvar);

	Q_strncpy(cmcvar.name, "_client_version", MAX_OSPATH);
	Q_strncpy(cmcvar.value, client_version, MAX_OSPATH);
	cvarMsg->m_ConVars.AddToTail(cmcvar);

	Q_strncpy(cmcvar.name, "_connectmethod", MAX_OSPATH);
	Q_strncpy(cmcvar.value, connect_method, MAX_OSPATH);
	cvarMsg->m_ConVars.AddToTail(cmcvar);
}

// Fill clc_RespondCvarValue
void CClientModManager::FillRespondCvarValue(SVC_GetCvarValue* inMsg, CLC_RespondCvarValue& returnMsg) 
{
	if (!cm_enabled.GetBool())
		return;

	if (!strcmp(inMsg->m_szCvarName, "~clientmod"))
	{
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_ValueIntact;
		returnMsg.m_szCvarValue = client_major_version;
	}
	else if (!strcmp(inMsg->m_szCvarName, "_client_version"))
	{
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_ValueIntact;
		returnMsg.m_szCvarValue = client_version;
	}
	else if (!strcmp(inMsg->m_szCvarName, "_connectmethod"))
	{
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_ValueIntact;
		returnMsg.m_szCvarValue = connect_method;
	}
}