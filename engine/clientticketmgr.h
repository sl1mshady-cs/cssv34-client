#pragma once
#include <tier1/bitbuf.h>

class ITicketMgr
{
public:
	virtual void Init() = 0;

	virtual void Shutdown() = 0;

	virtual void WriteTicket(bf_write& buf) = 0;
};

extern ITicketMgr* ticketmgr;