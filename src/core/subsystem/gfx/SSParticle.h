/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <gfx/particle/ParticleEngine.h>
#include "../../CallbackMessage.h"
#include <gfx/DebugRenderer.h>
#include "../../../core/datadriven/ComponentTypes.h"

// Conveniency access function
#define g_SSParticle SSParticle::GetInstance()

#define PARTICLE_EXPLOSION_HEIGHT_OFFSET 1.0

enum PARTICLE_TYPE
{
	PARTICLE_TYPE_EXPLODING,
	PARTICLE_TYPE_SMOKE_CLOUD,
	PARTICLE_TYPE_SMOKE_TRAIL,
	PARTICLE_TYPE_LASER,
	PARTICLE_TYPE_FIRE_BALL,
	PARTICLE_TYPE_CONE,
	PARTICLE_TYPE_BULLET,

	PARTICLE_TYPE_SIZE,
};

class SSParticle : public Subsystem
{
public:
	static SSParticle& GetInstance();

	void Startup( ) override;
	void UpdateUserLayer( const float deltaTime ) override;
	void Shutdown( ) override;

	gfx::ParticleSystem* SpawnParticleSystem( PARTICLE_TYPE particleType, gfx::ParticleSystem& particleSystem, bool fogOfWarCull = true );
	const gfx::ParticleSystem& GetDefaultParticleSystem( PARTICLE_TYPE particleType ) const;

	void EntityKilled( CallbackMessage* message );
	void DamageEffect(Entity target);

private:
	// No external instancing allowed
	SSParticle ( ) : Subsystem( "Particle" ) {}
	SSParticle ( const SSParticle & rhs );
	~SSParticle ( ) {};
	SSParticle& operator=(const SSParticle & rhs);
	bool DoCullParticleSystem( const gfx::ParticleSystem& particleSystem );

	gfx::ParticleEngine 		m_ParticleEngine;
	CallbackMessageRegisterID 	m_CallbackMessageEntityKilledID;
	rVector<gfx::Texture*> 		m_Textures;
	gfx::ParticleTypeIdentifier	m_ParticleTypes[PARTICLE_TYPE_SIZE];
};
