#include <thread>
#include "servermaster.h"
#include "servermanager.h"

CServerMaster::CServerMaster(CServerManager* pServerManager, TMasterRequest* pMasterRequest)
{
	m_pServerManager = pServerManager;

	// socket
	m_pMasterSocket = new CSocket();

	std::thread thread(&CServerMaster::StartQuery, this, pMasterRequest);
	thread.detach();
}

CServerMaster::~CServerMaster(void)
{
	delete m_pMasterSocket;
	m_pServerManager->m_bIsDownloading = false;
}

// TODO: remove this? i think using bitbuf is better
char* CServerMaster::ConstructPacket(byte messageType,
	byte regionCode,
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
	memcpy(szData, &messageType, 1);
	// Region Code
	memcpy(szData + 1, &regionCode, 1);
	// IP Iterator
	memcpy(szData + 2, cszIPIterator, strlen(cszIPIterator) + 1);
	// Filter
	memcpy(szData + 2 + strlen(cszIPIterator) + 1, cszFilter, strlen(cszFilter) + 1);

	return szData;
}

void CServerMaster::StartQuery(TMasterRequest* pRequest)
{
	// Create Socket
	m_pMasterSocket->Open(0);
	if (!m_pMasterSocket->IsValid())
		return;

	// Construct Packet
	unsigned int uPacketSize = 0;
	char* szPacket = ConstructPacket(0x31,
		pRequest->regionCode,
		pRequest->szIPIterator,
		pRequest->szFilter,
		&uPacketSize);

	// Send Request
	if (m_pMasterSocket->Send(pRequest->masterAddress, szPacket, uPacketSize) == -1)
		return;

	Msg("Requested Server List...\n");

	// Delete Packet buffer
	delete szPacket;

	char recvBuffer[MAX_RECEIVEABLE_PACKET];
	bf_read read;

	TMasterReply masterReply;
	netadr_t* adrIterator = 0;

	// Receive List
	while (true)
	{
		netadr_t from;
		int iBytesReceived = m_pMasterSocket->Receive(recvBuffer, MAX_RECEIVEABLE_PACKET, from);

		Msg("Receive() returned %d from %s\n",
			iBytesReceived,
			from.ToString());

		if (iBytesReceived == -1)
			return;

		read.StartReading(recvBuffer, iBytesReceived);

		// Check if reply is correct
		int connectionlessHeader = read.ReadLong();
		if (connectionlessHeader != -1)
			break;

		short masterReplyHeader = read.ReadWord();
		// ok

		char* szReply = recvBuffer;

		// The Packet got more than 1 IP
		for (unsigned int i = 6; i < iBytesReceived; i += 6)
		{
			memcpy(&masterReply, szReply + i, sizeof(TMasterReply));

			// Format: IP:Port
			adrIterator = new netadr_t(
				ntohl(masterReply.unIP),
				ntohs(masterReply.usPort));

			// End of List?
			if (adrIterator->GetIPHostByteOrder() == 0 && adrIterator->GetPort() == 0)
				return;

			// Store IP
			m_pServerManager->m_vecServer.AddToTail(adrIterator);

			Msg("Got IP from master: %s\n", adrIterator->ToString());

			if (!m_pServerManager->m_bIsRefresh)
				break;
		}

		// Construct new Request
		szPacket = ConstructPacket(0x31,
			pRequest->regionCode,
			adrIterator->ToString(),
			pRequest->szFilter,
			&uPacketSize);

		// Send Request
		if (m_pMasterSocket->Send(pRequest->masterAddress, szPacket, uPacketSize) == -1)
			return;

		delete szPacket;
		if (!m_pServerManager->m_bIsRefresh)
			break;
	}

	Msg("FATAL & %d\n", m_pServerManager->m_bIsRefresh);
}