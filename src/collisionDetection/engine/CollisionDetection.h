/**************************************************
Zlib Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../ICollisionDetection.h"

#include <memory/Alloc.h>
#include "../entity/Entity.h"
#include "../detection/IntersectionTestLookupTable.h"

class CollisionDetection : public ICollisionDetection
{
public:
								~CollisionDetection			();
	void						Shutdown					() override;

	void						Step						() override;

	void						PickingWithRay				( glm::vec3& rayPosition, glm::vec3& rayDirection, const rVector<int>& pickAgainstGroupsIDs, rVector<unsigned int>& outEntities, glm::vec3* outPosition ) override;
	void						PickingWithFrustum			( glm::vec3 (&planePositions)[6], glm::vec3 (&planeNormals)[6], const rVector<int>& pickAgainstGroupsIDs, rVector<unsigned int>& outIntersections ) override;
	void						SetGroupsAffectedByFog		( const rVector<int>& groupsAffectedByFog ) override;
	void						SetFogOfWarInfo				( const rVector<rVector<bool>>& texture, const glm::ivec2& textureSize, const glm::vec2& tileSize ) override;

	ICollisionEntity*			CreateEntity				() override;
	void						DestroyEntity				( ICollisionEntity* entity ) override;

	void						AddCollisionVolumeRay		( ICollisionEntity* entity, const glm::vec3& position, const glm::vec3& direction		) override;
	void						AddCollisionVolumeOBB		( ICollisionEntity* entity, const glm::vec3& position, const glm::vec3 (&directions)[3]	) override;
	void						AddCollisionVolumePlane		( ICollisionEntity* entity, const glm::vec3& position, const glm::vec3& normal			) override;
	void						AddCollisionVolumeSphere	( ICollisionEntity* entity, const glm::vec3& position, const float radius				) override;
	void						AddCollisionVolumeHeightMap	( ICollisionEntity* entity, GetTerrainHeightFunction getTerrainHeightFunction			) override;

private:
	void						PickingWithVolume			( Volume* pickingVolume, const rVector<int>& pickAgainstGroupsIDs, rVector<unsigned int>& outEntities, glm::vec3* outPosition, bool pickClosest );
	bool						CalcVisibilityForPosition	( const glm::vec3& position ) const;

	rVector<Entity*>			m_Entities;
	IntersectionTestLookupTable m_TestLookup;
	rVector<int>				m_GroupsAffectedByFog;
	rVector<rVector<bool>>		m_FogOfWarTexture;
	glm::ivec2					m_FogOfWarTextureSize;
	glm::vec2					m_FogOfWarTileSize;
};