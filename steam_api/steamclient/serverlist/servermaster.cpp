#include <thread>
#include "servermaster.h"
#include "servermanager.h"


CServerMaster::CServerMaster(CServerManager* pServerManager, TMasterRequest* pMasterRequest)
{
	m_pServerManager = pServerManager;

	// initiliaze TServerMaster
	TServerMaster* pServerMaster = new TServerMaster;

	pServerMaster->pServerMaster = this;
	pServerMaster->pMasterRequest = pMasterRequest;

	m_pServerManager->m_bIsDownloading = true;
	m_sckMaster = INVALID_SOCKET;

	std::thread thread(CServerMaster::QueryThread, pServerMaster);
	thread.detach();
}

CServerMaster::~CServerMaster(void)
{
	if (m_sckMaster != INVALID_SOCKET)
		closesocket(m_sckMaster);

	m_pServerManager->m_bIsDownloading = false;
}

void CServerMaster::QueryThread(void* pQuery)
{
	TServerMaster* pQueryThread = (TServerMaster*)pQuery;
	if (!pQueryThread)
		return;

	CServerMaster* pServerMaster = (CServerMaster*)pQueryThread->pServerMaster;
	if (!pServerMaster)
		return;

	TMasterRequest* pRequest = (TMasterRequest*)pQueryThread->pMasterRequest;
	if (!pRequest)
		return;

	// Get Server Info
	pServerMaster->StartQuery(pRequest);

	delete pQueryThread;
	delete pServerMaster;
}

char* CServerMaster::ConstructPacket(byte bytMessageType,
	byte bytRegionCode,
	const char* cszIPIterator,
	const char* cszFilter,
	unsigned int* uPacketSize
)
{
	// Length = MessageType + RegionCode + IP Iterator + Filter
	*uPacketSize = sizeof(byte) + sizeof(byte) +
		(strlen(cszIPIterator) + 1) + (strlen(cszFilter) + 1);

	char* szData = new char[*uPacketSize];
	memset(szData, 0, sizeof(szData));

	// MessageType
	memcpy(szData, &bytMessageType, 1);
	// Region Code
	memcpy(szData + 1, &bytRegionCode, 1);
	// IP Iterator
	memcpy(szData + 2, cszIPIterator, strlen(cszIPIterator) + 1);
	// Filter
	memcpy(szData + 2 + strlen(cszIPIterator) + 1, cszFilter, strlen(cszFilter) + 1);

	return szData;
}

void CServerMaster::StartQuery(TMasterRequest* pRequest)
{
	SOCKADDR_IN		sckAddress;
	int				iRemoteAddrSize = sizeof(SOCKADDR_IN);
	unsigned int	uBytesReceived = 0;
	char			szData[NET_UDP_RECVSIZE];
	char			szIPIterator[22];
	char			szIP[19];
	TMasterReply	MasterReply;

	// Create Socket
	m_sckMaster = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_sckMaster == INVALID_SOCKET)
		return;

	// Set Connection
	if (!m_pServerManager->SetConnectionInfo(sckAddress,
		pRequest->masterAddress))
		return;

	// Construct Packet
	unsigned int uPacketSize = 0;
	char* szPacket = ConstructPacket(0x31,
		pRequest->bytRegionCode,
		pRequest->szIPIterator,
		pRequest->szFilter,
		&uPacketSize);

	// Send Request
	if (!sendto(m_sckMaster, szPacket, uPacketSize, 0, (sockaddr*)&sckAddress, sizeof(sckAddress)))
		return;

	Msg("Requested Server List...\n");

	// Delete Packet buffer
	delete szPacket;

	// Receive List
	while (true)
	{
		uBytesReceived = recvfrom(m_sckMaster,
			(char*)&szData,
			NET_UDP_RECVSIZE,
			0,
			(sockaddr*)&sckAddress,
			&iRemoteAddrSize);

		if (!(uBytesReceived > 0))
			return;

		// Check if reply is correct
		memcpy(&MasterReply, szData, sizeof(TMasterReply));
		if (!(MasterReply.bytOctet1 == 0xFF &&
			MasterReply.bytOctet2 == 0xFF &&
			MasterReply.bytOctet3 == 0xFF &&
			MasterReply.bytOctet4 == 0xFF))
			return;

		char* szReply = (char*)&szData;

		// The Packet got more than 1 IP
		for (unsigned int i = 6; i < uBytesReceived; i += 6)
		{
			memcpy(&MasterReply, szReply + i, sizeof(TMasterReply));

			// Format: IP
			sprintf(szIP, "%d.%d.%d.%d", MasterReply.bytOctet1,
				MasterReply.bytOctet2,
				MasterReply.bytOctet3,
				MasterReply.bytOctet4);

			// Format: IP:Port
			sprintf(szIPIterator, "%s:%u", szIP, ntohs(MasterReply.sPort));
			//printf("%s\n", szIPIterator);

			// End of List?
			if (!strcmp(szIPIterator, "0.0.0.0:0"))
				return;

			// Store IP
			TServerIP* pIP = new TServerIP;
			pIP->sPort = ntohs(MasterReply.sPort);
			strcpy(pIP->szIP, szIP);
			m_pServerManager->m_vecServer.AddToTail(pIP);

			if (!m_pServerManager->m_bIsRefresh)
				return;
		}

		// Construct new Request
		szPacket = ConstructPacket(0x31,
			pRequest->bytRegionCode,
			szIPIterator,
			pRequest->szFilter,
			&uPacketSize);

		// Send Request
		if (!sendto(m_sckMaster, szPacket, uPacketSize, 0, (sockaddr*)&sckAddress, sizeof(sckAddress)))
			return;

		delete szPacket;
		if (!m_pServerManager->m_bIsRefresh)
			return;
	}

}