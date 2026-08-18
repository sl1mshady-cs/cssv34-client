#pragma once
#include <mutex>
#include <string>
#include <vector>
#include "socket.h"
#include "tier1/utlvector.h"
#include "steam/steam_api.h"

class CServerList;

#define ADDFILTER(name, val) CMasterServer::AddFilterTable(name, val)
#define MAX_QUERYTHREADS 200

typedef void(__cdecl* cbStatus)(gameserveritem_t*);
typedef void(__cdecl* cbComplete)(void);

enum EServerRegion 
{
	eServerRegionUSEast = 0,
	eServerRegionUSWest,
	eServerRegionSouthAmerica,
	eServerRegionEurope,
	eServerRegionAsia,
	eServerRegionAustralia,
	eServerRegionMiddleEast,
	eServerRegionAfrica,
	eServerRegionOther = 0xFF
};

struct TMasterQueryRules {
	std::string			strAddress;		// Address of master Server
	short				sPortNumber;	// Ip of master Server
	byte				bRegion;		// Region Code
	char* strIPIterator;	// Used to send for latest IP
	std::string			strFilter;		// Filter rules
};

struct TMasterServerList {
	char* szAddress;		// Server address
	unsigned short		sPortNumber;	// Server port
};

struct TServerHandle2 {
	unsigned int		ServerIterator;
	char* szServerIp;
	unsigned short		sServerPort;
	bool				bAllowRefresh;
};

#pragma pack(push)	// Change alignment
#pragma pack(1) 
struct TMasterServerQueryReponse {
	byte				bIpOctet1;		// First octet of IP
	byte				bIpOctet2;		// Second octet of IP
	byte				bIpOctet3;		// Third octet of IP
	byte				bIpOctet4;		// Fourth octet of IP
	unsigned short		sPortNumber;	// Port number (2 byte)
};
#pragma pack(pop)

struct TServerQueryResponse {
	// To Do
};

class CMasterServer
{
public:
	CMasterServer(const CMasterServer& masterServer, CServerList* pList);
	~CMasterServer(void);

private:
	static void				NextRefresh(void* pMasterClass);
	static void				RefreshServerList(void* pCallback);
	static char* ReadString(char* szData, unsigned int* uRead = NULL);
	static void				ServerInfoResponse(void* pArgs);
	static void				GetServerIpList(void* pArgs);
	static void				SetConnectionInfo(sockaddr_in& sckAddrIn, const char* cszAddress, unsigned int uPort);
	void					StartSocket(void);
	void					StopSocket(void);

public:
	static std::string		AddFilterTable(std::string strName, std::string strValue);

public:
	// Master Server
	void					QueryServerList(TMasterQueryRules* pQueryRules);
	void					ClearServerList(void);

public:
	// Server Query
	TServerHandle2* GetFirstServer(void);
	bool					GetNextServer(TServerHandle2* pServerHandle);
	void					GetCloseServer(TServerHandle2* pServerHandle);

	// Query Info
	void					GetServerInfo(TServerHandle2* pServerHandle/*, void * pCallbackFunction*/);

public:
	void					RunFrame();
	bool					StartRefresh(/*void * pCallbackStatus, void * pCallbackComplete*/);
	bool					StopRefresh();
	bool					IsRefreshing();

private:
	bool m_bResponse;
	CUtlVector<gameserveritem_t*> ServerItems;
	int sckMaster;
	CServerList* m_pServerList;

public:
	std::vector<TMasterServerList*> ServerList;
	bool m_bIsDownloadingList;
	bool m_bIsRefresh;
	unsigned int m_uServerCountListed;
	unsigned int m_uActiveThreads;
	std::mutex m_Critical;
	TServerHandle2* m_pHandle;

};