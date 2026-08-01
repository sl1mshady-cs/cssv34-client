//======177== (C) Copyright 1999, 2000 Valve, L.L.C. All rights reserved. ========
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
// $NoKeywords: $
//=============================================================================
#include "quakedef.h"
#include "server.h"
#include "master.h"
#include "proto_oob.h"
#include "host.h"
#include "eiface.h"
#include "server.h"
#include "utlmap.h"

extern ConVar sv_tags;
extern ConVar sv_lan;

#define S2A_EDF_GAMEPORT 0x80
#define S2A_EDF_STEAMID 0x10
#define S2A_EDF_SOURCETV 0x40
#define S2A_EDF_GAMETAGS 0x20
#define S2A_EDF_GAMEID 0x01

#define RETRY_INFO_REQUEST_TIME 0.4 // seconds
#define MASTER_RESPONSE_TIMEOUT 1.5 // seconds
#define INFO_REQUEST_TIMEOUT 5.0 // seconds

#ifdef DEDICATED
#define IsLan() false
#else
#define IsLan() sv_lan.GetInt()
#endif

//-----------------------------------------------------------------------------
// Purpose: Implements the master server interface
//-----------------------------------------------------------------------------
class CMaster : public IMaster, public IServersInfo
{
public:
	CMaster( void );
	virtual ~CMaster( void );

	// Heartbeat functions.
	void Init( void );
	void Shutdown( void );
	// Sets up master address
	void ShutdownConnection(void);
	void AddServer( struct netadr_s *adr );
	void UseDefault ( void );
	void PingServer( netadr_t &svadr );

	void ProcessConnectionlessPacket( netpacket_t *packet );

	void AddMaster_f( const CCommand &args );

	void RunFrame();
	void RetryServersInfoRequest();

	void ReplyInfo( const netadr_t &adr );
	newgameserver_t &ProcessInfo( bf_read &buf );

	// SeversInfo
	void RequestInternetServerList( const char *gamedir, IServerListResponse *response );
	void RequestLANServerList( const char *gamedir, IServerListResponse *response );
	void AddServerAddresses( netadr_t **adr, int count );
	void RequestServerInfo( const netadr_t &adr );
	void StopRefresh();

	void PingServer(uint32 unIP, uint16 usPort, IServerPingResponse* pRequestServersResponse);
	void PlayerDetails(uint32 unIP, uint16 usPort, IServerPlayersResponse* pRequestServersResponse);
	virtual void RemoveResponse(IServerPingResponse* resp, IServerPlayersResponse* resp2);
private:

	bool m_bInitialized;
	bool m_bRefreshing;

	int m_iServersResponded;

	double m_flStartRequestTime;
	double m_flRetryRequestTime;
	double m_flMasterRequestTime;

	char m_szGameDir[256];

	// If nomaster is true, the server will not send heartbeats to the master server
	bool	m_bNoMasters;

	CUtlMap<netadr_t, bool> m_serverAddresses;
	CUtlMap<uint, double> m_serversRequestTime;

	CUtlVector<IServerPingResponse*> m_ServerPings;
	CUtlVector<IServerPlayersResponse*> m_ServerPlayers;

	IServerListResponse *m_serverListResponse;
};

static CMaster s_MasterServer;
IMaster *master = (IMaster *)&s_MasterServer;

IServersInfo *g_pServersInfo = (IServersInfo*)&s_MasterServer;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CMaster, IServersInfo, SERVERLIST_INTERFACE_VERSION, s_MasterServer );

#define	HEARTBEAT_SECONDS	140.0

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMaster::CMaster( void )
{
	m_bNoMasters		= false;
	m_bInitialized = false;
	m_iServersResponded = 0;

	m_serverListResponse = NULL;
	SetDefLessFunc( m_serverAddresses );
	SetDefLessFunc( m_serversRequestTime );
	m_bRefreshing = false;

	Init();
}

CMaster::~CMaster( void )
{
}

void CMaster::RunFrame()
{
	if( !m_bRefreshing )
		return;

	if( m_serverListResponse &&
		m_flStartRequestTime < Plat_FloatTime()-INFO_REQUEST_TIMEOUT )
	{
		StopRefresh();
		m_serverListResponse->RefreshComplete( NServerResponse::nServerFailedToRespond );
		return;
	}

	if( m_iServersResponded > 0 &&
			m_iServersResponded >= m_serverAddresses.Count() &&
			m_flMasterRequestTime < Plat_FloatTime() - MASTER_RESPONSE_TIMEOUT )
	{
		StopRefresh();
		m_serverListResponse->RefreshComplete( NServerResponse::nServerResponded );
		return;
	}

	if( m_flRetryRequestTime < Plat_FloatTime() - RETRY_INFO_REQUEST_TIME )
	{
		m_flRetryRequestTime = Plat_FloatTime();

		if( m_serverAddresses.Count() == 0 ) // Retry masterserver request
		{
			g_pServersInfo->RequestInternetServerList(m_szGameDir, NULL);
			return;
		}

		if( m_iServersResponded < m_serverAddresses.Count() )
			RetryServersInfoRequest();
	}
}

