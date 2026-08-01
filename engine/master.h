//======== (C) Copyright 1999, 2000 Valve, L.L.C. All rights reserved. ========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#ifndef MASTER_H
#define MASTER_H
#ifdef _WIN32
#pragma once
#endif

#include "engine/iserversinfo.h"

//-----------------------------------------------------------------------------
// Purpose: Implements a master server interface.
//-----------------------------------------------------------------------------
class IMaster
{
public:
	// Allow master server to register cvars/commands
	virtual void Init( void ) = 0;
	// System is shutting down
	virtual void Shutdown( void ) = 0;
	// Server is shutting down
	virtual void ShutdownConnection( void ) = 0;
	// Add server to global master list
	virtual void AddServer( struct netadr_s *adr ) = 0;
	// Console command to set/remove master server
	virtual void AddMaster_f( const CCommand &args ) = 0;

	virtual void ProcessConnectionlessPacket( netpacket_t *packet ) = 0;

	virtual void RunFrame( void ) = 0;
};

extern IMaster *master;
extern IServersInfo *g_pServersInfo;

#endif // MASTER_H
