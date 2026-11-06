//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "cbase.h"
#include "c_smoke_trail.h"
#include "fx.h"
#include "engine/ivdebugoverlay.h"
#include "engine/IEngineSound.h"
#include "c_te_effect_dispatch.h"
#include "glow_overlay.h"
#include "fx_explosion.h"
#include "tier1/KeyValues.h"
#include "toolframework_client.h"
#include "view.h"
#include "clienteffectprecachesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//
// CRocketTrailParticle
//

class CRocketTrailParticle : public CSimpleEmitter
{
public:
	
	CRocketTrailParticle( const char *pDebugName ) : CSimpleEmitter( pDebugName ) {}
	
	//Create
	static CRocketTrailParticle *Create( const char *pDebugName )
	{
		return new CRocketTrailParticle( pDebugName );
	}

	//Roll
	virtual	float UpdateRoll( SimpleParticle *pParticle, float timeDelta )
	{
		pParticle->m_flRoll += pParticle->m_flRollDelta * timeDelta;
		
		pParticle->m_flRollDelta += pParticle->m_flRollDelta * ( timeDelta * -8.0f );

		//Cap the minimum roll
		if ( fabs( pParticle->m_flRollDelta ) < 0.5f )
		{
			pParticle->m_flRollDelta = ( pParticle->m_flRollDelta > 0.0f ) ? 0.5f : -0.5f;
		}

		return pParticle->m_flRoll;
	}

	//Alpha
	virtual float UpdateAlpha( const SimpleParticle *pParticle )
	{
		return ( ((float)pParticle->m_uchStartAlpha/255.0f) * sin( M_PI * (pParticle->m_flLifetime / pParticle->m_flDieTime) ) );
	}

private:
	CRocketTrailParticle( const CRocketTrailParticle & );
};

//
// CSmokeParticle
//

class CSmokeParticle : public CSimpleEmitter
{
public:
	
	CSmokeParticle( const char *pDebugName ) : CSimpleEmitter( pDebugName ) {}
	
	//Create
	static CSmokeParticle *Create( const char *pDebugName )
	{
		return new CSmokeParticle( pDebugName );
	}

	//Alpha
	virtual float UpdateAlpha( const SimpleParticle *pParticle )
	{
		return ( ((float)pParticle->m_uchStartAlpha/255.0f) * sin( M_PI * (pParticle->m_flLifetime / pParticle->m_flDieTime) ) );
	}

	//Color
	virtual Vector UpdateColor( const SimpleParticle *pParticle )
	{
		Vector	color;

		float	tLifetime = pParticle->m_flLifetime / pParticle->m_flDieTime;
		float	ramp = 1.0f - tLifetime;

		color[0] = ( (float) pParticle->m_uchColor[0] * ramp ) / 255.0f;
		color[1] = ( (float) pParticle->m_uchColor[1] * ramp ) / 255.0f;
		color[2] = ( (float) pParticle->m_uchColor[2] * ramp ) / 255.0f;

		return color;
	}

	//Roll
	virtual	float UpdateRoll( SimpleParticle *pParticle, float timeDelta )
	{
		pParticle->m_flRoll += pParticle->m_flRollDelta * timeDelta;
		
		pParticle->m_flRollDelta += pParticle->m_flRollDelta * ( timeDelta * -8.0f );

		//Cap the minimum roll
		if ( fabs( pParticle->m_flRollDelta ) < 0.5f )
		{
			pParticle->m_flRollDelta = ( pParticle->m_flRollDelta > 0.0f ) ? 0.5f : -0.5f;
		}

		return pParticle->m_flRoll;
	}

private:
	CSmokeParticle( const CSmokeParticle & );
};