void CMaster::StopRefresh()
{
	if( !m_bRefreshing )
		return;

	m_iServersResponded = 0;
	m_bRefreshing = false;
	m_serverAddresses.RemoveAll();
	m_serversRequestTime.RemoveAll();
}

void CMaster::ReplyInfo( const netadr_t &adr )
{
	static char gamedir[MAX_OSPATH];
	Q_FileBase( com_gamedir, gamedir, sizeof( gamedir ) );

	CUtlBuffer buf;
	buf.EnsureCapacity( 2048 );

	buf.PutUnsignedInt( LittleDWord( CONNECTIONLESS_HEADER ) );
	buf.PutUnsignedChar( S2A_INFO_SRC );

	buf.PutUnsignedInt(0);
	buf.PutUnsignedChar( PROTOCOL_VERSION ); // Hardcoded protocol version number
	buf.PutString( sv.GetName() );
	buf.PutString( sv.GetMapName() );
	buf.PutString( gamedir );
	buf.PutString( serverGameDLL->GetGameDescription() );

	// player info
	buf.PutUnsignedChar( sv.GetNumClients() );
	buf.PutUnsignedChar( sv.GetMaxClients() );
	buf.PutUnsignedChar( sv.GetNumFakeClients() );

	// Password?
	buf.PutUnsignedChar( sv.GetPassword() != NULL ? 1 : 0 );

	// Write a byte with some flags that describe what is to follow.
	const char *pchTags = sv_tags.GetString();
	int nFlags = 0;

	if ( pchTags && pchTags[0] != '\0' )
		nFlags |= S2A_EDF_GAMETAGS;

	buf.PutUnsignedInt( nFlags );

	if ( nFlags & S2A_EDF_GAMETAGS)
		buf.PutString( pchTags );

	NET_SendPacket( NULL, NS_SERVER, adr, (unsigned char *)buf.Base(), buf.TellPut() );
}

newgameserver_t &CMaster::ProcessInfo(bf_read &buf)
{
	static newgameserver_t s;
	memset(&s, 0, sizeof(s));

	s.m_nProtocolVersion = buf.ReadByte();

	buf.ReadString(s.m_szServerName, sizeof(s.m_szServerName));
	buf.ReadString(s.m_szMap, sizeof(s.m_szMap));
	buf.ReadString(s.m_szGameDir, sizeof(s.m_szGameDir));

	buf.ReadString(s.m_szGameDescription, sizeof(s.m_szGameDescription));
	s.m_nAppID = buf.ReadShort();

	// player info
	s.m_nPlayers = buf.ReadByte();
	s.m_nMaxPlayers = buf.ReadByte();
	s.m_nBotPlayers = buf.ReadByte();

	// Password?
	buf.ReadByte(); // server type
	buf.ReadByte(); // env

	s.m_bPassword = buf.ReadByte();
	s.m_bSecure = buf.ReadByte();
	buf.ReadString(s.m_szGameVersion, sizeof(s.m_szGameVersion));

	s.m_iFlags = buf.ReadByte();

	if (s.m_iFlags & S2A_EDF_GAMEPORT)
	{
		s.m_NetAdr.SetPort(buf.ReadShort());
	}

	if (s.m_iFlags & S2A_EDF_STEAMID)
	{
		uint64 ulSteamID;
		buf.ReadBytes(&ulSteamID, 8);
	}

	if (s.m_iFlags & S2A_EDF_SOURCETV)
	{
		char str[64];
		buf.ReadShort(); // spectator port (unused)
		buf.ReadString(str, sizeof(str)); // spectator sv name
	}

	if (s.m_iFlags & S2A_EDF_GAMETAGS)
	{
		buf.ReadString(s.m_szGameTags, sizeof(s.m_szGameTags));
	}

	if (s.m_iFlags & S2A_EDF_GAMEID)
	{
		uint64 ulGameID;
		buf.ReadBytes(&ulGameID, 8);
	}

	return s;
}

