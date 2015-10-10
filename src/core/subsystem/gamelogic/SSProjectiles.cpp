/**************************************************
Copyright 2015 David Pejtersen
***************************************************/

#include <script/ScriptEngine.h>

#include "SSProjectiles.h"
#include "SSUpgrades.h"
#include "../audio/SSAudio.h"

#include "../../datadriven/ComponentHelper.h"
#include "../../component/PlacementComponent.h"
#include "../../component/ColourComponent.h"
#include "../../component/OwnerComponent.h"
#include "../../component/ChildComponent.h"
#include "../../component/StealthComponent.h"
#include "../../utility/Alliances.h"
#include "../gfx/SSFogOfWar.h"
#include "../gfx/SSDecaling.h"
#include "../../EntityFactory.h"

#define GRAVITY_CONSTANT			(9.82f * 3.0f)
#define MODEL_PROJECTILE_UP			glm::vec3( 0.0f, 0.0f, 1.0f )
#define MODEL_BEAM_UP				glm::vec3( 0.0f, 0.0f, -1.0f )
#define RESERVE_PROJECTILE_COUNT	128

glm::quat RotationAlignment( const glm::vec3& currentUp, const glm::vec3& targetUp )
{
	glm::quat q;
	glm::vec3 c	= glm::cross( currentUp, targetUp );
	float d		= glm::dot( currentUp, targetUp );
	float s		= sqrtf( (1.0f + d) * 2.0f );
	float rs	= 1.0f / s;

	q.x = c.x * rs;
	q.y = c.y * rs;
	q.z = c.z * rs;
	q.w = s* 0.5f;

	return q;
}

SSProjectiles& SSProjectiles::GetInstance( )
{
	static SSProjectiles instance;
	return instance;
}

void SSProjectiles::Startup( )
{
	m_Projectiles.reserve( RESERVE_PROJECTILE_COUNT );
	for ( auto& it : m_Projectiles )
		it.Active = false;
}

void SSProjectiles::Shutdown( )
{
	m_Projectiles.clear();
}

