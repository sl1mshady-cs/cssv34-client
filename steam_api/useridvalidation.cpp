#include "steamclient/serverlist/socket.h"
#include "tier0/dbg.h"
#include "revCommon.h"
#include "logging.h"
#include "useridvalidation.h"

extern CLoggingSystem* Logger;
extern bool g_bLogging;

extern CSteamID g_uSteamID; 
extern char g_szHostID[128];

// TODO: add rev.ini support
bool g_bAllowNonRev = false;
bool g_bAllowLegacyRev = false;
bool g_bAllowRevEmu2 = true;
bool g_bAllowRevEmu3 = true;
bool g_bAllowSteamEmu = false;

/*
* Convert SteamID to readable format
*/
const char* GetUserIDString(const CSteamID& steamid)
{
	static char idstr[128];
	_snprintf(idstr, sizeof(idstr) - 1, "STEAM_%u:%u:%u", 
		steamid.GetAccountID() & 1, // 1 in most of the cases
		steamid.GetAccountID() % 2,
		steamid.GetAccountID() / 2);

	idstr[sizeof(idstr) - 1] = '\0';

	return idstr;
}

/*
* ERevClientType -> string
*/
const char* GetClientTypeString(ERevClientType type)
{
	static char typestr[18];
	memset(typestr, 0, 18);
	switch (type)
	{
	case eClientLegacyRev:
		strcpy(typestr, "Very old Rev Emu");
		break;
	case eClientRevEmu2:
		strcpy(typestr, "Old Rev Emu");
		break;
	case eClientRevEmu3:
		strcpy(typestr, "Old Rev Emu v74");
		break;
	case eClientRevEmu4:
		strcpy(typestr, "Rev Emu");
		break;
	case eClientSteamEmu:
		strcpy(typestr, "Steam Emu");
		break;
	case eClientUnknown:
		strcpy(typestr, "Unknown");
		break;
	}

	return typestr;
}

/*
* helper log function for logging stats
*/
void LogStats(bool bConnecting, bool bDisconnecting, TRevUserValidationHandle* handle)
{
	const char* clientIP = inet_ntoa(*(in_addr*)&handle->uClientIP);
	const char* steamID = GetUserIDString(handle->uSteamID);
	const char* clientType = GetClientTypeString(handle->eClientType);

	Logger->Write("Ticket: %s.\n", clientType);

	if (bConnecting)
		Logger->Write("UserConnect IP = %s | ", clientIP);
	else if (bDisconnecting)
		Logger->Write("SteamDisconnect IP = %s | ", clientIP);

	Logger->Write("SteamID = %s\n", steamID);

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

	Msg( "%04d/%02d/%02d %02d:%02d:%02d // RevEmu Stats: <%s><%s> <%s> %s\t",
		local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
		local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, 
		steamID,
		clientIP,
		clientType,
		handle->eReturnCode == eSteamErrorNone ? "ACCEPTED" : "REJECTED");
}

/*
* SteamGetEncryptedUserIDTicket (simplified)
*/
S_API ESteamError S_CALLTYPE SteamGetEncryptedUserIDTicket(void* buf, unsigned int buflen, unsigned int* ticketlen)
{
	if (!buf) {
		*ticketlen == 0;
		return eSteamErrorBadArg;
	}

	if (!g_uSteamID.IsValid()) {
		*ticketlen == 2048;
		return eSteamErrorLoginFailed;
	}

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

/*
* First step of verification (checks basics like ticketlen and clienttype)
*/
S_API ESteamError S_CALLTYPE SteamStartValidatingUserIDTicket(void* ticket, unsigned int ticketlen, unsigned int clientip, TRevUserValidationHandle** recvHandle)
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
			hRevHandle->eReturnCode = eSteamErrorNone;
		else
			// Corrupted ticket
			hRevHandle->eReturnCode = eSteamErrorCorruptEncryptedUserIDTicket;
	}
	else if (ticketlen == 10)
	{
		// That is deprecated old format
		hRevHandle->eClientType = eClientLegacyRev;

		if (pTicket[0] == 0xFFFF)
			hRevHandle->eReturnCode = eSteamErrorNone;
		else
			// Corrupted ticket
			hRevHandle->eReturnCode = eSteamErrorCorruptEncryptedUserIDTicket;
	}
	else if (ticketlen == 768)
	{
		// SteamEmu client
		hRevHandle->eClientType = eClientSteamEmu;

		if (pTicket[20] == -1)
			hRevHandle->eReturnCode = eSteamErrorNone;
		else
			// Corrupted ticket
			hRevHandle->eReturnCode = eSteamErrorCorruptEncryptedUserIDTicket;
	}
	else
	{
		// Unknown client
		hRevHandle->eClientType = eClientUnknown;
		hRevHandle->eReturnCode = eSteamErrorNone;
	}

	*recvHandle = hRevHandle;

	return eSteamErrorNotFinishedProcessing;
}