void CMaster::ProcessConnectionlessPacket( netpacket_t *packet )
{
	static ALIGN4 char string[2048] ALIGN4_POST;    // Buffer for sending heartbeat

	uint ip; uint16 port;

	bf_read msg = packet->message;
	char c = msg.ReadChar();

	if ( c == 0  )
		return;

	switch( c )
	{
		case 0x41:
		{
			int challenge = msg.ReadLong();

			static ALIGN4 char string[256] ALIGN4_POST;
			bf_write msg(string, sizeof(string));

			msg.WriteLong(CONNECTIONLESS_HEADER);
			msg.WriteByte(A2S_PLAYER);
			msg.WriteLong(challenge);
			
			NET_SendPacket(NULL, NS_CLIENT, packet->from, msg.GetData(), msg.GetNumBytesWritten());

			break;
		}

		case 0x44:
		{
			int nPlayers = msg.ReadByte();
			for (int i = 0; i < nPlayers; ++i)
			{
				char str[256];

				msg.ReadByte();
				msg.ReadString(str, sizeof(str));
				int score = msg.ReadLong();
				float duration = msg.ReadFloat();

				if (str[0] != 0)
				{
					FOR_EACH_VEC(m_ServerPlayers, i)
					{
						IServerPlayersResponse* resp = m_ServerPlayers[i];
						resp->AddPlayerToList(str, score, duration);
					}
				}
			}

			FOR_EACH_VEC(m_ServerPlayers, i)
			{
				IServerPlayersResponse* resp = m_ServerPlayers[i];
				resp->PlayersRefreshComplete();
			}
		}

		case M2A_SERVER_BATCH:
		{
			if( !m_bRefreshing )
				break;

			msg.ReadByte();

			while (true)
			{
				if (msg.GetNumBytesLeft() < 6)
				{
					break;
				}

				ip = msg.ReadLong();
				port = msg.ReadShort();
				if (ip == 0 || port == 0) {
					break;
				}

				netadr_t adr(BigLong(ip), BigShort(port));

				unsigned short index = m_serverAddresses.Find(adr);
				if (index != m_serverAddresses.InvalidIndex())
				{
					continue;
				}

				m_serverAddresses.Insert(adr, false);
				RequestServerInfo(adr);
			}

			if (ip != 0 && port != 0)
			{
				ALIGN4 char buf[256] ALIGN4_POST;
				bf_write msg(buf, sizeof(buf));

				netadr_t startadr(BigLong(ip), BigShort(port));

				msg.WriteByte(C2M_CLIENTQUERY);
				msg.WriteByte(255); // region always 255 (all)
				msg.WriteString(startadr.ToString());
				msg.WriteString(va("\\gamedir\\%s", COM_GetModDirectory()));

				netadr_t adr("78.154.103.37:10232");
				NET_SendPacket(NULL, NS_CLIENT, adr, msg.GetData(), msg.GetNumBytesWritten());
			}

			break;
		}
		case C2S_INFOREQUEST:
		{
			ReplyInfo(packet->from);
			break;
		}
		case S2A_INFO_SRC:
		{
			if( !m_bRefreshing )
				break;

			newgameserver_t &s = ProcessInfo( msg );

			unsigned short index = m_serverAddresses.Find(packet->from);
			unsigned short rindex = m_serversRequestTime.Find(packet->from.GetIPHostByteOrder()+packet->from.GetPort());

			if( index == m_serverAddresses.InvalidIndex() ||
				rindex == m_serversRequestTime.InvalidIndex() )
				break;

			double requestTime = m_serversRequestTime[rindex];

			if( m_serverAddresses[index] ) // shit happens
				return;

			m_serverAddresses[index] = true;
			s.m_nPing = (Plat_FloatTime()-requestTime)*1000.0;
			s.m_NetAdr = packet->from;
			m_serverListResponse->ServerResponded( s );

			FOR_EACH_VEC(m_ServerPings, i)
			{
				IServerPingResponse* resp = m_ServerPings[i];
				resp->ServerResponded(s);
			}

			m_iServersResponded++;
			break;
		}
	}
}

void CMaster::RequestServerInfo( const netadr_t &adr )
{
	static ALIGN4 char string[256] ALIGN4_POST;    // Buffer for sending heartbeat
	bf_write msg( string, sizeof(string) );

	msg.WriteLong( CONNECTIONLESS_HEADER );
	msg.WriteByte( A2S_INFO );
	msg.WriteString( A2S_KEY_STRING );
	m_serversRequestTime.Insert(adr.GetIPHostByteOrder()+adr.GetPort(), Plat_FloatTime());

	NET_SendPacket( NULL, NS_CLIENT, adr, msg.GetData(), msg.GetNumBytesWritten() );
}

