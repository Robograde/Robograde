/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#include "CollisionDetection.h"

#include "../volume/Ray.h"
#include "../volume/OBB.h"
#include "../volume/Plane.h"
#include "../volume/Sphere.h"
#include "../volume/HeightMap.h"

void CollisionDetection::AddCollisionVolumeRay( ICollisionEntity* entity, const glm::vec3& position, const glm::vec3& direction )
{
	Ray* volume				= pNew( Ray );
	volume->Position		= position;
	volume->Direction		= direction;
	static_cast<Entity*>( entity )->AddStorageVolume( volume );
}

void CollisionDetection::AddCollisionVolumeOBB( ICollisionEntity* entity, const glm::vec3& position, const glm::vec3 (&directions)[OBB_DIRECTIONS] )
{
	OBB* volume				= pNew( OBB );
	volume->Position		= position;
	for ( int i = 0; i < OBB_DIRECTIONS; ++i )
	{
		volume->HalfSizes[i]	= glm::length( directions[i] );
		volume->Directions[i]	= directions[i] / volume->HalfSizes[i];
	}
	static_cast<Entity*>( entity )->AddStorageVolume( volume );
}

void CollisionDetection::AddCollisionVolumePlane( ICollisionEntity* entity, const glm::vec3& position, const glm::vec3& normal )
{
	Plane* volume			= pNew( Plane );
	volume->Position		= position;
	volume->Normal			= normal;
	static_cast<Entity*>( entity )->AddStorageVolume( volume );
}

void CollisionDetection::AddCollisionVolumeSphere( ICollisionEntity* entity, const glm::vec3& position, const float radius )
{
	Sphere* volume			= pNew( Sphere );
	volume->Position		= position;
	volume->Radius			= radius;
	static_cast<Entity*>( entity )->AddStorageVolume( volume );
}

void CollisionDetection::AddCollisionVolumeHeightMap( ICollisionEntity* entity, GetTerrainHeightFunction getTerrainHeightFunction )
{
	HeightMap* volume		= pNew( HeightMap, getTerrainHeightFunction );
	static_cast<Entity*>( entity )->AddStorageVolume( volume );
}
