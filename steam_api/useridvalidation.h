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

enum EAuthStatus
{
	eAuthStatusOK = 0,
	eAuthStatus_CorruptedTicket,
	eAuthStatus_TicketRejected,
	eAuthStatus_TicketVersionRejected,
	eAuthStatus_TicketCorruptHWID,
	eAuthStatus_TicketCorruptSTEAMID,
	eAuthStatus_TicketCorruptHASH
};

/*
* Validation handle used for validating rev clients
*/
struct TRevUserValidationHandle
{
	ERevClientType	eClientType;
	CSteamID		uSteamID;
	unsigned int	uClientIP;
	EAuthStatus		eReturnCode;

	// contains details for eReturnCode
	char			szDetails[1024];
};

const char* GetUserIDString(const CSteamID steamid);
const char* GetClientTypeString(ERevClientType type);

void LogStats(bool bConnecting, bool bDisconnecting, TRevUserValidationHandle* handle);

ESteamError S_CALLTYPE SteamGetEncryptedUserIDTicket(
	void* buf,
	unsigned int buflen,
	unsigned int* ticketlen);

void S_CALLTYPE SteamStartValidatingUserIDTicket(
	void* ticket,
	unsigned int ticketlen,
	unsigned int clientip,
	TRevUserValidationHandle** recvHandle);

bool S_CALLTYPE SteamProcessOngoingUserIDTicketValidation(
	TRevUserValidationHandle** recvHandle,
	void* ticket,
	int ticketlen
);

#endif // _USERID_VALIDATION_H