void SSProjectiles::UpdateSimLayer( const float timestep )
{
	for ( auto& it : m_Projectiles )
	{
		it.TimeLeft -= timestep;
		if ( it.Active && it.TimeLeft < 0.0f )
		{
			it.Active = false;

			if ( it.Effect )
			{
				it.Effect->TimeToLive		= it.Effect->ParticlesTimeToLive;
				it.Effect->EmitterPosition	= glm::vec3( FLT_MIN );
				it.Effect = nullptr;
			}

			if ( it.Type == PROJECTILE_TYPE_GRENADE )
			{
				unsigned int entityID = 0;
				EntityMask agentFlag = GetDenseComponentFlag<AgentComponent>( );
				for ( auto& mask : g_EntityManager.GetEntityMasks( ) )
				{
					if ( mask & agentFlag )
					{
						PlacementComponent* pos = GetDenseComponent<PlacementComponent>( entityID );
						float lngth = glm::distance(pos->Position,it.Destination);
						if (lngth < it.AoE && !g_Alliances.IsAllied(GetDenseComponent<OwnerComponent>(entityID)->OwnerID, it.OwnerID))
						{
							it.Target = entityID;
							float damageSaved = it.Damage;
							it.Damage = it.Damage - lngth;

							if (it.Damage < 1)
								it.Damage = 1;

							DealDamage( it );
							it.Damage = damageSaved;
						}
							
					}
					++entityID;
				}

				gfx::ParticleSystem effect	= g_SSParticle.GetDefaultParticleSystem( PARTICLE_TYPE_FIRE_BALL );
				effect.EmitterPosition		= it.Destination + glm::vec3( 0, 1, 0 );
				effect.ParticlesSpeed		= ( it.AoE - effect.Size.x ) / effect.ParticlesTimeToLive;
				g_SSParticle.SpawnParticleSystem( PARTICLE_TYPE_FIRE_BALL, effect );

				///Play boomboom sound when hitting ground
				///TODO: Place this here?
				SFXEvent event;
				event.AtBeat = BeatType::NONE;
				event.Name = "../../../asset/audio/collection/misc/explosion.sfxc";
				event.Info3D.DistMin = 100.0f;
				event.Info3D.DistMax = 1000.0f;
				event.Info3D.Is3D = true;
				event.Info3D.Position = it.Destination;
				g_SSAudio.PostEventSFX(event);
				
				g_SSDecaling.AddTimedDecal( it.Position, m_DecalTextureNameExplosion.c_str( ), 15.0f, glm::vec4( 1.0f ), 3.0f, 2.0f );
			}
			else if ( it.Type == PROJECTILE_TYPE_CONE || it.Weapon == WEAPON_PIERCING_LASER )
			{
				unsigned int entityID = 0;
				EntityMask agentFlag = GetDenseComponentFlag<AgentComponent>( );
				for ( auto& mask : g_EntityManager.GetEntityMasks( ) )
				{
					if ( mask & agentFlag )
					{
						PlacementComponent* pos = GetDenseComponent<PlacementComponent>( entityID );

						float	distanceSquared;
						bool	inRange = false;

						// Calculate minimum distance from line segment
						{
							float lineLength = it.Scale.z;
							glm::vec3 lineStart = it.Origin;
							glm::vec3 lineEnd = lineLength * glm::normalize( it.Destination - it.Origin ) + it.Origin;
							glm::vec3 point = pos->Position;

							float dist2 = glm::length2( lineEnd - lineStart );
							float t = glm::dot( point - lineStart, lineEnd - lineStart ) / dist2;
							if ( t < 0.0f )
								distanceSquared = glm::length2( point - lineStart );
							else if ( t > 1.0f )
								distanceSquared = glm::length2( point - lineEnd );
							
							glm::vec3 projection = lineStart + t * (lineEnd - lineStart);

							distanceSquared = glm::length2( point - projection );

							if ( it.Type == PROJECTILE_TYPE_CONE && 0.0f <= t && t <= 1.0f )
								inRange = distanceSquared <= (it.AoE * t) * (it.AoE * t);
							else if ( it.Weapon == WEAPON_PIERCING_LASER )
								inRange = distanceSquared <= it.AoE * it.AoE;
						}

						if ( inRange && !g_Alliances.IsAllied( GetDenseComponent<OwnerComponent>( entityID )->OwnerID, it.OwnerID ) ) // TODODP: Move owner check one step out ?
						{
							it.Target = entityID;
							DealDamage( it );
						}
					}
					++entityID;
				}
			}
			else
			{
				DealDamage( it );
			}
		}
		else
		{
			UpdateProjectileMovement( it, timestep );
		}
	}
}

void SSProjectiles::UpdateProjectileMovement( Projectile& it,  const float deltaTime )
{
	if ( it.Active )
	{
		if ( it.Tracer )
			it.Light.Position = it.Position;

		if ( it.Type == PROJECTILE_TYPE_BULLET )
		{
			it.Position += it.Velocity * deltaTime;
			it.Light.Position = it.Position;
		}
		else if ( it.Type == PROJECTILE_TYPE_GRENADE )
		{
			glm::vec3 oldPos = it.Position;

			it.Velocity.y -= GRAVITY_CONSTANT * deltaTime;
			it.Position += it.Velocity * deltaTime;
			it.Light.Position = it.Position;

			it.Orientation = RotationAlignment( MODEL_PROJECTILE_UP, glm::normalize( it.Position - oldPos ) );

			if ( it.Effect )
			{
				it.Effect->EmitterPosition	= oldPos;
				it.Effect->Direction		= glm::normalize( oldPos - it.Position );
			}
		}
	}
}

