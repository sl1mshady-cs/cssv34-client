#ifndef _USERID_VALIDATION_H
#define _USERID_VALIDATION_H

#ifdef _WIN32
#pragma once
#endif

/*
* Client type
*/
enum ERevClientType
{
	eClientLegacyRev = 0,
	eClientRevEmu2,
	eClientRevEmu3,
	eClientRevEmu4,
	eClientSteamEmu,
	eClientUnknown
};

/*
* Validation handle used for validating rev clients
*/
struct TRevUserValidationHandle
{
	ERevClientType	eClientType;
	CSteamID		uSteamID;
	unsigned int	uClientIP;
	ESteamError		eReturnCode;
};

const char* GetUserIDString(const CSteamID& steamid);
const char* GetClientTypeString(ERevClientType type);

void LogStats(bool bConnecting, bool bDisconnecting, TRevUserValidationHandle* handle);

S_API ESteamError S_CALLTYPE SteamGetEncryptedUserIDTicket(
	void* buf,
	unsigned int buflen,
	unsigned int* ticketlen);

S_API ESteamError S_CALLTYPE SteamStartValidatingUserIDTicket(
	void* ticket,
	unsigned int ticketlen,
	unsigned int clientip,
	TRevUserValidationHandle** recvHandle);

S_API ESteamError S_CALLTYPE SteamProcessOngoingUserIDTicketValidation(
	TRevUserValidationHandle** recvHandle,
	void* ticket,
	int ticketlen
);

#endif // _USERID_VALIDATION_H