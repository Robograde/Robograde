/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "CollisionDetection.h"

CollisionDetection::~CollisionDetection()
{
	this->Shutdown();
}

void CollisionDetection::Shutdown()
{
	for ( auto& entity : m_Entities )
	{
		pDelete( entity );
	}
	m_Entities.clear();
}

void CollisionDetection::Step()
{
	for ( auto& entity : m_Entities )
	{
		entity->CalculateWorldVolumes();
	}
}

void CollisionDetection::SetGroupsAffectedByFog( const rVector<int>& groupsAffectedByFog )
{
	m_GroupsAffectedByFog = groupsAffectedByFog;
}

void CollisionDetection::SetFogOfWarInfo( const rVector<rVector<bool>>& texture, const glm::ivec2& textureSize, const glm::vec2& tileSize )
{
	m_FogOfWarTexture		= texture;
	m_FogOfWarTextureSize	= textureSize;
	m_FogOfWarTileSize		= tileSize;
}

ICollisionEntity* CollisionDetection::CreateEntity()
{
	Entity* entity = pNew( Entity );
	m_Entities.push_back( entity );
	return entity;
}

void CollisionDetection::DestroyEntity( ICollisionEntity* entity )
{
	auto entity_it = std::find( m_Entities.begin(), m_Entities.end(), entity );

	assert ( entity_it != m_Entities.end() );	// Makes sure that the entity was created by this instance.

	pDelete( *entity_it );
	m_Entities.erase( entity_it );
}

bool CollisionDetection::CalcVisibilityForPosition( const glm::vec3& position ) const
{
	const int x = static_cast<int>( position.x / m_FogOfWarTileSize.x);
	const int z = static_cast<int>( position.z / m_FogOfWarTileSize.y);

	if ( x < 0 || z < 0 || x >= m_FogOfWarTextureSize.x || z >= m_FogOfWarTextureSize.y )
	{
		return false;
	}

	return m_FogOfWarTexture[z][x];
}