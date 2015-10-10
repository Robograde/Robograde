/**************************************************
Copyright 2015 David Pejtersen
***************************************************/

#include "SSResourceManager.h"

#include <glm/gtx/transform.hpp>

#include <utility/Randomizer.h>
#include <gfx/DecalManager.h>
#include <gfx/GraphicsEngine.h>

#include "../../datadriven/EntityManager.h"
#include "../../datadriven/ComponentHelper.h"
#include "../../EntityFactory.h"
#include "../gui/SSMiniMap.h"
#include "../audio/SSAudio.h"

#define RESOURCE_DROP_SFX_PATH	"../../../asset/audio/collection/misc/resource_fall.sfxc"
#define RESOURCE_DROP_SFX_MIN	100.0f
#define RESOURCE_DROP_SFX_MAX	1000.0f


SSResourceManager& SSResourceManager::GetInstance( )
{
	static SSResourceManager instance;
	return instance;
}

void SSResourceManager::Startup( )
{
	// Rare -- Lots
	m_RespawnTypes[0].RespawnRate			= 300.0f;
	m_RespawnTypes[0].MaximumAmount			= 30;
	m_RespawnTypes[0].CurrentAmountHigh		= 20;
	m_RespawnTypes[0].CurrentAmountLittle	= 10;
	m_RespawnTypes[0].SpawnAmountAtLittle	= 10;
	m_RespawnTypes[0].SpawnAmountInBetween	= 5;
	m_RespawnTypes[0].SpawnAmountAtHigh		= 2;

	// Average -- Average
	m_RespawnTypes[1].RespawnRate			= 180.0f;
	m_RespawnTypes[1].MaximumAmount			= 25;
	m_RespawnTypes[1].CurrentAmountHigh		= 15;
	m_RespawnTypes[1].CurrentAmountLittle	= 10;
	m_RespawnTypes[1].SpawnAmountAtLittle	= 5;
	m_RespawnTypes[1].SpawnAmountInBetween	= 3;
	m_RespawnTypes[1].SpawnAmountAtHigh		= 1;

	// Common -- Little
	m_RespawnTypes[2].RespawnRate			= 90.0f;
	m_RespawnTypes[2].MaximumAmount			= 10;
	m_RespawnTypes[2].CurrentAmountHigh		= 10;
	m_RespawnTypes[2].CurrentAmountLittle	= 5;
	m_RespawnTypes[2].SpawnAmountAtLittle	= 2;
	m_RespawnTypes[2].SpawnAmountInBetween	= 1;
	m_RespawnTypes[2].SpawnAmountAtHigh		= 1;
}

void SSResourceManager::Shutdown( )
{
	m_Resources.clear( );
	m_ResourceModels.clear( );

	m_ResourceSpawn.clear( );
	if ( m_InitialSpawn )
	{
		tDelete( m_InitialSpawn );
		m_InitialSpawn = nullptr;
	}

	m_FirstSpawnDelay = RESOURCE_SPAWN_INITIAL_DELAY;
}

