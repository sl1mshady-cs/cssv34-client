#include <client_pch.h>

#ifdef _WIN32
#include <windows.h>
#undef SetPort
#undef CreateEvent
#else
#include <unistd.h>
#endif

#include <clientticketmgr.h>

DEFINE_LOGGING_CHANNEL_NO_TAGS(LOG_CLTMGR, "CLTMgr", 0, LS_MESSAGE, Color(255, 150, 150, 255));

// RevEmu3 Ticket
struct ticketdata_t
{
	uint32 type;
	uint32 hash;
	uint32 magic;
	uint32 zero;
#if 1
	uint32 steamidlow;
	uint32 steamidhigh;
#else
	uint64 steamid;
#endif
	char hwid[127];
};
#define TICKET_SIZE sizeof(ticketdata_t)

uint32 JSHash(const uint8* data, int size)
{
	uint32 hash = 1315423911u;

	for (int i = 0; i < size; ++i)
	{
		hash ^= (hash << 5) + (hash >> 2) + data[i];
	}

	return hash;
}

unsigned int get_host_identifier()
{
#ifdef _WIN32
	DWORD serialNumber = 0;
	GetVolumeInformationA("C:\\", NULL, 0, &serialNumber, NULL, NULL, NULL, 0);
	return serialNumber;
#else
	unsigned int id = 0;
	char hostname[256] = { 0 };

	if (gethostname(hostname, sizeof(hostname)) == 0)
	{
		return JSHash(hostname, strlen(hostname));
	}
#endif
	return 0;
}

//todo: make validator, add more tickets support
class CTicket : public ITicketMgr
{
public:
	virtual void Init();

	virtual void Shutdown();

	virtual void WriteTicket(bf_write& buf);
private:
	ticketdata_t m_Ticket;

	uint32 m_nHostIdentifier;
	uint32 m_nSteamID;
};

static CTicket tmgr;
ITicketMgr* ticketmgr = &tmgr;

void CTicket::Init()
{
	m_nHostIdentifier = get_host_identifier();

	char id[16];
	snprintf(id, sizeof(id), "%u", m_nHostIdentifier);

	m_nSteamID = JSHash((const uint8*)id, strlen(id));

	CSteamID steamID(m_nSteamID * 2, k_EUniversePublic, k_EAccountTypeIndividual);

	Log_Msg(LOG_CLTMGR, "Your SteamID: %s\n", steamID.RenderAsSteam2String());

	memset(&m_Ticket, 0, TICKET_SIZE);
	m_Ticket.type = 'J';
	m_Ticket.magic = 'rev';
	m_Ticket.zero = 0;

	m_Ticket.hash = m_nSteamID;
	m_Ticket.steamidlow = m_nSteamID << 1;
	m_Ticket.steamidhigh = 0x01100001;

	snprintf(m_Ticket.hwid, sizeof(m_Ticket.hwid), "%u", m_nHostIdentifier);
}

void CTicket::Shutdown()
{
}

void CTicket::WriteTicket(bf_write& buf)
{
	buf.WriteShort(TICKET_SIZE);
	if (TICKET_SIZE > 0)
	{
		buf.WriteBytes(&m_Ticket, TICKET_SIZE);
	}
}