void CMaster::RetryServersInfoRequest()
{
	FOR_EACH_MAP_FAST( m_serverAddresses, i )
	{
		bool bResponded = m_serverAddresses.Element(i);
		if( bResponded )
			continue;

		const netadr_t adr = m_serverAddresses.Key(i);
		RequestServerInfo( adr );
	}
}

void CMaster::PingServer(uint32 unIP, uint16 usPort, IServerPingResponse* pRequestServersResponse)
{
	netadr_t adr(unIP, usPort);
	m_ServerPings.AddToTail(pRequestServersResponse);

	static ALIGN4 char string[256] ALIGN4_POST;
	bf_write msg(string, sizeof(string));

	msg.WriteLong(CONNECTIONLESS_HEADER);
	msg.WriteByte(A2S_INFO);
	msg.WriteString(A2S_KEY_STRING);
	m_serversRequestTime.Insert(adr.GetIPHostByteOrder() + adr.GetPort(), Plat_FloatTime());

	NET_SendPacket(NULL, NS_CLIENT, adr, msg.GetData(), msg.GetNumBytesWritten());
}

void CMaster::PlayerDetails(uint32 unIP, uint16 usPort, IServerPlayersResponse* pRequestServersResponse)
{
	netadr_t adr(unIP, usPort);
	m_ServerPlayers.AddToTail(pRequestServersResponse);

	static ALIGN4 char string[256] ALIGN4_POST;
	bf_write msg(string, sizeof(string));

	msg.WriteLong(CONNECTIONLESS_HEADER);
	msg.WriteByte(0x57);
	msg.WriteString("000000000000");

	NET_SendPacket(NULL, NS_CLIENT, adr, msg.GetData(), msg.GetNumBytesWritten());
}

void CMaster::RemoveResponse(IServerPingResponse* resp, IServerPlayersResponse* resp2)
{
	m_ServerPings.FindAndFastRemove(resp);
	m_ServerPlayers.FindAndFastRemove(resp2);
}

//-----------------------------------------------------------------------------
// Purpose: Server is shutting down, unload master servers list, tell masters that we are closing the server
//-----------------------------------------------------------------------------
void CMaster::ShutdownConnection( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Add server to the master list
// Input  : *adr - 
//-----------------------------------------------------------------------------
void CMaster::AddServer( netadr_t *adr )
{
}

//-----------------------------------------------------------------------------
// Purpose: Add built-in default master if woncomm.lst doesn't parse
//-----------------------------------------------------------------------------
void CMaster::UseDefault ( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Add/remove master servers
//-----------------------------------------------------------------------------
void CMaster::AddMaster_f ( const CCommand &args )
{
	CUtlString cmd( ( args.ArgC() > 1 ) ? args[ 1 ] : "" );

	netadr_t adr;

	if( !NET_StringToAdr(cmd.String(), &adr) )
	{
		Warning("Invalid address\n");
		return;
	}

	this->AddServer(&adr);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void AddMaster_f( const CCommand &args )
{
	master->AddMaster_f( args );
}

static ConCommand setmaster("addmaster", AddMaster_f );

//-----------------------------------------------------------------------------
// Purpose: Adds master server console commands
//-----------------------------------------------------------------------------
void CMaster::Init( void )
{
	// Already able to initialize at least once?
	if ( m_bInitialized )
		return;

	// So we don't do this a send time.sv_mas
	m_bInitialized = true;

	UseDefault();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMaster::Shutdown(void)
{
}

// ServersInfo
void CMaster::RequestInternetServerList(const char *gamedir, IServerListResponse *response)
{
	if( m_bNoMasters ) return;
	strncpy( m_szGameDir, gamedir, sizeof(m_szGameDir) );

	if( response )
	{
		StopRefresh();
		m_bRefreshing = true;
		m_serverListResponse = response;
		m_flRetryRequestTime = m_flStartRequestTime = m_flMasterRequestTime = Plat_FloatTime();
	}

	ALIGN4 char buf[256] ALIGN4_POST;
	bf_write msg(buf, sizeof(buf));

	msg.WriteByte( C2M_CLIENTQUERY );
	msg.WriteByte( 255 ); // region always 255 (all)
	msg.WriteString("0.0.0.0:0");
	msg.WriteString(va("\\gamedir\\%s", COM_GetModDirectory()));

	netadr_t adr("78.154.103.37:10232");
	NET_SendPacket(NULL, NS_CLIENT, adr, msg.GetData(), msg.GetNumBytesWritten() );
}

void CMaster::RequestLANServerList(const char *gamedir, IServerListResponse *response)
{

}

void CMaster::AddServerAddresses( netadr_t **adr, int count )
{

}
