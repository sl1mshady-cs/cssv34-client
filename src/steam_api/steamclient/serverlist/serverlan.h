#pragma once
#include "servermanager.h"

struct TServerRefreshLan {
	void *				pServerInfo;		// Reference to ServerManager
	unsigned short		sPort;				// Server Port
};

class CServerLan
{
private:
	CServerManager * m_pServerManager;
	int m_sckQuery;

public:
	CServerLan(CServerManager * pManager, unsigned short sPort);
	~CServerLan(void);

public:
	static void		RefreshServer(void * pServer);
	static byte		ReadByte(char ** szData);
	static char *	ReadString(char ** szData);
	static int		ReadInt32(char ** szData);
	static short	ReadShort(char ** szData);
	static bool		ReadBool(char ** szData);
	static float	ReadFloat(char ** szData);

public:
	void GetLanIP(unsigned short sPort);
};
