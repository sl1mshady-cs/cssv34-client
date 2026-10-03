#include "steamclient/serverlist/socket.h"
#include "convar.h"
#include "tier0/dbg.h"
#include "callback_system.h"
#include "logging.h"
#include "steamclient.h"
#include "auth.h"

extern CLoggingFile* Logger;
extern bool g_bLogging;

extern CSteamID g_uSteamID;
extern char g_szHostID[128];

// TODO: add rev.ini support
bool g_bAllowNonRev = false;
bool g_bAllowLegacyRev = false;
bool g_bAllowRevEmu2 = true;
bool g_bAllowRevEmu3 = true;
bool g_bAllowSteamEmu = false;

DEFINE_LOGGING_CHANNEL_NO_TAGS(LOG_AUTH, "rev-auth", 0, LS_MESSAGE, Color(250, 161, 92, 255));

#define PRINT_DEBUG(msg, ...) Logger->Write("[AuthSystem] " msg "\n", __VA_ARGS__)
#define PRINT_MSG(msg, ...) Log_Msg(LOG_AUTH, msg "\n", __VA_ARGS__)

// anything lower than that can only be achieved via spoofing
#define MIN_ALLOWED_ACCOUNT_ID 3072


namespace auth
{


/*
* ERevClientType -> string
*/
const char* GetClientTypeString(EClientType type)
{
	const char* clientType = 0;

	switch (type)
	{
	case eClientLegacyRev:
		clientType = "Very old Rev Emu";
		break;
	case eClientRevEmu2:
		clientType = "Old Rev Emu";
		break;
	case eClientRevEmu3:
		clientType = "Old Rev Emu v74";
		break;
	case eClientRevEmu4:
		clientType = "Rev Emu";
		break;
	case eClientSteamEmu:
		clientType = "Steam Emu";
		break;
	case eClientUnknown:
		clientType = "Unknown";
		break;
	}

	return clientType;
}

const char* GetAuthStatusString(EAuthStatus status)
{
	const char* authStatus = 0;

	switch (status)
	{
	case eAuthStatusOK:
	{
		authStatus = "eAuthStatusOK";
		break;
	}
	case eAuthStatus_CorruptedTicket:
	{
		authStatus = "eAuthStatus_CorruptedTicket";
		break;
	}
	case eAuthStatus_RejectedByGameServerPolicy:
	{
		authStatus = "eAuthStatus_RejectedByGameServerPolicy";
		break;
	}
	case eAuthStatus_TicketCorruptHWID:
	{
		authStatus = "eAuthStatus_TicketCorruptHWID";
		break;
	}
	case eAuthStatus_TicketCorruptSTEAMID:
	{
		authStatus = "eAuthStatus_TicketCorruptSTEAMID";
		break;
	}
	case eAuthStatus_TicketCorruptHASH:
	{
		authStatus = "eAuthStatus_TicketCorruptHASH";
		break;
	}
	default:
		break;
	}

	return authStatus;
}

/*
* helper log function for logging stats
*/
void CAuthSystem::LogStats(bool bConnecting, bool bDisconnecting, TRevUserValidationHandle* handle)
{
	const char* clientIP = inet_ntoa(*(in_addr*)&handle->uClientIP);
	const char* steamID = handle->uSteamID.RenderAsSteam2String();
	const char* clientType = GetClientTypeString(handle->eClientType);

	if (bConnecting)
		PRINT_DEBUG("Ticket: %s.", clientType);

	if (bConnecting)
		PRINT_DEBUG("UserConnect IP = %s | SteamID = %s", clientIP, steamID);
	else if (bDisconnecting)
		PRINT_DEBUG("SteamDisconnect IP = %s | SteamID = %s", clientIP, steamID);

	// Write the timestamp.
	std::time_t raw_time = std::time(nullptr);
	std::tm local_tm;

#if defined(_WIN32) || defined(_WIN64)
	// Windows thread-safe version (arguments reversed)
	localtime_s(&local_tm, &raw_time);
#else
	// Linux/POSIX thread-safe version
	localtime_r(&raw_time, &local_tm);
#endif

	if (bConnecting)
	{
		PRINT_MSG("%04d/%02d/%02d %02d:%02d:%02d // Stats: <%s><%s> <%s> %s",
			local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
			local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec,
			steamID,
			clientIP,
			clientType,
			handle->eAuthStatus == eAuthStatusOK ? "ACCEPTED" : "REJECTED");
	}
}

CAuthSystem::CAuthSystem()
{
	users.clear();
	callbacks_server = g_pSteamClient->callbacks_server;
	callbacks_client = g_pSteamClient->callbacks_client;

	AddRangeFilter(0, MIN_ALLOWED_ACCOUNT_ID);
}

CAuthSystem::~CAuthSystem()
{
	users.clear();
}

void CAuthSystem::RunCallbacks()
{
	if (call_ticket_validation && check_timedout(validation_time, 0.1))
	{
		call_ticket_validation = false;

		callbacks_client->AddCallbackResult(validation_response_data.k_iCallback, &validation_response_data,
			sizeof(ValidateAuthTicketResponse_t));

		callbacks_server->AddCallbackResult(validation_response_data.k_iCallback, &validation_response_data,
			sizeof(ValidateAuthTicketResponse_t));
	}
}

/*
* SteamGetEncryptedUserIDTicket (simplified)
*/
ESteamError CAuthSystem::GenerateTicket(void* buf, unsigned int buflen, unsigned int* ticketlen)
{
	*ticketlen == 0;

	if (!buf)
		return eSteamErrorBadArg;
	else if (!g_uSteamID.IsValid())
		return eSteamErrorLoginFailed;

	// ticket
	TRevTicket revTicket{};

	revTicket.version = REVTICKET_VERSION;			// +4
	revTicket.hash = g_uSteamID.GetAccountID() / 2;	// +8
	revTicket.signature = REVTICKET_SIGNATURE;		// +16
	revTicket.steamID = *(uint64*)&g_uSteamID;		// +24

	// +152
	strncpy(revTicket.hwid, g_szHostID, sizeof(g_szHostID));

	// copy ticket to output buffer
	memcpy(buf, &revTicket, sizeof(TRevTicket));
	*ticketlen = sizeof(TRevTicket);

	return eSteamErrorNone;
}

// Retrieve ticket to be sent to the entity who wishes to authenticate you ( using BeginAuthSession API ). 
// pcbTicket retrieves the length of the actual ticket.
void CAuthSystem::GetAuthTicket(void* pTicket, int cbMaxTicket, uint32* pcbTicket) 
{
	ESteamError status = GenerateTicket(pTicket, 2048, pcbTicket);
	
	switch (status)
	{

	case eSteamErrorBadArg: {
		PRINT_DEBUG("Invalid pTicket in GetAuthTicket");
		return;
	}
	case eSteamErrorLoginFailed: {
		PRINT_DEBUG("Login failed");
		return;
	}
	case eSteamErrorNone: {
		// write some debug logs
		PRINT_DEBUG("GetAuthTicket: size %u, steamID <%s> %s", 
			*pcbTicket, g_uSteamID.RenderAsSteam2String(), g_uSteamID.Render());
		break;
	}
	default:
		return;

	}
}

// Authenticate ticket ( from GetAuthSessionTicket ) from entity steamID to be sure it is valid and isnt reused
// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
EBeginAuthSessionResult CAuthSystem::BeginAuth(const void* pAuthTicket, int cbAuthTicket, uint32 unClientIP, CSteamID* pSteamID)
{
	TRevUserValidationHandle* authData = StartTicketValidation(pAuthTicket, cbAuthTicket, unClientIP);
	authData->uSteamID.SetAccountID(unClientIP << 1);

	if (authData->eAuthStatus == eAuthStatus_CorruptedTicket)
	{
		PRINT_DEBUG("Validation/1 failed with %s", GetAuthStatusString(authData->eAuthStatus));
		LogStats(true, false, authData);
		return k_EBeginAuthSessionResultInvalidTicket;
	}

	bool validationStatus = FinishTicketValidation(&authData, pAuthTicket, cbAuthTicket);

	if (!validationStatus)
	{
		PRINT_DEBUG("Validation/2 failed with %s: %s", GetAuthStatusString(authData->eAuthStatus), authData->szDetails);
		LogStats(true, false, authData);
		return k_EBeginAuthSessionResultInvalidTicket;
	}

	PRINT_DEBUG("Validation successful");
	LogStats(true, false, authData);

	pSteamID->SetFromUint64(authData->uSteamID.ConvertToUint64());

	// check for duplicates
	auto it = std::find_if(users.begin(), users.end(),
		[pSteamID](const user_t& u) -> bool {
			return u.unSteamID.ConvertToUint64() == pSteamID->ConvertToUint64();
		});

	if (it != users.end())
		return k_EBeginAuthSessionResultDuplicateRequest;

	for (const auto& range : m_vecAccountIDRangeFilters)
	{
		if (pSteamID->GetAccountID() >= range.min && pSteamID->GetAccountID() <= range.max)
			return k_EBeginAuthSessionResultInvalidTicket;
	}

	for (const auto& id : m_vecAccountIDFilters)
	{
		if (pSteamID->GetAccountID() == id)
			return k_EBeginAuthSessionResultInvalidTicket;
	}

	for (const auto& ip : m_vecIPFilters)
	{
		if (unClientIP == ip)
			return k_EBeginAuthSessionResultInvalidTicket;
	}

	// set up time and other data for running callbacks
	validation_time = std::chrono::high_resolution_clock::now();
	validation_response_data.m_SteamID = authData->uSteamID;
	validation_response_data.m_OwnerSteamID = authData->uSteamID;

	call_ticket_validation = true;

	user_t new_user;
	new_user.unSteamID.SetFromUint64(authData->uSteamID.ConvertToUint64());
	new_user.unClientIP = unClientIP;

	users.push_back(new_user);

	return k_EBeginAuthSessionResultOK;
}

// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
void CAuthSystem::EndAuth(uint32 unClientIP, CSteamID steamID)
{
	if (steamID.GetAccountID() == 0)
		return;

	auto it = std::find_if(users.begin(), users.end(),
		[steamID](const user_t& u) -> bool {
			return u.unSteamID.ConvertToUint64() == steamID.ConvertToUint64();
		});

	if (unClientIP == 0)
	{
		if (it != users.end())
			unClientIP = (*it).unClientIP;
	}

	TRevUserValidationHandle authData = { steamID, unClientIP };
	
	LogStats(false, true, &authData);

	validation_time = std::chrono::high_resolution_clock::time_point();
	memset(&validation_response_data, 0, sizeof(validation_response_data));

	if (it != users.end())
		users.erase(it);
}

/*
* First step of verification (checks basics like ticketlen and clienttype)
*/
TRevUserValidationHandle* CAuthSystem::StartTicketValidation(const void* ticket, unsigned int ticketlen, unsigned int clientip)
{
	TRevUserValidationHandle* hRevHandle = new TRevUserValidationHandle();
	memset(hRevHandle, 0, sizeof(TRevUserValidationHandle));

	// initialize with default values
	hRevHandle->uClientIP = clientip;
	hRevHandle->uSteamID = k_steamIDNotInitYetGS;

	int* pTicket = (int*)ticket;

	// Check signature ('rev')
	if (ticketlen == 24 || ticketlen == sizeof(TRevTicket) ||
		ticketlen == sizeof(TRevTicket) + 12)
	{
		// This is our auth ticket format.
		hRevHandle->eClientType = eClientRevEmu2;

		if (pTicket[2] == REVTICKET_SIGNATURE)
			hRevHandle->eAuthStatus = eAuthStatusOK;
		else
			// Corrupted ticket
			hRevHandle->eAuthStatus = eAuthStatus_CorruptedTicket;
	}
	else if (ticketlen == 10)
	{
		// That is deprecated old format
		hRevHandle->eClientType = eClientLegacyRev;

		if (pTicket[0] == 0xFFFF)
			hRevHandle->eAuthStatus = eAuthStatusOK;
		else
			// Corrupted ticket
			hRevHandle->eAuthStatus = eAuthStatus_CorruptedTicket;
	}
	else if (ticketlen == 768)
	{
		// SteamEmu client
		hRevHandle->eClientType = eClientSteamEmu;

		if (pTicket[20] == -1)
			hRevHandle->eAuthStatus = eAuthStatusOK;
		else
			// Corrupted ticket
			hRevHandle->eAuthStatus = eAuthStatus_CorruptedTicket;
	}
	else
	{
		// Unknown client
		hRevHandle->eClientType = eClientUnknown;
		hRevHandle->eAuthStatus = eAuthStatusOK;
	}

	return hRevHandle;
}

/*
* Second step of verification and configuration
*/
bool CAuthSystem::FinishTicketValidation(TRevUserValidationHandle** recvHandle, const void* ticket, int ticketlen)
{
	if (!ticketlen || ticketlen < 10)
		return eSteamErrorInvalidUserIDTicket;

	TRevUserValidationHandle* hRevHandle = *recvHandle;
	const TRevTicket* pRevTicket = (TRevTicket*)ticket;
	int* pTicket = (int*)ticket;

	if (!hRevHandle || !pRevTicket)
		return false;

	// If client is RevEmu one, the eClientRevEmu2 is set by default
	if (hRevHandle->eClientType >= eClientRevEmu2)
	{
		// We need to check version more accurately
		switch (pRevTicket->version)
		{
		case REVTICKET_VERSION:
			hRevHandle->eClientType = eClientRevEmu4;
			break;
		case REVTICKET_VERSION_74:
			hRevHandle->eClientType = eClientRevEmu3;
			break;
		case REVTICKET_VERSION_46:
			hRevHandle->eClientType = eClientRevEmu2;
			break;
		default:
			break;
		}

		// ClientMod check (uses revemu2013, fuck that for now)
		if (pRevTicket->version >= 85) {
			sprintf(hRevHandle->szDetails, "invalid version 85 in RevEmu ticket");
			hRevHandle->eAuthStatus = eAuthStatus_TicketVersionRejected;
			hRevHandle->eClientType = eClientUnknown;
			return false;
		}

		// verify hash
		uint32 hash = JSHash(pRevTicket->hwid, strlen(pRevTicket->hwid));

		if (hash != pRevTicket->hash) {
			sprintf(hRevHandle->szDetails, "%u != %u", hash, pRevTicket->hash);
			hRevHandle->eAuthStatus = eAuthStatus_TicketCorruptHWID;
		}
	}

	// check SteamGameServer policy
	if (!g_bAllowLegacyRev && hRevHandle->eClientType == eClientLegacyRev)
		hRevHandle->eAuthStatus = eAuthStatus_RejectedByGameServerPolicy;

	if (!g_bAllowRevEmu2 && hRevHandle->eClientType == eClientRevEmu2)
		hRevHandle->eAuthStatus = eAuthStatus_RejectedByGameServerPolicy;

	if (!g_bAllowRevEmu3 && hRevHandle->eClientType == eClientRevEmu3)
		hRevHandle->eAuthStatus = eAuthStatus_RejectedByGameServerPolicy;

	if (!g_bAllowNonRev && hRevHandle->eClientType == eClientUnknown)
		hRevHandle->eAuthStatus = eAuthStatus_RejectedByGameServerPolicy;

	// Check if above verification steps failed
	if (hRevHandle->eAuthStatus != eAuthStatusOK)
		return false;

	// convert 64bit steamid into uint32 array (view CSteamID structure for more info)
	auto steamID = (uint32*)&pRevTicket->steamID;

	//
	hRevHandle->uSteamID.CreateBlankAnonUserLogon(k_EUniversePublic);

	// Here we set our steamid
	switch (hRevHandle->eClientType)
	{
	case eClientLegacyRev: {
		hRevHandle->uSteamID.Set(pTicket[1], k_EUniversePublic, k_EAccountTypeIndividual);
		break;
	}

	case eClientRevEmu2: {
		if (pRevTicket->hash != 20) {
			sprintf(hRevHandle->szDetails, "%u != %u", pRevTicket->hash, 20);
			hRevHandle->eAuthStatus = eAuthStatus_TicketCorruptHASH;
			break;
		}

		if (pRevTicket->steamID == 0 || steamID[0] == 0)
		{
			sprintf(hRevHandle->szDetails, "steamID == 0");
			hRevHandle->eAuthStatus = eAuthStatus_TicketCorruptSTEAMID;
			break;
		}

		// in this case account instance is 0 which indicates that this is old client
		hRevHandle->uSteamID.FullSet(pRevTicket->steamID, k_EUniversePublic, k_EAccountTypeIndividual);

		if (!hRevHandle->uSteamID.IsValid()) {
			sprintf(hRevHandle->szDetails, "invalid CSteamID object");
			hRevHandle->eAuthStatus = eAuthStatus_TicketCorruptSTEAMID;
		}

		break;
	}

	case eClientRevEmu3: {
		if (pRevTicket->hash != (steamID[0] / 2)) {
			sprintf(hRevHandle->szDetails, "%u != %u", pRevTicket->hash, (steamID[0] / 2));
			hRevHandle->eAuthStatus = eAuthStatus_TicketCorruptHASH;
			break;
		}

		hRevHandle->uSteamID.SetFromUint64(pRevTicket->steamID);

		if (!hRevHandle->uSteamID.IsValid()) {
			sprintf(hRevHandle->szDetails, "invalid CSteamID object");
			hRevHandle->eAuthStatus = eAuthStatus_TicketCorruptSTEAMID;
		}

		break;
	}

	case eClientRevEmu4: {
		if (pRevTicket->hash != (steamID[0] / 2)) {
			sprintf(hRevHandle->szDetails, "%u != %u", pRevTicket->hash, (steamID[0] / 2));
			hRevHandle->eAuthStatus = eAuthStatus_TicketCorruptHASH;
			break;
		}

		hRevHandle->uSteamID.SetFromUint64(pRevTicket->steamID);

		if (!hRevHandle->uSteamID.IsValid()) {
			sprintf(hRevHandle->szDetails, "invalid CSteamID object");
			hRevHandle->eAuthStatus = eAuthStatus_TicketCorruptSTEAMID;
		}

		break;
	};

	case eClientSteamEmu: {
		if (!hRevHandle->uClientIP)
		{
			hRevHandle->eAuthStatus = eAuthStatus_RejectedByGameServerPolicy;
			break;
		}

		// manual generation
		if (pTicket[21] == 777) {
			byte hash[16];
			snprintf((char*)hash, sizeof(hash), "%u", hRevHandle->uClientIP);

			uint32 accountID = murmur3_32(hash, sizeof(hash), 47);
			hRevHandle->uSteamID.Set(accountID, k_EUniversePublic, k_EAccountTypeIndividual);
			break;
		}

		// in this case account instance is 0 which indicates that this is old client
		hRevHandle->uSteamID.FullSet(pTicket[21], k_EUniversePublic, k_EAccountTypeIndividual);

		break;
	};

	case eClientUnknown: {
		if (!hRevHandle->uClientIP)
		{
			hRevHandle->eAuthStatus = eAuthStatus_RejectedByGameServerPolicy;
			break;
		}

		// unknown client, generate steamid from ip
		byte hash[16];
		snprintf((char*)hash, sizeof(hash), "%u", hRevHandle->uClientIP);

		uint32 accountID = murmur3_32(hash, sizeof(hash), 47);
		hRevHandle->uSteamID.Set(accountID, k_EUniversePublic, k_EAccountTypeIndividual);

		break;
	}
	default:
		sprintf(hRevHandle->szDetails, "invalid eClientType");
		hRevHandle->eAuthStatus = eAuthStatus_RejectedByGameServerPolicy;
		break;
	}

	if (hRevHandle->eAuthStatus != eAuthStatusOK)
		return false;

	return true;
}

// Add range filters for account id
void CAuthSystem::AddRangeFilter(uint32 accountIDmin, uint32 accountIDmax)
{
	accountIdRangeFilter filter = { accountIDmin, accountIDmax };
	m_vecAccountIDRangeFilters.push_back(filter);
}

// Add single filter for account id
void CAuthSystem::AddFilter(uint32 accountID)
{
	m_vecAccountIDFilters.push_back(accountID);
}

// Add single filter for ip
void CAuthSystem::AddIPFilter(uint32 clientIP)
{
	m_vecIPFilters.push_back(clientIP);
}

void CAuthSystem::RemoveRangeFilter(uint32 accountIDmin, uint32 accountIDmax)
{
	accountIdRangeFilter filter = { accountIDmin, accountIDmax };

	auto it = std::find_if(m_vecAccountIDRangeFilters.begin(), m_vecAccountIDRangeFilters.end(),
		[accountIDmin, accountIDmax](const accountIdRangeFilter& f) -> bool {
			return (f.min == accountIDmin) && (f.max == accountIDmax);
		});

	if (it != m_vecAccountIDRangeFilters.end())
	{
		m_vecAccountIDRangeFilters.erase(it);
	}
}

// Remove single filter for account id
void CAuthSystem::RemoveFilter(uint32 accountID)
{
	auto it = std::find_if(m_vecAccountIDFilters.begin(), m_vecAccountIDFilters.end(),
		[accountID](const uint32& f) -> bool {
			return f == accountID;
		});

	if (it != m_vecAccountIDFilters.end())
	{
		m_vecAccountIDFilters.erase(it);
	}
}

// Remove single filter for ip
void CAuthSystem::RemoveIPFilter(uint32 clientIP) 
{
	auto it = std::find_if(m_vecIPFilters.begin(), m_vecIPFilters.end(),
		[clientIP](const uint32& f) -> bool {
			return f == clientIP;
		});

	if (it != m_vecIPFilters.end())
	{
		m_vecIPFilters.erase(it);
	}
}

CAuthSystem* system()
{
	static CAuthSystem* authsystem = new CAuthSystem();
	return authsystem;
}

}

