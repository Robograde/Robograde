/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSHashGenerator.h"

#include <network/NetworkInfo.h>
#include <utility/Randomizer.h>
#include "../replay/SSReplayPlayer.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/PlacementComponent.h"
#include "../../component/AgentComponent.h"
#include "../../datadriven/EntityManager.h"

SSHashGenerator& SSHashGenerator::GetInstance()
{
	static SSHashGenerator instance;
	return instance;
}

void SSHashGenerator::UpdateSimLayer( const float timeStep )
{
	m_PositionHash = CalculatePositionHash();
}

void SSHashGenerator::Shutdown()
{
	m_PositionHash = 0;
}

unsigned int SSHashGenerator::CalculatePositionHash() const
{
	unsigned int hash = 0;
	EntityMask combinedFlags = DenseComponentCollection<PlacementComponent>::GetInstance().GetComponentTypeFlag() | DenseComponentCollection<AgentComponent>::GetInstance().GetComponentTypeFlag();
	int entityID = 0;
	for ( auto& entityMask : EntityManager::GetInstance().GetEntityMasks() )
	{
		// Check if entity has a placement component
		if ( ( entityMask & combinedFlags ) )
		{
			PlacementComponent* placement = GetDenseComponent<PlacementComponent>( entityID );

			hash += reinterpret_cast<unsigned int&>( placement->Position.x );
			hash += reinterpret_cast<unsigned int&>( placement->Position.z );
		}
		entityID++;
	}

	return hash;
}

unsigned int SSHashGenerator::GetHash() const
{
	return m_PositionHash;
}