/*
* Second step of verification and configuration
*/
S_API ESteamError S_CALLTYPE SteamProcessOngoingUserIDTicketValidation(TRevUserValidationHandle** recvHandle, void* ticket, int ticketlen)
{
	if (!ticketlen || ticketlen < 10)
		return eSteamErrorInvalidUserIDTicket;

	TRevUserValidationHandle* hRevHandle = *recvHandle;
	const TRevTicket* pRevTicket = (TRevTicket*)ticket;
	int* pTicket = (int*)ticket;

	if (!hRevHandle || !pRevTicket)
		return eSteamErrorInvalidUserIDTicket;

	// If client is RevEmu one, the eClientRevEmu2 is set by default
	if (hRevHandle->eClientType >= eClientRevEmu2)
	{
		// We need to check version more accurately
		if (pRevTicket->version == REVTICKET_VERSION)
			hRevHandle->eClientType = eClientRevEmu4;
		else if (pRevTicket->version == REVTICKET_VERSION_74)
			hRevHandle->eClientType = eClientRevEmu3;
		else if (pRevTicket->version == REVTICKET_VERSION_46)
			hRevHandle->eClientType = eClientRevEmu2;

		// ClientMod check (uses revemu2013, fuck that for now)
		if (pRevTicket->version >= 85)
			return eSteamErrorInvalidUserIDTicket;

		// verify hash
		int hash = JSHash(pRevTicket->hwid, 16 * sizeof(char));

		if (hash != pRevTicket->hash)
			return eSteamErrorInvalidUserIDTicket;
	}

	// check SteamGameServer policy
	if (!g_bAllowLegacyRev && hRevHandle->eClientType == eClientLegacyRev)
		return eSteamErrorInvalidUserIDTicket;

	if (!g_bAllowRevEmu2 && hRevHandle->eClientType == eClientRevEmu2)
		return eSteamErrorInvalidUserIDTicket;

	if (!g_bAllowRevEmu3 && hRevHandle->eClientType == eClientRevEmu3)
		return eSteamErrorInvalidUserIDTicket;

	if (!g_bAllowNonRev && hRevHandle->eClientType == eClientUnknown)
		return eSteamErrorInvalidUserIDTicket;

	// Check if the ticket is already corrupted
	if (hRevHandle->eReturnCode == eSteamErrorCorruptEncryptedUserIDTicket)
		return hRevHandle->eReturnCode;

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
			hRevHandle->eReturnCode = eSteamErrorCorruptEncryptedUserIDTicket;
			break;
		}

		if (pRevTicket->steamID == 0 || steamID[0] == 0)
		{
			hRevHandle->eReturnCode = eSteamErrorCorruptEncryptedUserIDTicket;
			break;
		}

		// in this case account instance is 0 which indicates that this is old client
		hRevHandle->uSteamID.FullSet(pRevTicket->steamID, k_EUniversePublic, k_EAccountTypeIndividual);
		
		if (!hRevHandle->uSteamID.IsValid()) {
			hRevHandle->eReturnCode = eSteamErrorCorruptEncryptedUserIDTicket;
		}

		break;
	}

	case eClientRevEmu3: {
		if ( pRevTicket->hash != (steamID[0] / 2) ) {
			hRevHandle->eReturnCode = eSteamErrorCorruptEncryptedUserIDTicket;
			break;
		}

		hRevHandle->uSteamID.SetFromUint64(pRevTicket->steamID);

		if (!hRevHandle->uSteamID.IsValid()) {
			hRevHandle->eReturnCode = eSteamErrorCorruptEncryptedUserIDTicket;
		}

		break;
	}

	case eClientRevEmu4: {
		if ( pRevTicket->hash != (steamID[0] / 2) ) {
			hRevHandle->eReturnCode = eSteamErrorCorruptEncryptedUserIDTicket;
			break;
		}

		hRevHandle->uSteamID.SetFromUint64(pRevTicket->steamID);

		if (!hRevHandle->uSteamID.IsValid()) {
			hRevHandle->eReturnCode = eSteamErrorCorruptEncryptedUserIDTicket;
		}

		break;
	};

	case eClientSteamEmu: {

		// steamemu failed generating steamid
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
		// unknown client, generate steamid from ip
		byte hash[16];
		snprintf((char*)hash, sizeof(hash), "%u", hRevHandle->uClientIP);

		uint32 accountID = murmur3_32(hash, sizeof(hash), 47);
		hRevHandle->uSteamID.Set(accountID, k_EUniversePublic, k_EAccountTypeIndividual);

		break;
	}
	default:
		hRevHandle->eReturnCode = eSteamErrorInvalidUserIDTicket;
		break;
	}

	return hRevHandle->eReturnCode;
}