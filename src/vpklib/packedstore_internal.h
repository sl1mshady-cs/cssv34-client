//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
// 
// VDC - https://developer.valvesoftware.com/wiki/VPK_File_Format#Header
//
//===========================================================================//

#ifndef _PACKEDSTORE_INTERNAL
#define _PACKEDSTORE_INTERNAL
#pragma once

#include "libzstd/zstd.h"
#include "libzstd/zdict.h"
#include "vpklib/packedstore.h"

// if a chunk refers to this file number, it is data embedded in the same file as the directory block.
#define VPKFILENUMBER_EMBEDDED_IN_DIR_FILE  0x7fff

#define VPK_HEADER_MARKER 0x55aa1234	// significes that this is a new vpk header format
#define VPK_COMPRESSED_VERSION 54		// marks that this is a ZStandard-Compressed vpk 
#define VPK_COMPRESSED_VERSION2 154	
#define VPK_CURRENT_VERSION 2
#define VPK_PREVIOUS_VERSION 1

struct VPKDirHeader_t
{
	int32 m_nHeaderMarker;	// VPK_HEADER_MARKER
	int32 m_nVersion;		// VPK_VERSION

	// Size in bytes of the directory tree
	int32 m_nDirectorySize;

	// How many bytes of file content are stored in this VPK file
	int32 m_nEmbeddedChunkSize;

	// Size of the section containing MD5 checksums for external archive content
	int32 m_nChunkHashesSize;

	// Size of the section containing MD5 checksums for content in this file (should always be 48)
	int32 m_nSelfHashesSize;

	// Size of the section containing the public key and signature. This is 296 in CS:S
	int32 m_nSignatureSize;

	VPKDirHeader_t( void )
	{
		m_nHeaderMarker = VPK_HEADER_MARKER;
		m_nVersion = VPK_CURRENT_VERSION;
		m_nDirectorySize = 0;
		m_nEmbeddedChunkSize = 0;
		m_nChunkHashesSize = 0;
		m_nSelfHashesSize = 0;
		m_nSignatureSize = 0;
	}

	uint32 ComputeSizeofSignedDataAfterHeader() const
	{
		return m_nDirectorySize + m_nEmbeddedChunkSize + m_nChunkHashesSize + m_nSelfHashesSize;
	}

};

struct VPKDirHeaderOld_t
{
	int32 m_nHeaderMarker;	// VPK_HEADER_MARKER
	int32 m_nVersion;		// VPK_VERSION

	// Size in bytes of the directory tree
	int32 m_nDirectorySize;

	VPKDirHeaderOld_t( void )
	{
		m_nHeaderMarker = VPK_HEADER_MARKER;
		m_nVersion = VPK_PREVIOUS_VERSION;
		m_nDirectorySize = 0;
	}

};


#endif // _PACKEDSTORE_INTERNAL