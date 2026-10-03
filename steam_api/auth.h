#ifndef _AUTH_H
#define _AUTH_H

#ifdef _WIN32
#pragma once
#endif

#include "revCommon.h"

class SteamCallbacks;


namespace auth
{
	// client type
	enum EClientType
	{
		eClientLegacyRev = 0,
		eClientRevEmu2,
		eClientRevEmu3,
		eClientRevEmu4,
		eClientSteamEmu,
		eClientUnknown
	};

	// auth status
	enum EAuthStatus
	{
		eAuthStatusOK = 0,
		eAuthStatus_CorruptedTicket,
		eAuthStatus_RejectedByGameServerPolicy,
		eAuthStatus_TicketVersionRejected,
		eAuthStatus_TicketCorruptHWID,
		eAuthStatus_TicketCorruptSTEAMID,
		eAuthStatus_TicketCorruptHASH
	};

	// validation handle used for verification
	struct TRevUserValidationHandle
	{
		CSteamID		uSteamID;
		unsigned int	uClientIP;
		EClientType		eClientType;
		EAuthStatus		eAuthStatus;
		char			szDetails[1024];
	};

	// user handle structure that is assigned to every user 
	// after they pass validation.
	struct user_t
	{
		CSteamID	unSteamID;
		uint32		unClientIP;
	};

	/*
	* -- Auth System --
	*/
	class CAuthSystem
	{
	public:
		CAuthSystem();
		~CAuthSystem();

		// Log stats (revStats)
		void LogStats( bool bConnecting, bool bDisconnecting, TRevUserValidationHandle* handle );

		void RunCallbacks();
		
		// Retrieve ticket to be sent to the entity who wishes to authenticate you ( using BeginAuthSession API ). 
		// pcbTicket retrieves the length of the actual ticket.
		void GetAuthTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket );

		// Authenticate ticket ( from GetAuthSessionTicket ) from entity steamID to be sure it is valid and isnt reused
		// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
		EBeginAuthSessionResult BeginAuth( const void *pAuthTicket, int cbAuthTicket, 
			uint32 unClientIP, CSteamID* pSteamID );

		// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
		void EndAuth( uint32 unClientIP, CSteamID steamID );

		/*
		* 
		* Custom filter system, will kick anyone who connects
		* within a specified accountid range/single accountid/client ip
		* 
		* kicks with message "STEAM validation rejected"
		* 
		*/

		// Add range filters for account id
		void AddRangeFilter(uint32 accountIDmin, uint32 accountIDmax);

		// Add single filter for account id
		void AddFilter(uint32 accountID);
		
		// Add single filter for ip
		void AddIPFilter(uint32 clientIP);

		void RemoveRangeFilter(uint32 accountIDmin, uint32 accountIDmax);

		// Remove single filter for account id
		void RemoveFilter(uint32 accountID);

		// Remove single filter for ip
		void RemoveIPFilter(uint32 clientIP);

	protected:
		
		// Generate ticket
		ESteamError GenerateTicket(void* buf, unsigned int buflen, unsigned int* ticketlen);

		// Start Ticket validation
		TRevUserValidationHandle* StartTicketValidation(
			const void* ticket,
			unsigned int ticketlen,
			unsigned int clientip
		);

		// Finish Ticket validation
		bool FinishTicketValidation(
			TRevUserValidationHandle** recvHandle,
			const void* ticket,
			int ticketlen
		);

	private:

		struct accountIdRangeFilter {
			uint32 min;
			uint32 max;
		};

		std::vector<accountIdRangeFilter> m_vecAccountIDRangeFilters;
		std::vector<uint32> m_vecAccountIDFilters;
		std::vector<uint32> m_vecIPFilters;

	private:
		// those would be set to ones from steamclient
		SteamCallbacks* callbacks_server{}, *callbacks_client{};
		std::vector<user_t> users;
		bool call_ticket_validation;

		ValidateAuthTicketResponse_t validation_response_data{};
		std::chrono::high_resolution_clock::time_point validation_time;
	};


	CAuthSystem* system();
}

#endif // _AUTH_H