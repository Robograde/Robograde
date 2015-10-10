/**************************************************
2015 Johan Melin och Viktor Kelkannen
***************************************************/

#include "SSKill.h"

#include <utility/Logger.h>
#include <gfx/GraphicsEngine.h>
#include "SSAI.h"
#include "SSCollision.h"
#include "../../datadriven/EntityManager.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/HealthComponent.h"
#include "../../component/CollisionComponent.h"
#include "../../component/ParentComponent.h"
#include "../../ai/Terrain.h"
#include "../../CallbackMessage.h"
#include "../../EntityFactory.h"
#include "../gfx/SSDecaling.h"

SSKill& SSKill::GetInstance( )
{
	static SSKill instance;
	return instance;
}

void SSKill::Startup()
{
	g_CallbackPump.AddCallbackMessageToSystem( CALLBACK_MESSAGE_ENTITY_KILLED );
}

void SSKill::UpdateSimLayer( const float timeStep )
{
	EntityMask healthFlag = GetDenseComponentFlag<HealthComponent>();
	auto components = GetDenseComponentArray<HealthComponent>();
	int entityID = 0;
	for ( auto& entityMask : EntityManager::GetInstance().GetEntityMasks() )
	{
		if ( entityMask & healthFlag )
		{
			HealthComponent* health = &components->at( entityID );

			if ( health->IsDead )
			{
				KillEntity( entityID );
			}
		}
		entityID++;
	}
}

void SSKill::KillEntity( Entity entityID )
{
	if ( HasComponent<ResourceComponent>( entityID ) )
	{
		ResourceComponent* resource = GetDenseComponent<ResourceComponent>(entityID );
		Terrain::GetInstance()->UpdateSensor( entityID, resource->TilePointer, resource->TilePointer, 1, true );
	}
	if ( HasComponent<AgentComponent>( entityID ) )
	{
		Agent* agent = GetDenseComponent<AgentComponent>( entityID )->Agent;
		Terrain::GetInstance()->UpdateSensor( entityID, agent->GetTile(), agent->GetTile(), agent->GetSize(), true );

		if ( HasComponent<PlacementComponent>( entityID ) )
		{
			g_SSDecaling.AddTimedDecal( GetDenseComponent<PlacementComponent>( entityID )->Position, "ashes.png", 7.0f, glm::vec4( 1.0f ), 1.0f, 1.0f );
		}

		g_SSAI.KillAgent( agent );
	}
	if ( HasComponent<CollisionComponent>( entityID ) )
	{
		g_SSCollision.RemoveCollisionEntity( entityID );
	}
	if ( HasComponent<ParentComponent>( entityID ) )
	{
		ParentComponent* parent = GetDenseComponent<ParentComponent>( entityID );
		auto removeChild = [parent] (int i)
		{
			if ( parent->Children[i] != ENTITY_INVALID )
			{
				g_EntityManager.RemoveEntity( parent->Children[i] );
			}
		};
		removeChild( 0 );
		removeChild( 1 );
		removeChild( 2 );
		removeChild( 3 );
	}
	//Logger::Log( "Killing entity with ID: " + rToString( entityID ), "SSKill", LogSeverity::DEBUG_MSG );
	g_CallbackPump.SendCallbackMessagePlease( tNew( DataCallbackMessage<Entity>, CALLBACK_MESSAGE_ENTITY_KILLED, entityID ) );
	g_CallbackPump.Update();
	g_EntityManager.RemoveEntity( entityID ); // WARNING: Don't use health component after this is run, it will be destructed!
}