void SSProjectiles::DealDamage( Projectile& projectile)
{
	float damageAmount = projectile.Damage;
	Entity target = projectile.Target;

	if ( HasComponent<HealthComponent>( target ) )
	{
		UpgradeComponent* upgrade = g_SSUpgrades.GetUnitUpgrade( target, MODULE_SLOT_BACK );
		if ( upgrade && upgrade->UpgradeDataID == UTILITY_PASSIVE_SHIELD )
		{
			float& shieldHealth = upgrade->Data.ShieldHealth;

			shieldHealth -= damageAmount;
			if ( shieldHealth <= 0.0f )
			{
				damageAmount = -shieldHealth;
				shieldHealth = 0.0f;
			}
			else
			{
				damageAmount = 0.0f;
			}
		}

		bool bounce = (static_cast<short>(g_Randomizer.SimRand(0, 100)) < projectile.MissChance);
		
		if (projectile.Tracer && bounce)
		{
			gfx::ParticleSystem effect = g_SSParticle.GetDefaultParticleSystem(PARTICLE_TYPE_BULLET);
			effect.ParticlesSpeed = projectile.Speed/2;
			effect.TimeToLive = 0.2f;
			effect.ParticlesTimeToLive = effect.TimeToLive;
			effect.EmitterPosition = projectile.Destination;// + (projectile.TimeToLive) * glm::normalize(projectile.Destination - projectile.Origin) * projectile.Speed;
			//effect.EmitterPosition = GetDenseComponent<PlacementComponent>(target)->Position;
			effect.Direction = glm::normalize(glm::vec3(static_cast<float>(g_Randomizer.UserRand(0, 1000)), static_cast<float>(g_Randomizer.UserRand(500, 1000)), static_cast<float>(g_Randomizer.UserRand(0, 1000))));
			g_SSParticle.SpawnParticleSystem(PARTICLE_TYPE_BULLET, effect);
		}
		else if ( projectile.Type == PROJECTILE_TYPE_BULLET )
		{
			//g_SSParticle.DamageEffect(target);
			GetDenseComponent<HealthComponent>(target)->OnAttacked(damageAmount);
		}
		else
		{
			GetDenseComponent<HealthComponent>( target )->OnAttacked( damageAmount );
		}

		if ( HasComponent<StealthComponent>( target ) )
			GetDenseComponent<StealthComponent>( target )->UnStealth( );
	}
}

void SSProjectiles::SetWeaponPropertiesToProjectile( WeaponComponent* weapon, Projectile* projectile )
{
	projectile->Weapon		= weapon->Name;
	projectile->Damage		= weapon->Damage;

	UpgradeData& upgrade	= g_SSUpgrades.GetUpgrade( weapon->Name );
	projectile->Type		= upgrade.Data.Weapon.ProjectileType;
	projectile->Speed		= upgrade.Data.Weapon.ProjectileSpeed;
	projectile->AoE			= upgrade.Data.Weapon.AreaOfEffect;


	projectile->Tracer		= (projectile->Type == PROJECTILE_TYPE_BULLET && g_Randomizer.SimRand(0, upgrade.Data.Weapon.TracerOccurence) == 0);
	projectile->MissChance	= upgrade.Data.Weapon.MissChance;
}

