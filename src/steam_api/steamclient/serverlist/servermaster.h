#ifndef _SERVERMASTER_H
#define _SERVERMASTER_H

#ifdef _WIN32
#pragma once
#endif

#include "socket.h"
#include "tier0/platform.h"

// forward declarations
struct TMasterRequest;
class CServerManager;
class CServerInfo;

/*
* Server master class
*/
class CServerMaster
{

public:
	CServerMaster(CServerManager* pServerManager, TMasterRequest* pMasterRequest);
	~CServerMaster(void);

	void StartQuery(TMasterRequest* pRequest);

private:
	char* ConstructPacket(byte messageType,
		byte regionCode,
		const char* cszIPIterator,
		const char* cszFilter,
		unsigned int* uPacketSize);

private:
	CServerManager* m_pServerManager;
	CSocket* m_pMasterSocket;
};

#endif // _SERVERMASTER_H