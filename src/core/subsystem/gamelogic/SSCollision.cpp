/**************************************************
2015 Ola Enberg
***************************************************/

#include "SSCollision.h"
#include <collisionDetection/ICollisionDetection.h>
#include "../../datadriven/DenseComponentCollection.h"
#include "../../datadriven/EntityManager.h"
#include "../../component/CollisionComponent.h"
#include "../../component/PlacementComponent.h"

SSCollision& SSCollision::GetInstance( )
{
	static SSCollision instance;
	return instance;
}

void SSCollision::Startup( )
{
	Subsystem::Startup();
}

void SSCollision::UpdateUserLayer( const float deltaTime )
{
	EntityMask collisionFlag = DenseComponentCollection<CollisionComponent>::GetInstance().GetComponentTypeFlag();
	EntityMask placementFlag = DenseComponentCollection<PlacementComponent>::GetInstance().GetComponentTypeFlag();
	int entityID = 0;

	for ( auto& entityMask : g_EntityManager.GetEntityMasks() )
	{
		if ( ( entityMask & collisionFlag ) && ( entityMask & placementFlag ) )
		{
			CollisionComponent* collisionComp = GetDenseComponent<CollisionComponent>( entityID );
			PlacementComponent* placementComp = GetDenseComponent<PlacementComponent>( entityID );

			collisionComp->CollisionEntity->SetPosition( placementComp->Position );
			collisionComp->CollisionEntity->SetScale( placementComp->Scale.x );
			collisionComp->CollisionEntity->SetOrientation( placementComp->Orientation );
		}
		entityID++;
	}

	g_CollisionDetection.Step();
}

void SSCollision::Shutdown( )
{
	g_CollisionDetection.Shutdown();
	Subsystem::Shutdown();
}

void SSCollision::RemoveCollisionEntity( Entity entity )
{
	g_CollisionDetection.DestroyEntity( GetDenseComponent<CollisionComponent>( entity )->CollisionEntity );
}

void SSCollision::SetGroupsAffectedByFog( const rVector<int>& groupsAffectedByFog )
{
	g_CollisionDetection.SetGroupsAffectedByFog( groupsAffectedByFog );
}

void SSCollision::SetFogOfWarInfo( const rVector<rVector<bool>>& texture, const glm::ivec2& textureSize, const glm::vec2& tileSize )
{
	g_CollisionDetection.SetFogOfWarInfo( texture, textureSize, tileSize );
}