CON_COMMAND(rev_auth_addidrange, "Add accountID range filter")
{
	if (args.ArgC() < 3)
	{
		PRINT_MSG("Usage: rev_auth_addidrange <min> <max>");
		return;
	}

	uint32 min = (uint32)atoi(args.Arg(1));
	uint32 max = (uint32)atoi(args.Arg(2));

	auth::system()->AddRangeFilter(min, max);
}

CON_COMMAND(rev_auth_addid, "Add accountID filter")
{
	if (args.ArgC() < 2)
	{
		PRINT_MSG("Usage: rev_auth_addid <accountID>");
		return;
	}

	uint32 accountID = (uint32)atoi(args.Arg(1));
	auth::system()->AddFilter(accountID);
} 

CON_COMMAND(rev_auth_addip, "Add IP filter")
{
	if (args.ArgC() < 2)
	{
		PRINT_MSG("Usage: rev_auth_addip <ip>");
		return;
	}

	uint32 clientIP = (uint32)atoi(args.Arg(1));
	auth::system()->AddIPFilter(clientIP);
}

CON_COMMAND(rev_auth_rmidrange, "Remove accountID range filter")
{
	if (args.ArgC() < 3)
	{
		PRINT_MSG("Usage: rev_auth_rmidrange <min> <max>");
		return;
	}

	uint32 min = (uint32)atoi(args.Arg(1));
	uint32 max = (uint32)atoi(args.Arg(2));

	auth::system()->RemoveRangeFilter(min, max);
}

CON_COMMAND(rev_auth_rmid, "Remove accountID filter")
{
	if (args.ArgC() < 2)
	{
		PRINT_MSG("Usage: rev_auth_rmid <accountID>");
		return;
	}

	uint32 accountID = (uint32)atoi(args.Arg(1));
	auth::system()->RemoveFilter(accountID);
}

CON_COMMAND(rev_auth_rmip, "Remove IP filter")
{
	if (args.ArgC() < 2)
	{
		PRINT_MSG("Usage: rev_auth_rmip <ip>");
		return;
	}

	uint32 clientIP = (uint32)atoi(args.Arg(1));
	auth::system()->RemoveIPFilter(clientIP);
}