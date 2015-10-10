/**************************************************
Copyright 2015 David Pejtersen
***************************************************/

#pragma once

#include <gfx/LightEngine.h>

#include "../Subsystem.h"
#include "../gfx/SSParticle.h"
#include "../../component/WeaponComponent.h"
#include "../../component/UpgradeComponent.h"
#include "../../ai/Agent.h"
#include "../gfx/SSRender.h"

#include <glm/glm.hpp>

#define g_SSProjectiles SSProjectiles::GetInstance( )

class SSProjectiles : public Subsystem
{
private:
	struct Projectile;

public:
	static SSProjectiles&	GetInstance				( );

	void					Startup					( )	override;
	void					Shutdown				( )	override;

	void					UpdateSimLayer			( const float timestep  )	override;

	void					UpdateProjectileMovement( Projectile& it, const float deltaTime );

	void					CreateProjectile		( Entity shooter, Entity target );

	void					Render					( rMap<int, InstancedObject>& renderBucket );

private:
	/* no type */	SSProjectiles	( ) : Subsystem( "Projectiles" ) { }
	/* no type */	~SSProjectiles	( ) { }
	/* no type */	SSProjectiles	( const SSProjectiles& );
	SSProjectiles&	operator=		( const SSProjectiles& );

	void			SetWeaponPropertiesToProjectile		( WeaponComponent* weapon,	Projectile* projectile		);
	void			SetSpecificProjectileProperties		( Projectile* projectile,	const glm::vec3& distance	);

	void			DealDamage( Projectile& projectile );

	const rString m_DecalTextureNameExplosion = "explosion.png";

	rVector<Projectile>	m_Projectiles;

	short				m_TracerRoundMax = 6;
	short				m_TracerRoundCounter = 0;
};

struct SSProjectiles::Projectile
{
	PROJECTILE_TYPE			Type;
	bool					Active;

	glm::vec3				Position;
	glm::quat				Orientation;
	glm::vec3				Scale;
	glm::vec4				Color;

	glm::vec3				Origin;
	glm::vec3				Destination;
	glm::vec3				Velocity;

	float					TimeLeft;
	float					TimeToLive;

	float					Damage;
	float					Speed;
	float					AoE;

	Entity					Target;
	short					OwnerID;
	UPGRADE_NAME			Weapon;

	gfx::ParticleSystem*	Effect;
	//gfx::LightHandle		LightHandle;
	gfx::Light				Light;

	bool					Tracer;
	short					MissChance;
	
};
