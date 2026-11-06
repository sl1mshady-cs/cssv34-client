/*
 *
 * Copyright (c) 2026 RuSHeRR
 *
 * Purpose: easy socket
 *	class implementation
 *
*/
#if !defined( SOCKET_H )
#define SOCKET_H
#ifdef _WIN32
#pragma once
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

typedef struct WSAData {
    unsigned short          wVersion;
    void* _placeholder;
} WSADATA;

#define WSAStartup()
#define WSACleanup()
#define closesocket close
#define ioctlsocket ioctl
#endif

#include "protocol.h"
#include "proto_oob.h"
#include "tier1/netadr.h"
#include "tier1/bitbuf.h"
#include "tier1/utlvector.h"
#include "tier1/utlmap.h"

#include <stdio.h>
#undef SendMessage

// ref https://github.com/rusherr-c/cssv34-engine/blob/dev/src/serverbrowser/TrackerProtocol.h#L15
#define SPLITPACKET_HEADER 0xFFFFFFFE

enum { 
    MAX_ROUTABLE_PACKET = 1400,
    MAX_RECEIVEABLE_PACKET = 8192,
    SPLIT_SIZE = (MAX_ROUTABLE_PACKET - 10),
    NET_MAX_MESSAGE = 96016
};

// forwards
class CSocket;

// Split long packets.  Anything over 1460 is failing on some routers
typedef struct
{
    int		currentSequence;
    int		splitCount;
    int		totalSize;
    char	buffer[NET_MAX_MESSAGE];
} LONGPACKET;

// Use this to pick apart the network stream, must be packed
#pragma pack(1)
typedef struct
{
	int		netID;
	int		sequenceNumber;
	short	packetID;
} SPLITPACKET;

// This one only exists in first split and when it's compressed
typedef struct
{
    int decompressedSize;
    int crc;
} SPLITPACKET_COMPRESSED;
#pragma pack()

struct splitpacket_t
{
    LONGPACKET packet;
    int flags[69];
    double lastReceiveTime;

    splitpacket_t()
    {
        Q_memset(&packet, 0, sizeof(packet));
        Q_memset(flags, 0, sizeof(flags));
        lastReceiveTime = 0.0;
    }
};

//-----------------------------------------------------------------------------
// Purpose: Creates a non-blocking, broadcast capable, UDP socket.  If port is
//  specified, binds it to listen on that port, otherwise, chooses a random port.
//-----------------------------------------------------------------------------
class CSocket
{
public:
    CSocket(uint16 port = 0, bool nonblocking = false);
    ~CSocket();

    bool Open(uint16 port = 0, bool nonblocking = false);
    void Close();

    bool IsValid() const;

    // in most of the cases you should use 'bf_write' variant
    int Send(const netadr_t& to, const void* data, int length);
    int Send(const netadr_t& to, bf_write& msg);

    int Broadcast(uint16 port, const void* data, int length);
    int Broadcast(uint16 port, bf_write& msg);

    int Receive(void* buf, int maxlen, netadr_t& from);
    //int Receive(bf_read& msg, netadr_t& from);

    void Frame(); // optional

    int32 GetSocket() const;
    const netadr_t& GetAddress() const;

protected:

    // receive status for ReceiveDataInternal (private)
    enum SockReceiveStatus
    {
        EReceivedInvalidPacket = 0, // invalid packet
        EReceivedSinglePacket,      // got single, also used for split packets if finished
        EReceivedSplitPacket        // got split
    };

    SockReceiveStatus ReceiveDataInternal(void* buf, int maxlen, sockaddr* from, int* fromlen, int* outsize);

    /* splits */

    splitpacket_t* FindOrCreateSplitPacket(const netadr_t& adr);
    void RemoveSplitPacket(const netadr_t& adr);
    void CleanupSplitPackets();

    CUtlMap< netadr_t, splitpacket_t > m_SplitPackets;
    
private:
    
    int m_hSocket;

    netadr_t m_Address;
};

#endif // SOCKET_H