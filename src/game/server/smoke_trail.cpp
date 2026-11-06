//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "smoke_trail.h"
#include "dt_send.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SMOKETRAIL_ENTITYNAME		"env_smoketrail"

LINK_ENTITY_TO_CLASS(env_smoketrail, SmokeTrail);

BEGIN_DATADESC( SmokeTrail )

	DEFINE_FIELD( m_StartColor, FIELD_VECTOR ),
	DEFINE_FIELD( m_EndColor, FIELD_VECTOR ),
	DEFINE_KEYFIELD( m_Opacity, FIELD_FLOAT, "opacity" ),
	DEFINE_KEYFIELD( m_SpawnRate, FIELD_FLOAT, "spawnrate" ),
	DEFINE_KEYFIELD( m_ParticleLifetime, FIELD_FLOAT, "lifetime" ),
	DEFINE_FIELD( m_StopEmitTime, FIELD_TIME ),
	DEFINE_KEYFIELD( m_MinSpeed, FIELD_FLOAT, "minspeed" ),
	DEFINE_KEYFIELD( m_MaxSpeed, FIELD_FLOAT, "maxspeed" ),
	DEFINE_KEYFIELD( m_MinDirectedSpeed, FIELD_FLOAT, "mindirectedspeed" ),
	DEFINE_KEYFIELD( m_MaxDirectedSpeed, FIELD_FLOAT, "maxdirectedspeed" ),
	DEFINE_KEYFIELD( m_StartSize, FIELD_FLOAT, "startsize" ),
	DEFINE_KEYFIELD( m_EndSize, FIELD_FLOAT, "endsize" ),
	DEFINE_KEYFIELD( m_SpawnRadius, FIELD_FLOAT, "spawnradius" ),
	DEFINE_FIELD( m_bEmit, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nAttachment, FIELD_INTEGER ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
SmokeTrail::SmokeTrail()
{
	m_SpawnRate = 10;
	m_StartColor.GetForModify().Init(0.5, 0.5, 0.5);
	m_EndColor.GetForModify().Init(0,0,0);
	m_ParticleLifetime = 5;
	m_StopEmitTime = 0; // Don't stop emitting particles
	m_MinSpeed = 2;
	m_MaxSpeed = 4;
	m_MinDirectedSpeed = m_MaxDirectedSpeed = 0;
	m_StartSize = 35;
	m_EndSize = 55;
	m_SpawnRadius = 2;
	m_bEmit = true;
	m_nAttachment	= 0;
	m_Opacity = 0.5f;
}


//-----------------------------------------------------------------------------
// Parse data from a map file
//-----------------------------------------------------------------------------
bool SmokeTrail::KeyValue( const char *szKeyName, const char *szValue ) 
{
	if ( FStrEq( szKeyName, "startcolor" ) )
	{
		color32 tmp;
		UTIL_StringToColor32( &tmp, szValue );
		m_StartColor.GetForModify().Init( tmp.r / 255.0f, tmp.g / 255.0f, tmp.b / 255.0f );
		return true;
	}

	if ( FStrEq( szKeyName, "endcolor" ) )
	{
		color32 tmp;
		UTIL_StringToColor32( &tmp, szValue );
		m_EndColor.GetForModify().Init( tmp.r / 255.0f, tmp.g / 255.0f, tmp.b / 255.0f );
		return true;
	}

	if ( FStrEq( szKeyName, "emittime" ) )
	{
		m_StopEmitTime = gpGlobals->curtime + atof( szValue );
		return true;
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}


//-----------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//-----------------------------------------------------------------------------
void SmokeTrail::SetEmit(bool bVal)
{
	m_bEmit = bVal;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : SmokeTrail*
//-----------------------------------------------------------------------------
SmokeTrail* SmokeTrail::CreateSmokeTrail()
{
	CBaseEntity *pEnt = CreateEntityByName(SMOKETRAIL_ENTITYNAME);
	if(pEnt)
	{
		SmokeTrail *pSmoke = dynamic_cast<SmokeTrail*>(pEnt);
		if(pSmoke)
		{
			pSmoke->Activate();
			return pSmoke;
		}
		else
		{
			UTIL_Remove(pEnt);
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Attach the smoke trail to an entity or point 
// Input  : index - entity that has the attachment
//			attachment - point to attach to
//-----------------------------------------------------------------------------
void SmokeTrail::FollowEntity( CBaseEntity *pEntity, const char *pAttachmentName )
{
	// For attachments
	if ( pAttachmentName && pEntity && pEntity->GetBaseAnimating() )
	{
		m_nAttachment = pEntity->GetBaseAnimating()->LookupAttachment( pAttachmentName );
	}
	else
	{
		m_nAttachment = 0;
	}

	BaseClass::FollowEntity( pEntity );
}