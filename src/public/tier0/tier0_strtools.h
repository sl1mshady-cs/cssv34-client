//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef _TIER0_STRTOOLS_H
#define _TIER0_STRTOOLS_H

#ifdef _WIN32
#pragma once
#endif

#include "platform.h"

PLATFORM_INTERFACE int V_tier0_stricmp( const char *a, const char *b );
PLATFORM_INTERFACE void V_tier0_strncpy( char *a, const char *b, int n );
PLATFORM_INTERFACE char *V_tier0_strncat( char *a, const char *b, int n, int m = -1 );
PLATFORM_INTERFACE int V_tier0_vsnprintf( char *a, int n, PRINTF_FORMAT_STRING const char *f, va_list l ) FMTFUNCTION( 3, 0 );
PLATFORM_INTERFACE int V_tier0_snprintf( char *a, int n, PRINTF_FORMAT_STRING const char *f, ... ) FMTFUNCTION( 3, 4 );

#endif // _TIER0_STRTOOLS_H