void SSResourceManager::UpdateSimLayer( const float timestep )
{
	fVector<int> resourceCounts( m_Resources.size( ) );
	for ( auto& it : resourceCounts )
		it = 0;

	// Get number of resources on spawn point
	Entity entityID = 0;
	for ( auto& entity : g_EntityManager.GetEntityMasks( ) )
	{
		if ( entity & GetDenseComponentFlag<ResourceComponent>( ) )
		{
			ResourceComponent* resourceComponent = GetDenseComponent<ResourceComponent>( entityID );
			if ( resourceComponent->SpawnedBy != -1 )
				++resourceCounts[resourceComponent->SpawnedBy];
		}
		++entityID;
	}

	// Initiate resource drop
	m_FirstSpawnDelay -= timestep;
	if ( m_FirstSpawnDelay <= 0.0f && m_InitialSpawn == nullptr )
	{
		int spawnID = 0;
		for ( auto& resource : m_Resources )
		{
			float respawnRate = m_RespawnTypes[resource.RespawnType].RespawnRate;

			resource.LastSpawn -= timestep;
			if ( resource.LastSpawn <= 0.0f )
			{
				resource.LastSpawn = respawnRate / 2.0f + respawnRate * (float) g_Randomizer.SimGenerateRandom( );

				float dropOrgX			= 50.0f * (2.0f * (float) g_Randomizer.SimGenerateRandom( ) - 1.0f);
				float dropOrgZ			= 50.0f * (2.0f * (float) g_Randomizer.SimGenerateRandom( ) - 1.0f);
				glm::vec3 spawnOffset	= glm::vec3( dropOrgX, RESOURCE_RESPAWN_DROP_HEIGHT, dropOrgZ );

				int respawnCount		= GetRespawnAmount( resourceCounts[spawnID], resource.RespawnType );

				for ( int i = 0; i < respawnCount; ++i )
				{
					float		spawnAngle			= (float) g_Randomizer.SimGenerateRandom( ) * glm::pi<float>( ) * 2.0f;
					float		spawnDist			= (float) g_Randomizer.SimGenerateRandom( ) * resource.RespawnRadius;

					glm::vec3	spawnPos;
					spawnPos.x						= resource.Position.x + glm::cos( spawnAngle ) * spawnDist;
					spawnPos.z						= resource.Position.z + glm::sin( spawnAngle ) * spawnDist;
					spawnPos.y						= gfx::g_GFXTerrain.GetHeightAtWorldCoord( spawnPos.x, spawnPos.z );

					float		orientationAngle	= (float) g_Randomizer.SimGenerateRandom( ) * glm::pi<float>( ) * 2.0f;
					glm::quat	spawnOrientation	= glm::rotate( resource.Orientation, orientationAngle, glm::vec3( 0.0f, 1.0f, 0.0f ) );

					pString		model				= m_ResourceModels.at( g_Randomizer.SimRand( 0, (unsigned int) m_ResourceModels.size( ) ) );

					DropResource( spawnPos, resource.Scale, spawnOrientation, model, spawnOffset, spawnID );					
				}
			}

			++spawnID;
		}
	}

	// Drop in initial resources
	else if ( m_FirstSpawnDelay <= 0.0f && m_InitialSpawn )
	{
		for ( auto& resource : *m_InitialSpawn )
			DropResource( resource.Position, resource.Scale, resource.Orientation, resource.ModelPath, glm::vec3( 0.0f, RESOURCE_RESPAWN_DROP_HEIGHT, 0.0f ), -1 );
		tDelete( m_InitialSpawn );
		m_InitialSpawn = nullptr;

		m_FirstSpawnDelay = RESOURCE_SPAWN_INITIAL_DELAY;
	}

	// Update resource drops
	for ( auto& drop : m_ResourceSpawn )
	{
		if ( drop.Active )
		{
			drop.DropTime -= timestep;
			if ( drop.DropTime <= 0.0f )
			{
				drop.Active = false;
				if ( drop.Effect )
					drop.Effect->TimeToLive = drop.Effect->ParticlesTimeToLive;

				EntityFactory::CreateResource( drop.Position.x, drop.Position.z, drop.Scale, drop.Orientation, drop.ModelPath.c_str( ), drop.SpawnedBy );
				g_SSMiniMap.PingLocation( drop.Position );
			}
		}
	}
}

int SSResourceManager::GetRespawnAmount( int currentAmount, int respawnType )
{
	// Make sure respawnType is valid
	if ( respawnType < 0 || respawnType >= RESPAWN_TYPE_COUNT )
		return 0;

	// Too many resources; return 0
	if ( currentAmount >= m_RespawnTypes[respawnType].MaximumAmount )
		return 0;

	// More than high; spawn a little
	if ( currentAmount >= m_RespawnTypes[respawnType].CurrentAmountHigh )
		return m_RespawnTypes[respawnType].SpawnAmountAtHigh;

	// Less than little; spawn many
	if ( currentAmount <= m_RespawnTypes[respawnType].CurrentAmountLittle )
		return m_RespawnTypes[respawnType].SpawnAmountAtLittle;

	// In all other cases; spawn some
	return m_RespawnTypes[respawnType].SpawnAmountInBetween;
}

void SSResourceManager::TryPlaySFX(glm::vec3 position)
{
	SFXEvent event;
	event.AtBeat = BeatType::NONE;
	event.Name = RESOURCE_DROP_SFX_PATH;
	event.Info3D.Is3D = true;
	event.Info3D.DistMin = RESOURCE_DROP_SFX_MIN;
	event.Info3D.DistMax = RESOURCE_DROP_SFX_MAX;
	event.Info3D.Position = position;
	g_SSAudio.PostEventSFX(event);
}