void SSProjectiles::SetSpecificProjectileProperties( Projectile* projectile, const glm::vec3& distance )
{
	// TODODP: TEMP
	//auto GetEffect = [] ( ) -> gfx::ParticleSystem
	//{
	//	ScriptArg args[17];
	//	g_Script.Run( "../../../script/Test.lua" );
	//	g_Script.GetArray( "ParticleTest", args, 17 );
	//	gfx::ParticleSystem ps	= g_SSParticle.GetDefaultParticleSystem( PARTICLE_TYPE_CONE );
	//	ps.EmitterPosition		= glm::vec3( args[0].asDouble, args[1].asDouble, args[2].asDouble );
	//	ps.TimeToLive			= args[3].asDouble;
	//	ps.Colour				= glm::vec4( args[4].asDouble, args[5].asDouble, args[6].asDouble, args[7].asDouble );
	//	ps.Direction			= glm::vec3( args[8].asDouble, args[9].asDouble, args[10].asDouble );
	//	ps.ParticlesSpeed		= args[11].asDouble;
	//	ps.Size					= glm::vec2( args[12].asDouble, args[13].asDouble );
	//	ps.ParticlesTimeToLive	= args[14].asDouble;
	//	ps.TransparencyPolynome	= glm::vec2( args[15].asDouble, args[16].asDouble );
	//	return ps;
	//};

	switch ( projectile->Type )
	{
		case PROJECTILE_TYPE_BULLET:
			projectile->Orientation = RotationAlignment( MODEL_PROJECTILE_UP, glm::normalize( projectile->Destination - projectile->Origin ) );

			//projectile->Scale.z = 1; // TODODP: Move hardcoded length
			projectile->Orientation = RotationAlignment(MODEL_BEAM_UP, glm::normalize(projectile->Destination - projectile->Origin));
			{


				if (projectile->Tracer)
				{
					gfx::ParticleSystem effect = g_SSParticle.GetDefaultParticleSystem(PARTICLE_TYPE_BULLET);
					effect.ParticlesSpeed = projectile->Speed;
					effect.TimeToLive = glm::length(distance) / projectile->Speed;
					effect.ParticlesTimeToLive = glm::length(distance) / projectile->Speed;
					effect.EmitterPosition = projectile->Origin + glm::vec3(0.0f, 0.5f, 0.0f);
					effect.Direction = glm::normalize(projectile->Destination - projectile->Origin) * projectile->Scale.z;
					effect.EmitterPosition = effect.EmitterPosition;

					if (projectile->Weapon == WEAPON_TANK_CANNON)
						effect.Size = glm::vec2(0.25f, 0.25f);
					else if (projectile->Weapon == WEAPON_MACHINE_GUN)
						effect.Size = glm::vec2(0.15f, 0.15f);

					g_SSParticle.SpawnParticleSystem(PARTICLE_TYPE_BULLET, effect);

					projectile->Light.Position = projectile->Position;
					projectile->Light.Range = 4;
					projectile->Light.Intensity = 15;
					projectile->Light.Color = vec4(1.0f, 0.2f, 0.2f, 1.0f);
				}
				else
				{
					projectile->Light.Position = projectile->Position;
					projectile->Light.Range = 7;
					projectile->Light.Intensity = 2;
					projectile->Light.Color = vec4(0.2f);
				}
			}

			break;

		case PROJECTILE_TYPE_GRENADE:
			projectile->Velocity.y	= -((projectile->Position.y - projectile->Destination.y) / projectile->TimeLeft - GRAVITY_CONSTANT * projectile->TimeLeft / 2.0f);
			{
				gfx::ParticleSystem effect	= g_SSParticle.GetDefaultParticleSystem( PARTICLE_TYPE_SMOKE_TRAIL );
				effect.EmitterPosition		= glm::vec3( FLT_MIN );
				projectile->Effect			= g_SSParticle.SpawnParticleSystem( PARTICLE_TYPE_SMOKE_TRAIL, effect );
			}
			{
				projectile->Light.Position		= projectile->Position;
				projectile->Light.Color		= glm::vec4( 1.0f );
				projectile->Light.Intensity	= 5.0f;
				projectile->Light.Range		= 100.0f;
			}
			break;

		case PROJECTILE_TYPE_BEAM:
			projectile->TimeLeft	= 0.1f;
			projectile->Scale.z		= projectile->Weapon == WEAPON_PIERCING_LASER ? 25.0f : glm::length( distance ); // TODODP: Move hardcoded length
			projectile->Orientation = RotationAlignment( MODEL_BEAM_UP, glm::normalize( projectile->Destination - projectile->Origin ) );
			projectile->Destination = projectile->Destination + glm::vec3( 0.0f, 0.5f, 0.0f );
			{
				gfx::ParticleSystem effect	= g_SSParticle.GetDefaultParticleSystem( PARTICLE_TYPE_LASER );
				effect.EmitterPosition = projectile->Origin;// +glm::vec3(0.0f, 0.5f, 0.0f);
				effect.Direction			= glm::normalize( projectile->Destination - projectile->Origin ) * projectile->Scale.z;

				if (projectile->Weapon == WEAPON_CUTTER)
				{
					effect.Colour = vec4(0.1f, 0.1f, 0.9f, 1);
					effect.Size = glm::vec2(0.1f, 0.1f);
				}

				g_SSParticle				.SpawnParticleSystem( PARTICLE_TYPE_LASER, effect );
				effect.EmitterPosition		= effect.EmitterPosition + glm::normalize( projectile->Destination - projectile->Origin ) * effect.Size.x / 2.0f;
				g_SSParticle				.SpawnParticleSystem( PARTICLE_TYPE_LASER, effect );

				if(projectile->Weapon == WEAPON_PIERCING_LASER)
				{
					projectile->Light = gfx::Light();
					projectile->Light.Position = projectile->Origin;
					projectile->Light.Direction = projectile->Destination - projectile->Origin;
					projectile->Light.Length = 25.0f; //glm::length( m_LineLight->Direction );
					projectile->Light.Range = 5;
					projectile->Light.Intensity = 3;
					projectile->Light.Color = vec4(0.9f,0.1f,0.1f,1);
				}

// 				if (projectile->Weapon == WEAPON_CUTTER)
// 				{
// 					projectile->Light.Direction = projectile->Destination - projectile->Origin;
// 					projectile->Light.Position = projectile->Destination;
// 					projectile->Light.Color = vec4(0.9f, 0.1f, 0.1f, 1);
// 					projectile->Light.Intensity = 500.0f;
// 					projectile->Light.Range = 100.0f;
// 					projectile->Light.Length = 25.0f;
// 				}
			}
			break;

		case PROJECTILE_TYPE_CONE:
			projectile->TimeLeft	= 0.2f;
			projectile->Scale.z		= g_SSUpgrades.GetUpgrade( WEAPON_PLASMA_SPEWER ).Data.Weapon.AreaOfEffect;
			projectile->Destination = projectile->Destination + glm::vec3( 0.0f, 0.5f, 0.0f );
			{
				gfx::ParticleSystem effect	= g_SSParticle.GetDefaultParticleSystem( PARTICLE_TYPE_CONE );
				effect.EmitterPosition		= projectile->Origin + glm::vec3( 0.0f, 0.5f, 0.0f );
				effect.Direction			= glm::normalize( projectile->Destination - projectile->Origin ) * projectile->Scale.z;
				effect.ParticlesSpeed		= projectile->AoE;
				g_SSParticle.SpawnParticleSystem( PARTICLE_TYPE_CONE, effect );
			}
			break;

		case PROJECTILE_TYPE_NONE:
		default:
			projectile->TimeLeft	= 0.0f;
			break;
	}
}

