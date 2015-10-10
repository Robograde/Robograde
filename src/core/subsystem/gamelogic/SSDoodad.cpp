/**************************************************
Copyright 2015 Viktor Kelkannen
***************************************************/

#include "SSDoodad.h"

#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/PlacementComponent.h"
#include "../../datadriven/EntityManager.h"
#include "../../component/DoodadComponent.h"
#include "../../datadriven/ComponentHelper.h"

SSDoodad& SSDoodad::GetInstance()
{
	static SSDoodad instance;
	return instance;
}

void SSDoodad::Startup()
{
	m_Wind = 0.0f;
	Subsystem::Startup();
}

void SSDoodad::UpdateUserLayer(const float deltaTime)
{
	EntityMask doodadFlag = GetDenseComponentFlag<DoodadComponent>( );
	auto components = GetDenseComponentArray<DoodadComponent>();
	for (unsigned int entityID = 0; entityID < g_EntityManager.GetEntityMasks().size(); ++entityID)
	{
		if ( g_EntityManager.GetEntityMask( entityID ) & doodadFlag )
		{
			DoodadComponent* dc = GetDenseComponent<DoodadComponent>( entityID );

			if ( !dc->IsTree )
			continue;

			PlacementComponent* pc = GetDenseComponent<PlacementComponent>( entityID );
			 float rotationAmount = 30 * deltaTime * sin(m_Wind) / dc->SwaySpeedDivisor;
			glm::quat rotation( glm::cos( rotationAmount ), dc->AngleVector * glm::sin( rotationAmount ) );
			pc->Orientation = glm::normalize( rotation * pc->Orientation );
		}
	}

	m_Wind += deltaTime;
}

void SSDoodad::Shutdown()
{
	Subsystem::Shutdown();
}