// Datatable.. this can have all the smoketrail parameters when we need it to.
IMPLEMENT_CLIENTCLASS_DT(C_SmokeTrail, DT_SmokeTrail, SmokeTrail)
	RecvPropFloat(RECVINFO(m_SpawnRate)),
	RecvPropVector(RECVINFO(m_StartColor)),
	RecvPropVector(RECVINFO(m_EndColor)),
	RecvPropFloat(RECVINFO(m_ParticleLifetime)),
	RecvPropFloat(RECVINFO(m_StopEmitTime)),
	RecvPropFloat(RECVINFO(m_MinSpeed)),
	RecvPropFloat(RECVINFO(m_MaxSpeed)),
	RecvPropFloat(RECVINFO(m_MinDirectedSpeed)),
	RecvPropFloat(RECVINFO(m_MaxDirectedSpeed)),
	RecvPropFloat(RECVINFO(m_StartSize)),
	RecvPropFloat(RECVINFO(m_EndSize)),
	RecvPropFloat(RECVINFO(m_SpawnRadius)),
	RecvPropInt(RECVINFO(m_bEmit)),
	RecvPropInt(RECVINFO(m_nAttachment)),	
	RecvPropFloat(RECVINFO(m_Opacity)),
END_RECV_TABLE()

// ------------------------------------------------------------------------- //
// ParticleMovieExplosion
// ------------------------------------------------------------------------- //
C_SmokeTrail::C_SmokeTrail()
{
	m_MaterialHandle[0] = NULL;
	m_MaterialHandle[1] = NULL;

	m_SpawnRate = 10;
	m_ParticleSpawn.Init(10);
	m_StartColor.Init(0.5, 0.5, 0.5);
	m_EndColor.Init(0,0,0);
	m_ParticleLifetime = 5;
	m_StopEmitTime = 0;	// No end time
	m_MinSpeed = 2;
	m_MaxSpeed = 4;
	m_MinDirectedSpeed = m_MaxDirectedSpeed = 0;
	m_StartSize = 35;
	m_EndSize = 55;
	m_SpawnRadius = 2;
	m_VelocityOffset.Init();
	m_Opacity = 0.5f;

	m_bEmit = true;

	m_nAttachment	= -1;

	m_pSmokeEmitter = NULL;
	m_pParticleMgr	= NULL;
}