void SSProjectiles::CreateProjectile( Entity shooter, Entity target )
{
	Projectile* projectile = nullptr;

	for ( auto& it : m_Projectiles )
		if ( it.Active == false )
			projectile = &it;

	if ( projectile == nullptr )
	{
		//create new projectile
		m_Projectiles.push_back( Projectile( ) );
		projectile = &m_Projectiles.back( );
		projectile->Light = gfx::Light();
		projectile->Light.Position = glm::vec3( FLT_MIN );
	}
	else
	{
		//reuse projectile
		projectile->Light = gfx::Light();
		projectile->Light.Position = glm::vec3( FLT_MIN );
	}

	projectile->Effect = nullptr;

	SetWeaponPropertiesToProjectile( GetDenseComponent<WeaponComponent>( shooter ), projectile );

	projectile->Position = GetDenseComponent<PlacementComponent>( shooter )->Position;
	projectile->Scale		= glm::vec3( 1.0f );
	projectile->Color = GetDenseComponent<ColourComponent>( shooter )->Colour * 3.0f; // TODODP: Find another way of intensifying the color

	projectile->Origin		= projectile->Position;
	projectile->Destination = GetDenseComponent<PlacementComponent>( target )->Position;

	glm::vec3 distance		= projectile->Destination - projectile->Position;

	projectile->Velocity	= glm::normalize( distance ) * projectile->Speed;

	projectile->TimeLeft	= glm::length( distance ) / projectile->Speed;
	projectile->TimeToLive	= projectile->TimeLeft;

	projectile->Active		= true;
	projectile->Target		= target;
	projectile->OwnerID		= GetDenseComponent<OwnerComponent>( GetDenseComponent<ChildComponent>( shooter )->Parent )->OwnerID;

	SetSpecificProjectileProperties( projectile, distance );
}

