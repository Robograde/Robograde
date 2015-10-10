/**************************************************
Copyright 2015 David Pejtersen
***************************************************/

#pragma once

#include "../Subsystem.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../gfx/SSParticle.h"
#include "../gfx/SSRender.h"

#define g_SSResourceManager SSResourceManager::GetInstance( )

#define RESOURCE_SPAWN_INITIAL_DELAY	10.0f

#define RESOURCE_RESPAWN_FIRST_DELAY	180.0f
#define RESOURCE_RESPAWN_DROP_HEIGHT	300.0f
#define RESOURCE_RESPAWN_DROP_TIME		2.071f ///Time until boom SFX is heard
#define RESOURCE_RESPAWN_DROP_TIME_DIFF	2.0f

#define RESPAWN_TYPE_COUNT				3

struct ResourceAttributes
{
	glm::vec3	Position;
	glm::vec3	Scale;
	glm::quat	Orientation;
	pString		ModelPath;

	bool		Respawning;
	int			RespawnType;
	float		RespawnRadius;
	float		LastSpawn;
};

class SSResourceManager : public Subsystem
{
public:
	static SSResourceManager& GetInstance( );

	void Startup( ) override;
	void Shutdown( ) override;

	void UpdateSimLayer( const float timestep ) override;

	void AddResource( glm::vec2 position, glm::vec3 scale, glm::quat orientation, const pString& modelPath, bool respawning = false, float respawnRadius = 0.0f, int respawnType = -1 );
	void AddModelPath( const pString& modelPath );

	void DropResource( glm::vec3 position, glm::vec3 scale, glm::quat orientation, const pString& modelPath, glm::vec3 dropOffset, int spawner = -1 );

	void Render( rMap<int, InstancedObject>& renderBucket );

	const pVector<ResourceAttributes>&	GetResources() const;
	glm::vec4							GetResourceSpawnTypeColour( int spawnType ) const;

private:
	/* no type */		SSResourceManager( ) : Subsystem( "ResourceManager" ) { }
	/* no type */		SSResourceManager( const SSResourceManager& );
	/* no type */		~SSResourceManager( ) { }
	SSResourceManager&	operator=( const SSResourceManager& );

	int	 GetRespawnAmount( int currentAmount, int respawnType );
	void TryPlaySFX(glm::vec3 position);

	struct ResourceSpawn
	{
		glm::vec3	Position;
		glm::vec3	Scale;
		glm::quat	Orientation;
		pString		ModelPath;

		int			SpawnedBy;
		bool		Active;
		float		DropTime;
		float		InitialDropTime;
		glm::vec3	DropOriginOffset;

		gfx::ParticleSystem* Effect;
	};

	struct RespawnTypes
	{
		float RespawnRate;

		int   MaximumAmount;

		int	  CurrentAmountHigh;
		int	  CurrentAmountLittle;

		int   SpawnAmountAtHigh;
		int   SpawnAmountInBetween;
		int   SpawnAmountAtLittle;
	};

	RespawnTypes					m_RespawnTypes[RESPAWN_TYPE_COUNT];
	pVector<ResourceAttributes>		m_Resources;
	pVector<ResourceSpawn>			m_ResourceSpawn;
	pVector<pString>				m_ResourceModels;
	float							m_FirstSpawnDelay = RESOURCE_SPAWN_INITIAL_DELAY;

	tVector<ResourceAttributes>*	m_InitialSpawn = nullptr;
};