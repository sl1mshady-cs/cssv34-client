#pragma once
#include "ServerManager.h"

struct TServerRefresh {
	ISteamMatchmakingPingResponse * pResponse;
	void *				pServerInfo;		// Reference to ServerManager
	char *				szAddress;			// Server Address
	unsigned short		sPort;				// Server Port
};

struct TPlayerQuery {
	ISteamMatchmakingPlayersResponse * pResponse;
	void *				pPlayerInfo;		// Reference to CPlayerInfo
	char *				szAddress;		// Server Address
	unsigned short		sPort;				// Server Port
};

class CServerManager;
class CServerMaster;

//
//
class CServerInfo
{
private:
	CServerManager * m_pServerManager;
	int m_sckQuery;

public:
	CServerInfo(CServerManager * pServerManager, 
		const char * cszIp, 
		unsigned short sPort,
		ISteamMatchmakingPingResponse * pResponse = NULL);

	~CServerInfo(void);
public:
	static void		RefreshServer(void * pServer);
	static byte		ReadByte(char ** szData);
	static char *	ReadString(char ** szData);
	static int		ReadInt32(char ** szData);
	static short	ReadShort(char ** szData);
	static bool		ReadBool(char ** szData);
	static float	ReadFloat(char ** szData);

public:
	void GetServerInfo(const char * cszAddress, 
		unsigned short sPort,
		ISteamMatchmakingPingResponse * pResponse);

};

//
//
class CServerInfoPlayers
{
private:
	SOCKET m_sckQuery;

public:
	CServerInfoPlayers(const char * cszIP, 
		unsigned short sPort, 
		ISteamMatchmakingPlayersResponse * pResponse);
	~CServerInfoPlayers();

private:
	static void		GetPlayerInfo( void * pQuery );
	unsigned int	GetChallenge(SOCKADDR_IN sckAddress);

public:
	void GetPlayers(const char * cszAddress, 
		unsigned short sPort,
		ISteamMatchmakingPlayersResponse * pResponse);
};