/*
 *
 * Copyright (c) 2026 RuSHeRR
 *
 * Purpose: easy socket
 *	class implementation
 *
*/
#if !defined( _X360 )
#define FD_SETSIZE 1024
#endif

#include <assert.h>
#include "socket.h"
#include "tier0/vcrmode.h"
#include "color.h"
#include "convar.h"
#include "../utils/bzip2/bzlib.h"
#include "checksum_crc.h"

#include <VGUI/IVGui.h>

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// [max 5], dont set to higher values otherwise 
// some servers will be dropped
#define RUNFRAME_SLEEP_INTERVAL 1

static ConVar sb_sock_debugging("rev_sock_debugging", "0", FCVAR_NONE, "Enable socket debugging messages");

const Color SocketDebugColor1(255, 100, 255, 255);
const Color SocketDebugColor2(255, 255, 100, 255);

bool SplitPacketLessFunc(const netadr_t& a, const netadr_t& b)
{
    int ip1 = *(int*)a.ip;
    int ip2 = *(int*)b.ip;

    if (ip1 != ip2)
        return ip1 < ip2;
    
    return a.port < b.port;
}

#ifdef LINUX
#define WSAEWOULDBLOCK EWOULDBLOCK
int WSAGetLastError()
{
    return errno;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Creates a non-blocking, broadcast capable, UDP socket.  If port is
//  specified, binds it to listen on that port, otherwise, chooses a random port.
//-----------------------------------------------------------------------------
CSocket::CSocket(uint16 port, bool nonblocking)
    : m_SplitPackets(0, 128, SplitPacketLessFunc)
{
    m_hSocket = INVALID_SOCKET;

    Open(port, nonblocking);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSocket::~CSocket()
{
    Close();
}

bool CSocket::IsValid() const
{
    return m_hSocket != INVALID_SOCKET;
}

//-----------------------------------------------------------------------------
// Purpose: Open socket on given port
// Input  : port -
// 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSocket::Open(uint16 port, bool nonblocking)
{
    Close();
    
    m_hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (m_hSocket == INVALID_SOCKET)
        return false;

    int32 broadcast = TRUE;

    setsockopt(
        m_hSocket,
        SOL_SOCKET,
        SO_BROADCAST,
        (char*)&broadcast,
        sizeof(broadcast));

    if (nonblocking)
    {
        u_long mode = 1;

        ioctlsocket(
            m_hSocket,
            FIONBIO,
            &mode);
    }

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(
        m_hSocket,
        (sockaddr*)&addr,
        sizeof(addr)) == SOCKET_ERROR)
    {
        Close();
        return false;
    }

    sockaddr_in local{};
    int len = sizeof(local);

    getsockname(
        m_hSocket,
        (sockaddr*)&local,
        &len);

    m_Address.SetFromSockadr(
        (sockaddr*)&local);

    //ConColorMsg(SocketDebugColor1, "Opened socket %i at port %d\n", m_hSocket, ntohs(local.sin_port));

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Close this socket
//-----------------------------------------------------------------------------
void CSocket::Close()
{
    if (m_hSocket != INVALID_SOCKET)
    {
        ConColorMsg(SocketDebugColor2, "Closed socket %i\n", m_hSocket);

        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }

    m_Address.Clear();
}

//-----------------------------------------------------------------------------
// Purpose: Send message to specified address
// Input  : to - 
//			data -
//			length -
// 
// Output : int - number of bytes sent
//-----------------------------------------------------------------------------
int CSocket::Send(
    const netadr_t& to,
    const void* data,
    int length)
{
    if (sb_sock_debugging.GetBool())
        ConColorMsg(SocketDebugColor1, "--> Send to %s data %s len %i sock %u\n", to.ToString(), (const char*)data, length, m_hSocket);

    sockaddr addr{};

    to.ToSockadr(&addr);

    int ret = sendto(
        m_hSocket,
        (const char*)data,
        length,
        0,
        &addr,
        sizeof(sockaddr_in));

    if (ret == -1)
    {
        Warning("!!! sendto failed: %d\n", WSAGetLastError());
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Purpose: Send message to specified address
// Input  : to - 
//			msg -
// 
// Output : int - number of bytes sent
//-----------------------------------------------------------------------------
int CSocket::Send(const netadr_t& to, bf_write& msg)
{
    return Send(
        to,
        msg.GetData(),
        msg.GetNumBytesWritten());
}

//-----------------------------------------------------------------------------
// Purpose: Send broadcast message on specified port
// Input  : port - 
//			data -
//			length -
// 
// Output : int - number of bytes sent
//-----------------------------------------------------------------------------
int CSocket::Broadcast(
    uint16 port,
    const void* data,
    int length)
{
    if (sb_sock_debugging.GetBool())
        ConColorMsg(SocketDebugColor2, "--> Broadcast port %d data %s len %i sock %u\n", port, (const char*)data, length, m_hSocket);

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_BROADCAST;

    return sendto(
        m_hSocket,
        (const char*)data,
        length,
        0,
        (sockaddr*)&addr,
        sizeof(addr));
}

//-----------------------------------------------------------------------------
// Purpose: Send broadcast message on specified port
// Input  : port - 
//			msg - 
// 
// Output : int - number of bytes sent
//-----------------------------------------------------------------------------
int CSocket::Broadcast(uint16 port, bf_write& msg)
{
    return Broadcast(
        port,
        msg.GetData(),
        msg.GetNumBytesWritten());
}

/*
* Receive incoming message
*/
int CSocket::Receive(void* buf, int maxlen, netadr_t& from)
{
    sockaddr sckFrom{};
    int sckFromLen = sizeof(sckFrom);
    int outsize = 0;

    // calling this to cleanup splits
    //Frame();

    while (true)
    {
        SockReceiveStatus status = ReceiveDataInternal(buf, maxlen, &sckFrom, &sckFromLen, &outsize);

        if (status == EReceivedInvalidPacket)
        {
            int error = WSAGetLastError();

            // No more data on socket
            if (error == WSAEWOULDBLOCK)
                break;

            // recvfrom returned error
            if (error != 0)
            {
                Warning("Socket %i receivedata failed (%d)\n",
                    m_hSocket, error);
            }

            break;
        }
        else if (status == EReceivedSinglePacket)
        {
            break;
        }
        else if (status != EReceivedSplitPacket)
        {
            Warning("Socket %i corrupt receivedata response\n",
                m_hSocket);

            break;
        }

        // it's splitpacket, continue...
        ///////////////////////////
    }

    // we've finished!
    from.SetFromSockadr(&sckFrom);

    if (outsize == -1)
    {
        Warning("!!! recvfrom failed: %d\n", WSAGetLastError());
    }

    return outsize;
}

/*
* Receive incoming message (bitbuf)
*/
/*
int CSocket::Receive(bf_read& msg, netadr_t& from)
{
    unsigned char* buf = (unsigned char*)msg.GetBasePointer();
    return Receive(buf, msg.TotalBytesAvailable(), from);
}*/

//-----------------------------------------------------------------------------
// Purpose: Called once per frame (outside of the socket thread) to allow socket to receive incoming messages
//  and route them as appropriate
//-----------------------------------------------------------------------------
void CSocket::Frame()
{
    if (!IsValid())
        return;

    // Cleanup splitpackets
    static double flLastCleanTime = Plat_FloatTime();
    double curtime = Plat_FloatTime();

    if (curtime - flLastCleanTime > 10.0f) {
        CleanupSplitPackets();
        flLastCleanTime = Plat_FloatTime();
    }

    /*
    while (true)
    {
        float curtime = Plat_FloatTime();
        if (curtime - flLastCleanTime > 10.0f)
            CleanupSplitPackets();

        if (!ReceiveData())
        {
            int error = WSAGetLastError();

            // No more data on socket
            if (error == WSAEWOULDBLOCK)
                break;

            // recvfrom returned error
            if (error != 0)
            {
                Warning("Socket %i recvfrom failed (%d)\n",
                    m_hSocket, error);
            }

            break;
        }

        // Optimization
        if (RUNFRAME_SLEEP_INTERVAL > 0)
            Sleep(RUNFRAME_SLEEP_INTERVAL);
    }
    */
}

//-----------------------------------------------------------------------------
// Purpose: Get socket
// Output : SOCKET
//-----------------------------------------------------------------------------
int32 CSocket::GetSocket(void) const
{
    return m_hSocket;
}

//-----------------------------------------------------------------------------
// Purpose: Resolves the socket address
// Output : const netadr_t
//-----------------------------------------------------------------------------
const netadr_t& CSocket::GetAddress() const
{
    return m_Address;
}

//-----------------------------------------------------------------------------
// Purpose: Called once FD_ISSET is detected
//-----------------------------------------------------------------------------
CSocket::SockReceiveStatus CSocket::ReceiveDataInternal(
    void* buf, int maxlen, sockaddr* from, int* fromlen, int* outsize)
{
    if (maxlen > MAX_RECEIVEABLE_PACKET)
        maxlen = MAX_RECEIVEABLE_PACKET;

    // we cannot use maxlen here :(
    char buffer[MAX_RECEIVEABLE_PACKET];

    int bytes = recvfrom(
        m_hSocket,
        buffer,
        maxlen,
        0,
        from,
        fromlen);

    *outsize = bytes;

    if (bytes == SOCKET_ERROR)
        return EReceivedInvalidPacket;

    if (bytes < 4 || bytes >= MAX_ROUTABLE_PACKET)
        return EReceivedInvalidPacket;

    netadr_t adr;
    adr.SetFromSockadr(from);

    //---------------------------------------------------------
    // Split packet?
    //---------------------------------------------------------

    if (*(int*)buffer == SPLITPACKET_HEADER)
    {
        splitpacket_t* entry = FindOrCreateSplitPacket(adr);
        LONGPACKET& packet = entry->packet;

        // update receive time
        entry->lastReceiveTime = Plat_FloatTime();

        SPLITPACKET* header = (SPLITPACKET*)buffer;

        int sequence = LittleLong(header->sequenceNumber);

        bool compressed = (sequence < 0);

        if (compressed)
            sequence &= 0x7FFFFFFF;

        short packetID = LittleShort(header->packetID);

        int packetNumber = packetID >> 8;
        int packetCount = packetID & 0xFF;

        if (packetCount <= 0 || packetNumber > 69 || packetCount > 69)
        {
            Warning(
                "Split packet from %s has invalid packet count (%d/%d)\n",
                adr.ToString(),
                packetNumber,
                packetCount);

            return EReceivedInvalidPacket;
        }

        // Start assembling new packet
        if (packet.currentSequence != sequence)
        {
            memset(&packet, 0, sizeof(packet));
            memset(entry->flags, 0, sizeof(entry->flags));

            packet.currentSequence = sequence;
            packet.splitCount = packetCount;
        }

        // Copy payload
        int payloadSize = bytes - sizeof(SPLITPACKET);

        if (entry->flags[packetNumber] != sequence)
        {
            entry->flags[packetNumber] = sequence;

            packet.splitCount--;

            if (packetNumber == packetCount - 1)
            {
                packet.totalSize =
                    packetNumber * SPLIT_SIZE + payloadSize;
            }

            if (sb_sock_debugging.GetBool())
            {
                Msg("<-- Split packet %i of %i, seq %i, size %i from %s\n",
                    packetNumber + 1,
                    packetCount,
                    sequence,
                    payloadSize,
                    adr.ToString());
            }

            int offset = packetNumber * SPLIT_SIZE;

            if (offset + payloadSize > sizeof(packet.buffer) || payloadSize <= 0)
            {
                RemoveSplitPacket(adr);
                return EReceivedInvalidPacket;
            }

            memcpy(
                packet.buffer + packetNumber * SPLIT_SIZE,
                buffer + sizeof(SPLITPACKET),
                payloadSize);
        }


        // Waiting for remaining fragments
        if (packet.splitCount > 0)
            return EReceivedSplitPacket;

        // Packet complete
        bytes = packet.totalSize;

        if (bytes > sizeof(packet.buffer))
        {
            Warning(
                "Split packet from %s is too large (%d bytes)\n",
                adr.ToString(),
                bytes);

            RemoveSplitPacket(adr);

            return EReceivedInvalidPacket;
        }

        if (!compressed)
        {
            memcpy(
                buffer,
                packet.buffer,
                bytes);
        }
        else
        {
            SPLITPACKET_COMPRESSED* cmp =
                (SPLITPACKET_COMPRESSED*)packet.buffer;

            CUtlMemory<byte> decompressed;
            decompressed.EnsureCapacity(NET_MAX_MESSAGE);

            unsigned int outSize = NET_MAX_MESSAGE;

            int result =
                BZ2_bzBuffToBuffDecompress(
                    (char*)decompressed.Base(),
                    &outSize,
                    (char*)packet.buffer + sizeof(SPLITPACKET_COMPRESSED),
                    bytes - sizeof(SPLITPACKET_COMPRESSED),
                    0,
                    0);

            if (result != BZ_OK)
            {
                Warning(
                    "Failed to decompress packet from %s (bz2 error %d)\n",
                    adr.ToString(),
                    result);

                RemoveSplitPacket(adr);

                return EReceivedInvalidPacket;
            }

            if (outSize != cmp->decompressedSize)
            {
                Warning(
                    "Decompressed packet has invalid size (%u != %u)\n",
                    outSize,
                    cmp->decompressedSize);

                RemoveSplitPacket(adr);

                return EReceivedInvalidPacket;
            }

            if (CRC32_ProcessSingleBuffer(
                (char*)decompressed.Base(),
                outSize) != cmp->crc)
            {
                Warning(
                    "Split packet CRC mismatch from %s\n",
                    adr.ToString());

                RemoveSplitPacket(adr);

                return EReceivedInvalidPacket;
            }

            memcpy(buffer, (char*)decompressed.Base(), outSize);

            bytes = outSize;
        }

        // Ready for next split packet
        RemoveSplitPacket(adr);
    }
    else if (*(int*)buffer != CONNECTIONLESS_HEADER)
    {
        return EReceivedInvalidPacket;
    }

    *outsize = bytes;
    memcpy(buf, buffer, bytes);

    if (*(int*)buffer == CONNECTIONLESS_HEADER && sb_sock_debugging.GetBool())
    {
        Msg("<-- Connectionless packet, size %i from %s\n",
            bytes,
            adr.ToString());
    } 

    // finished
    return EReceivedSinglePacket;
}

/*
 * Find split packet, if nothing was found, create one
*/
splitpacket_t* CSocket::FindOrCreateSplitPacket(const netadr_t& adr)
{
    int idx = m_SplitPackets.Find(adr);

    if (!m_SplitPackets.IsValidIndex(idx))
    {
        idx = m_SplitPackets.Insert(adr);
        memset(&m_SplitPackets[idx], 0, sizeof(splitpacket_t));
    }

    return &m_SplitPackets[idx];
}

/*
 * Remove split packet
*/
void CSocket::RemoveSplitPacket(const netadr_t& adr)
{
    int idx = m_SplitPackets.Find(adr);

    if (m_SplitPackets.IsValidIndex(idx))
        m_SplitPackets.RemoveAt(idx);
}

/*
 * Cleanup split packets
*/
void CSocket::CleanupSplitPackets()
{
    double curtime = Plat_FloatTime();

    unsigned short idx = m_SplitPackets.FirstInorder();

    while (m_SplitPackets.IsValidIndex(idx))
    {
        unsigned short next = m_SplitPackets.NextInorder(idx);

        if (curtime - m_SplitPackets[idx].lastReceiveTime > 5.0)
        {
            Warning("Dropping incomplete split packet from %s\n",
                m_SplitPackets.Key(idx).ToString());

            m_SplitPackets.RemoveAt(idx);
        }

        idx = next;
    }
}