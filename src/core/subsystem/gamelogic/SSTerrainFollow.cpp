/**************************************************
Copyright 2015 Ola Enberg
***************************************************/

#include "SSTerrainFollow.h"

#include <gfx/GraphicsEngine.h>
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/PlacementComponent.h"
#include "../../component/TerrainFollowComponent.h"
#include "../../datadriven/EntityManager.h"

SSTerrainFollow& SSTerrainFollow::GetInstance()
{
	static SSTerrainFollow instance;
	return instance;
}

void SSTerrainFollow::Startup()
{
	Subsystem::Startup();
}

void SSTerrainFollow::UpdateUserLayer( const float deltaTime )
{
	EntityMask placementFlag = DenseComponentCollection<PlacementComponent>::GetInstance().GetComponentTypeFlag();
	EntityMask terrainFollowFlag = DenseComponentCollection<TerrainFollowComponent>::GetInstance().GetComponentTypeFlag();
	EntityMask combinedFlag = placementFlag | terrainFollowFlag;
	int entityID = 0;
	for ( auto& entityMask : EntityManager::GetInstance().GetEntityMasks() )
	{
		// Check if entity has a placement component
		if ( ( entityMask & combinedFlag ) == combinedFlag )
		{
			PlacementComponent* placementComponent = GetDenseComponent<PlacementComponent>( entityID );
			TerrainFollowComponent* terrainFollowComponent = GetDenseComponent<TerrainFollowComponent>( entityID );

			float addedHeight = terrainFollowComponent->Offset * placementComponent->Scale.y;
			placementComponent->Position.y = gfx::g_GFXTerrain.GetHeightAtWorldCoord(placementComponent->Position.x, placementComponent->Position.z) + addedHeight + 0.2f;
		}
		entityID++;
	}
}

void SSTerrainFollow::Shutdown()
{
	Subsystem::Shutdown();
}