void SSResourceManager::AddResource( glm::vec2 position, glm::vec3 scale, glm::quat orientation, const pString& modelPath, bool respawning, float respawnRadius, int respawnType )
{
	float height			= gfx::g_GFXTerrain.GetHeightAtWorldCoord( position.x, position.y );

	ResourceAttributes resource;
	resource.Position		= glm::vec3( position.x, height, position.y );
	resource.Scale			= scale;
	resource.Orientation	= orientation;
	resource.ModelPath		= modelPath;
	resource.Respawning		= respawning;
	resource.RespawnType	= respawnType;
	resource.RespawnRadius	= respawnRadius;
	resource.LastSpawn		= RESOURCE_RESPAWN_FIRST_DELAY * (float) g_Randomizer.SimGenerateRandom( );

	if ( respawning )
		m_Resources.push_back( resource );
	else
	{
		if ( m_InitialSpawn == nullptr )
			m_InitialSpawn = tNew( tVector<ResourceAttributes> );
		m_InitialSpawn->push_back( resource );
	}
}

void SSResourceManager::AddModelPath( const pString& modelPath )
{
	m_ResourceModels.push_back( modelPath );
}

void SSResourceManager::DropResource( glm::vec3 position, glm::vec3 scale, glm::quat orientation, const pString& modelPath, glm::vec3 dropOffset, int spawner )
{
	ResourceSpawn* drop = nullptr;
	for ( auto& res : m_ResourceSpawn )
	{
		if ( res.Active == false )
		{
			drop = &res;
			break;
		}
	}

	if ( drop == nullptr )
	{
		m_ResourceSpawn.push_back( ResourceSpawn( ) );
		drop = &m_ResourceSpawn.back( );
	}

	drop->Position			= position;
	drop->Scale				= scale;
	drop->Orientation		= orientation;
	drop->ModelPath			= modelPath;
	drop->SpawnedBy			= spawner;
	drop->DropTime			= RESOURCE_RESPAWN_DROP_TIME + (float) g_Randomizer.SimGenerateRandom( ) * RESOURCE_RESPAWN_DROP_TIME_DIFF;
	drop->InitialDropTime	= drop->DropTime;
	drop->Active			= true;
	drop->DropOriginOffset	= dropOffset;

	gfx::ParticleSystem effect	= g_SSParticle.GetDefaultParticleSystem( PARTICLE_TYPE_SMOKE_CLOUD );
	effect.EmitterPosition		= position + dropOffset;
	effect.TimeToLive			= 9999.9f;
	drop->Effect				= g_SSParticle.SpawnParticleSystem( PARTICLE_TYPE_SMOKE_CLOUD, effect, false );

	TryPlaySFX(position);
}

void SSResourceManager::Render( rMap<int, InstancedObject>& renderBucket )
{
	// Render drop area
	for ( auto& resource : m_Resources )
	{
		gfx::Decal d;
		d.World		= glm::translate( resource.Position )
					* glm::rotate( glm::half_pi<float>( ), glm::vec3( 1.0f, 0.0f, 0.0f ) )
					* glm::scale( glm::vec3( resource.RespawnRadius ) );

		d.Texture	= g_DecalManager.GetTextureAtlas( )->GetHandle( "resource_zone.png" );
		d.Tint		= this->GetResourceSpawnTypeColour( resource.RespawnType );

		g_DecalManager.AddToQueue( d );
	}

	// Render drops
	for ( auto& drop : m_ResourceSpawn )
	{
		if ( drop.Active )
		{
			glm::vec3 position				= drop.Position + drop.DropOriginOffset * drop.DropTime / drop.InitialDropTime;

			if ( drop.Effect )
				drop.Effect->EmitterPosition = position;

			glm::mat4 world; glm::quat orientation = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );
			int bucket = gfx::g_ModelBank.LoadModel( "pod1Shape.robo" );
			g_SSRender.CalculateWorldMatrix( position, glm::vec3( 1.0f, 1.0f, 1.0f ), orientation, world );
			renderBucket[bucket].worlds.push_back( world );
			renderBucket[bucket].colors.push_back( glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
		}
	}
}

const pVector<ResourceAttributes>& SSResourceManager::GetResources() const
{
	return m_Resources;
}

glm::vec4 SSResourceManager::GetResourceSpawnTypeColour( int spawnType ) const
{
	// Let this be staticly defined like this, lobby will use it :D
	if ( spawnType == 0 )
		return glm::vec4( 1.0f, 0.0f, 1.0f, 1.0f );
	else if ( spawnType == 1 )
		return glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f );
	else if ( spawnType == 2 )
		return glm::vec4( 0.0f, 1.0f, 1.0f, 1.0f );

	return glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
}
