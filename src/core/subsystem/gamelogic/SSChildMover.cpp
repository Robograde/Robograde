/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#include "SSChildMover.h"

#include "../../datadriven/DenseComponentCollection.h"
#include "../../datadriven/EntityManager.h"
#include "../../datadriven/ComponentHelper.h"
#include "../../component/ChildComponent.h"
#include "../../component/PlacementComponent.h"
#include "../../component/ParentComponent.h"
#include "../../component/AgentComponent.h"
#include "../../component/WeaponComponent.h"

SSChildMover& SSChildMover::GetInstance( )
{
	static SSChildMover instance;
	return instance;
}

void SSChildMover::UpdateSimLayer( const float deltaTime )
{
	EntityMask weaponFlag = GetDenseComponentFlag<WeaponComponent>( );
	auto components = GetDenseComponentArray<ChildComponent>();
	for ( unsigned int entityID = 0; entityID < g_EntityManager.GetEntityMasks().size(); ++entityID )
	{
		if ( HasComponent<ChildComponent>( entityID ) )
		{
			ChildComponent* child = &components->at( entityID );
			Entity parent = child->Parent;

			if ( parent != ENTITY_INVALID )
			{
				// Make sure both child and parent has a placement component
				if ( HasComponent<PlacementComponent>( parent ) && HasComponent<PlacementComponent>( entityID ) )
				{
					PlacementComponent* parentPlacement = GetDenseComponent<PlacementComponent>( parent );
					PlacementComponent* childPlacement = GetDenseComponent<PlacementComponent>( entityID );

					childPlacement->Position = (parentPlacement->Position) + parentPlacement->Orientation*child->Offset;
					//child->Orientation = glm::normalize(glm::quat(1.0f, 0.05f, 0.02f, 0.0f)) * child->Orientation;

					if ( g_EntityManager.GetEntityMask( entityID ) & weaponFlag )
					{
						WeaponComponent* weaponComponent = GetDenseComponent<WeaponComponent>( entityID );

						if ( weaponComponent->IsTurret )
							continue;
					}

					childPlacement->Orientation = parentPlacement->Orientation * child->Orientation;
				}
			}
		}
	}
}
