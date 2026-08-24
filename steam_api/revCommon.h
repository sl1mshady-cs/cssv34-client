#ifndef _REVCOMMON_H
#define _REVCOMMON_H

#ifdef _WIN32
#pragma once
#endif

#ifndef STEAM_API_EXPORTS
#define STEAM_API_EXPORTS
#endif

// standard io
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstdint>

// common
#include "tier1/checksum_crc.h"
#include "tier1/strtools.h"
#include "murmur32.h"

// steam common
#include "userid.h"
#include "steam/steam_api.h"

// "rev"
#ifdef _WIN32
#define REVTICKET_SIGNATURE     'rev'
#else // gcc compat
#define REVTICKET_SIGNATURE		0x726576
#endif

// RevClient 9.83
#define REVTICKET_VERSION 83

// RevClient 9.74
#define REVTICKET_VERSION_74 74

// RevEmu Gen 2
#define REVTICKET_VERSION_46 46

// v3/v4 shared ticket (S3 164, REV2013 ???)
struct TRevTicket
{
	int			version;
	unsigned	hash;
	uint64		signature;
	uint64		steamID;	// TSteamSplitLocalUserID
	char		hwid[128];	// host id
};


// A bitwise hash function written by Justin Sobel
// * can be inlined
inline uint32 JSHash(const char* data, int size)
{
	uint32 hash = 1315423911;
	for (int i = 0; i < size; ++i)
	{
		hash ^= (hash << 5) + (hash >> 2) + data[i];
	}
	return hash;
}

#endif // _REVCOMMON_H