C_SmokeTrail::~C_SmokeTrail()
{
	if ( ToolsEnabled() && clienttools->IsInRecordingMode() && m_pSmokeEmitter.IsValid() && m_pSmokeEmitter->GetToolParticleEffectId() != TOOLPARTICLESYSTEMID_INVALID )
	{
		KeyValues *msg = new KeyValues( "OldParticleSystem_ActivateEmitter" );
		msg->SetInt( "id", m_pSmokeEmitter->GetToolParticleEffectId() );
		msg->SetInt( "emitter", 0 );
		msg->SetInt( "active", false );
		msg->SetFloat( "time", gpGlobals->curtime );
		ToolFramework_PostToolMessage( HTOOLHANDLE_INVALID, msg );
		msg->deleteThis();
	}

	if ( m_pParticleMgr )
	{
		m_pParticleMgr->RemoveEffect( &m_ParticleEffect );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_SmokeTrail::GetAimEntOrigin( IClientEntity *pAttachedTo, Vector *pAbsOrigin, QAngle *pAbsAngles )
{
	C_BaseEntity *pEnt = pAttachedTo->GetBaseEntity();
	if (pEnt && (m_nAttachment > 0))
	{
		pEnt->GetAttachment( m_nAttachment, *pAbsOrigin, *pAbsAngles );
	}
	else
	{
		BaseClass::GetAimEntOrigin( pAttachedTo, pAbsOrigin, pAbsAngles );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bEmit - 
//-----------------------------------------------------------------------------
void C_SmokeTrail::SetEmit(bool bEmit)
{
	m_bEmit = bEmit;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : rate - 
//-----------------------------------------------------------------------------
void C_SmokeTrail::SetSpawnRate(float rate)
{
	m_SpawnRate = rate;
	m_ParticleSpawn.Init(rate);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bnewentity - 
//-----------------------------------------------------------------------------
void C_SmokeTrail::OnDataChanged(DataUpdateType_t updateType)
{
	C_BaseEntity::OnDataChanged(updateType);

	if ( updateType == DATA_UPDATE_CREATED )
	{
		Start( ParticleMgr(), NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pParticleMgr - 
//			*pArgs - 
//-----------------------------------------------------------------------------
void C_SmokeTrail::Start( CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs )
{
	if(!pParticleMgr->AddEffect( &m_ParticleEffect, this ))
		return;

	m_pParticleMgr	= pParticleMgr;
	m_pSmokeEmitter = CSmokeParticle::Create("smokeTrail");
	
	if ( !m_pSmokeEmitter )
	{
		Assert( false );
		return;
	}

	m_pSmokeEmitter->SetSortOrigin( GetAbsOrigin() );
	m_pSmokeEmitter->SetNearClip( 64.0f, 128.0f );

	m_MaterialHandle[0] = g_Mat_DustPuff[0];
	m_MaterialHandle[1] = g_Mat_DustPuff[1];
	
	m_ParticleSpawn.Init( m_SpawnRate );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : fTimeDelta - 
//-----------------------------------------------------------------------------
void C_SmokeTrail::Update( float fTimeDelta )
{
	if ( !m_pSmokeEmitter )
		return;

	Vector	offsetColor;

	// Add new particles
	if ( !m_bEmit )
		return;

	if ( ( m_StopEmitTime != 0 ) && ( m_StopEmitTime <= gpGlobals->curtime ) )
		return;

	float tempDelta = fTimeDelta;

	SimpleParticle	*pParticle;
	Vector			offset;

	Vector vecOrigin;
	VectorMA( GetAbsOrigin(), -fTimeDelta, GetAbsVelocity(), vecOrigin );

	Vector vecForward;
	GetVectors( &vecForward, NULL, NULL );

	while( m_ParticleSpawn.NextEvent( tempDelta ) )
	{
		float fldt = fTimeDelta - tempDelta;

		offset.Random( -m_SpawnRadius, m_SpawnRadius );
		offset += vecOrigin;
		VectorMA( offset, fldt, GetAbsVelocity(), offset );

		pParticle = (SimpleParticle *) m_pSmokeEmitter->AddParticle( sizeof( SimpleParticle ), m_MaterialHandle[random->RandomInt(0,1)], offset );

		if ( pParticle == NULL )
			continue;

		pParticle->m_flLifetime		= 0.0f;
		pParticle->m_flDieTime		= m_ParticleLifetime;

		pParticle->m_vecVelocity.Random( -1.0f, 1.0f );
		pParticle->m_vecVelocity *= random->RandomFloat( m_MinSpeed, m_MaxSpeed );

		pParticle->m_vecVelocity = pParticle->m_vecVelocity + GetAbsVelocity();
		
		float flDirectedVel = random->RandomFloat( m_MinDirectedSpeed, m_MaxDirectedSpeed );
		VectorMA( pParticle->m_vecVelocity, flDirectedVel, vecForward, pParticle->m_vecVelocity );

		offsetColor = m_StartColor;
		float flMaxVal = MAX( m_StartColor[0], m_StartColor[1] );
		if ( flMaxVal < m_StartColor[2] )
		{
			flMaxVal = m_StartColor[2];
		}
		offsetColor /= flMaxVal;

		offsetColor *= random->RandomFloat( -0.2f, 0.2f );
		offsetColor += m_StartColor;

		offsetColor[0] = clamp( offsetColor[0], 0.0f, 1.0f );
		offsetColor[1] = clamp( offsetColor[1], 0.0f, 1.0f );
		offsetColor[2] = clamp( offsetColor[2], 0.0f, 1.0f );

		pParticle->m_uchColor[0]	= offsetColor[0]*255.0f;
		pParticle->m_uchColor[1]	= offsetColor[1]*255.0f;
		pParticle->m_uchColor[2]	= offsetColor[2]*255.0f;
		
		pParticle->m_uchStartSize	= m_StartSize;
		pParticle->m_uchEndSize		= m_EndSize;
		
		float alpha = random->RandomFloat( m_Opacity*0.75f, m_Opacity*1.25f );
		alpha = clamp( alpha, 0.0f, 1.0f );

		pParticle->m_uchStartAlpha	= alpha * 255; 
		pParticle->m_uchEndAlpha	= 0;
			
		pParticle->m_flRoll			= random->RandomInt( 0, 360 );
		pParticle->m_flRollDelta	= random->RandomFloat( -1.0f, 1.0f );
    }
}


void C_SmokeTrail::RenderParticles( CParticleRenderIterator *pIterator )
{
}


void C_SmokeTrail::SimulateParticles( CParticleSimulateIterator *pIterator )
{
}


//-----------------------------------------------------------------------------
// This is called after sending this entity's recording state
//-----------------------------------------------------------------------------

void C_SmokeTrail::CleanupToolRecordingState( KeyValues *msg )
{
	if ( !ToolsEnabled() )
		return;

	BaseClass::CleanupToolRecordingState( msg );

	// Generally, this is used to allow the entity to clean up
	// allocated state it put into the message, but here we're going
	// to use it to send particle system messages because we
	// know the grenade has been recorded at this point
	if ( !clienttools->IsInRecordingMode() || !m_pSmokeEmitter.IsValid() )
		return;
	
	// For now, we can't record smoketrails that don't have a moveparent
	C_BaseEntity *pEnt = GetMoveParent();
	if ( !pEnt )
		return;

	bool bEmitterActive = m_bEmit && ( ( m_StopEmitTime == 0 ) || ( m_StopEmitTime > gpGlobals->curtime ) );

	// NOTE: Particle system destruction message will be sent by the particle effect itself.
	if ( m_pSmokeEmitter->GetToolParticleEffectId() == TOOLPARTICLESYSTEMID_INVALID )
	{
		int nId = m_pSmokeEmitter->AllocateToolParticleEffectId();

		KeyValues *msg = new KeyValues( "OldParticleSystem_Create" );
		msg->SetString( "name", "C_SmokeTrail" );
		msg->SetInt( "id", nId );
		msg->SetFloat( "time", gpGlobals->curtime );

		KeyValues *pRandomEmitter = msg->FindKey( "DmeRandomEmitter", true );
		pRandomEmitter->SetInt( "count", m_SpawnRate );	// particles per second, when duration is < 0
		pRandomEmitter->SetFloat( "duration", -1 );
		pRandomEmitter->SetInt( "active", bEmitterActive );

		KeyValues *pEmitterParent1 = pRandomEmitter->FindKey( "emitter1", true );
		pEmitterParent1->SetFloat( "randomamount", 0.5f );
		KeyValues *pEmitterParent2 = pRandomEmitter->FindKey( "emitter2", true );
		pEmitterParent2->SetFloat( "randomamount", 0.5f );

		KeyValues *pEmitter = pEmitterParent1->FindKey( "DmeSpriteEmitter", true );
		pEmitter->SetString( "material", "particle/particle_smokegrenade" );

		KeyValues *pInitializers = pEmitter->FindKey( "initializers", true );

		// FIXME: Until we can interpolate ent logs during emission, this can't work
		KeyValues *pPosition = pInitializers->FindKey( "DmePositionPointToEntityInitializer", true );
		pPosition->SetPtr( "entindex", (void*)(intp)pEnt->entindex() );
		pPosition->SetInt( "attachmentIndex", m_nAttachment );
		pPosition->SetFloat( "randomDist", m_SpawnRadius );
		pPosition->SetFloat( "startx", pEnt->GetAbsOrigin().x );
		pPosition->SetFloat( "starty", pEnt->GetAbsOrigin().y );
		pPosition->SetFloat( "startz", pEnt->GetAbsOrigin().z );

		KeyValues *pLifetime = pInitializers->FindKey( "DmeRandomLifetimeInitializer", true );
		pLifetime->SetFloat( "minLifetime", m_ParticleLifetime );
 		pLifetime->SetFloat( "maxLifetime", m_ParticleLifetime );

		KeyValues *pVelocity = pInitializers->FindKey( "DmeAttachmentVelocityInitializer", true );
		pVelocity->SetPtr( "entindex", (void*)(intp)entindex() );
		pVelocity->SetFloat( "minAttachmentSpeed", m_MinDirectedSpeed );
 		pVelocity->SetFloat( "maxAttachmentSpeed", m_MaxDirectedSpeed );
 		pVelocity->SetFloat( "minRandomSpeed", m_MinSpeed );
 		pVelocity->SetFloat( "maxRandomSpeed", m_MaxSpeed );

		KeyValues *pRoll = pInitializers->FindKey( "DmeRandomRollInitializer", true );
		pRoll->SetFloat( "minRoll", 0.0f );
 		pRoll->SetFloat( "maxRoll", 360.0f );

		KeyValues *pRollSpeed = pInitializers->FindKey( "DmeRandomRollSpeedInitializer", true );
		pRollSpeed->SetFloat( "minRollSpeed", -1.0f );
 		pRollSpeed->SetFloat( "maxRollSpeed", 1.0f );

		KeyValues *pColor = pInitializers->FindKey( "DmeRandomValueColorInitializer", true );
		Color c(
			FastFToC( clamp( m_StartColor.x, 0.f, 1.f ) ),
			FastFToC( clamp( m_StartColor.y, 0.f, 1.f ) ),
			FastFToC( clamp( m_StartColor.z, 0.f, 1.f ) ),
			255 );
		pColor->SetColor( "startColor", c );
		pColor->SetFloat( "minStartValueDelta", -0.2f );
 		pColor->SetFloat( "maxStartValueDelta", 0.2f );
		pColor->SetColor( "endColor", Color( 0, 0, 0, 255 ) );

		KeyValues *pAlpha = pInitializers->FindKey( "DmeRandomAlphaInitializer", true );
		int nMinAlpha = 255 * m_Opacity * 0.75f;
		int nMaxAlpha = 255 * m_Opacity * 1.25f;
		pAlpha->SetInt( "minStartAlpha", 0 );
		pAlpha->SetInt( "maxStartAlpha", 0 );
		pAlpha->SetInt( "minEndAlpha", clamp( nMinAlpha, 0, 255 ) );
		pAlpha->SetInt( "maxEndAlpha", clamp( nMaxAlpha, 0, 255 ) );

		KeyValues *pSize = pInitializers->FindKey( "DmeRandomSizeInitializer", true );
		pSize->SetFloat( "minStartSize", m_StartSize );
		pSize->SetFloat( "maxStartSize", m_StartSize );
		pSize->SetFloat( "minEndSize", m_EndSize );
		pSize->SetFloat( "maxEndSize", m_EndSize );

		KeyValues *pUpdaters = pEmitter->FindKey( "updaters", true );
	    
		pUpdaters->FindKey( "DmePositionVelocityUpdater", true );
		pUpdaters->FindKey( "DmeRollUpdater", true );

		KeyValues *pRollSpeedUpdater = pUpdaters->FindKey( "DmeRollSpeedAttenuateUpdater", true );
		pRollSpeedUpdater->SetFloat( "attenuation", 1.0f - 8.0f / 30.0f );
		pRollSpeedUpdater->SetFloat( "attenuationTme", 1.0f / 30.0f );
		pRollSpeedUpdater->SetFloat( "minRollSpeed", 0.5f );

		pUpdaters->FindKey( "DmeAlphaSineUpdater", true );
		pUpdaters->FindKey( "DmeColorUpdater", true );
		pUpdaters->FindKey( "DmeSizeUpdater", true );

		KeyValues *pEmitter2 = pEmitter->MakeCopy();
		pEmitter2->SetString( "material", "particle/particle_noisesphere" );
		pEmitterParent2->AddSubKey( pEmitter2 );

		ToolFramework_PostToolMessage( HTOOLHANDLE_INVALID, msg );
		msg->deleteThis();
	}
	else 
	{
		KeyValues *msg = new KeyValues( "OldParticleSystem_ActivateEmitter" );
		msg->SetInt( "id", m_pSmokeEmitter->GetToolParticleEffectId() );
		msg->SetInt( "emitter", 0 );
		msg->SetInt( "active", bEmitterActive );
		msg->SetFloat( "time", gpGlobals->curtime );
		ToolFramework_PostToolMessage( HTOOLHANDLE_INVALID, msg );
		msg->deleteThis();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void RPGShotDownCallback( const CEffectData &data )
{
	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, "Missile.ShotDown", &data.m_vOrigin );

	if ( CExplosionOverlay *pOverlay = new CExplosionOverlay )
	{
		pOverlay->m_flLifetime	= 0;
		pOverlay->m_vPos		= data.m_vOrigin;
		pOverlay->m_nSprites	= 1;
		
		pOverlay->m_vBaseColors[0].Init( 1.0f, 0.9f, 0.7f );

		pOverlay->m_Sprites[0].m_flHorzSize = 0.01f;
		pOverlay->m_Sprites[0].m_flVertSize = pOverlay->m_Sprites[0].m_flHorzSize*0.5f;

		pOverlay->Activate();
	}
}

DECLARE_CLIENT_EFFECT( "RPGShotDown", RPGShotDownCallback );
