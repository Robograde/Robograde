/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include <memory/Alloc.h>
#include <glm/vec3.hpp>
#include "entity/ICollisionEntity.h"

#define g_CollisionDetection	ICollisionDetection::GetInstance()

typedef std::function<float( const float x, const float z )> GetTerrainHeightFunction;

// Interface to Collision Detection Engine
class COLLISION_DETECTION_API ICollisionDetection
{
public:
	static ICollisionDetection&	GetInstance					();

	virtual void				Shutdown					() = 0;

	virtual void				Step						() = 0;

	virtual void				PickingWithRay				( glm::vec3& rayPosition, glm::vec3& rayDirection, const rVector<int>& pickAgainstGroupsIDs, rVector<unsigned int>& outEntities, glm::vec3* outPosition ) = 0;
	virtual void				PickingWithFrustum			( glm::vec3 (&planePositions)[6], glm::vec3 (&planeNormals)[6], const rVector<int>& pickAgainstGroupsIDs, rVector<unsigned int>& outEntities ) = 0;
	virtual void				SetGroupsAffectedByFog		( const rVector<int>& groupsAffectedByFog ) = 0;
	virtual void				SetFogOfWarInfo				( const rVector<rVector<bool>>& texture, const glm::ivec2& textureSize, const glm::vec2& tileSize ) = 0;

	virtual ICollisionEntity*	CreateEntity				() = 0;
	virtual void				DestroyEntity				( ICollisionEntity* entity ) = 0;

	virtual void				AddCollisionVolumeRay		( ICollisionEntity* entity, const glm::vec3& position, const glm::vec3& direction		) = 0;
	virtual void				AddCollisionVolumeOBB		( ICollisionEntity* entity, const glm::vec3& position, const glm::vec3 (&directions)[3]	) = 0;
	virtual void				AddCollisionVolumePlane		( ICollisionEntity* entity, const glm::vec3& position, const glm::vec3& normal			) = 0;
	virtual void				AddCollisionVolumeSphere	( ICollisionEntity* entity, const glm::vec3& position, const float radius				) = 0;
	virtual void				AddCollisionVolumeHeightMap	( ICollisionEntity* entity, GetTerrainHeightFunction getTerrainHeightFunction			) = 0;
};