void SSProjectiles::Render( rMap<int, InstancedObject>& renderBucket )
{
	for ( auto& it : m_Projectiles )
	{
		if ( it.Active )
		{
			if ( !g_SSFogOfWar.CalcVisibilityForPosition( it.Position ) )
			{
				continue;
			}

			if (/* it.Type == PROJECTILE_TYPE_BULLET || */it.Type == PROJECTILE_TYPE_GRENADE )
			{
				glm::mat4 world;
				int bucket = gfx::g_ModelBank.LoadModel( "artilleyShellShape.robo" );
				g_SSRender.CalculateWorldMatrix( it.Position, it.Scale, it.Orientation, world );
				renderBucket[bucket].worlds.push_back( world );
				renderBucket[bucket].colors.push_back( it.Color );
				if(it.Tracer)
					gfx::g_LightEngine.AddPointLightToQueue( it.Light );
			}
			else if ( it.Type == PROJECTILE_TYPE_BULLET )
			{
				if(g_SSFogOfWar.CalcVisibilityForPosition(it.Light.Position))
					gfx::g_LightEngine.AddPointLightToQueue( it.Light );
			}
			else if ( it.Type == PROJECTILE_TYPE_BEAM )
			{
				if( it.Weapon == WEAPON_PIERCING_LASER )
				{
					//check
					if(g_SSFogOfWar.CalcVisibilityForPosition(it.Light.Position) || g_SSFogOfWar.CalcVisibilityForPosition(it.Light.Position + it.Light.Direction * it.Light.Length))
						gfx::g_LightEngine.AddLineLightToQueue( it.Light );
				}
			//	glm::mat4 world;
			//	int bucket = gfx::g_ModelBank.LoadModel( "laserShape.robo" );
			//	g_SSRender.CalculateWorldMatrix( it.Position, it.Scale, it.Orientation, world );
			//	renderBucket[bucket].worlds.push_back( world );
			//	renderBucket[bucket].colors.push_back( it.Color );
			}
			//else if ( it.Type == PROJECTILE_TYPE_CONE )
			//{
			//	glm::mat4 world;
			//	int bucket = gfx::g_ModelBank.LoadModel( "laserShape.robo" );

			//	int nrOfBeams = 5;
			//	for ( int i = -nrOfBeams / 2; i <= nrOfBeams / 2; ++i )
			//	{
			//		glm::vec3 normal	= glm::vec3( it.Destination.z - it.Origin.z, 0.0f, it.Origin.x - it.Destination.x ); // Normal in 2D
			//		normal				= glm::normalize( normal );

			//		glm::quat orientation = RotationAlignment( MODEL_BEAM_UP, glm::normalize( (it.Destination + (float) i / (float) nrOfBeams * normal * it.AoE) - it.Origin ) );

			//		g_SSRender.CalculateWorldMatrix( it.Position, it.Scale, orientation, world );
			//		renderBucket[bucket].worlds.push_back( world );
			//		renderBucket[bucket].colors.push_back( it.Color );
			//	}
			//}
		}
	}
}
