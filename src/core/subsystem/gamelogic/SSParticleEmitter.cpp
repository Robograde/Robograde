/**************************************************
2015 Jens Stjernkvist
***************************************************/

#include "SSParticleEmitter.h"

#include "../../datadriven/DenseComponentCollection.h"
#include "../../datadriven/EntityManager.h"
#include "../../component/PlacementComponent.h"

//+----------------------------------------------------------------------------+
//|SSParticleEmitter& GetInstance(void)
//\----------------------------------------------------------------------------+
SSParticleEmitter& SSParticleEmitter::GetInstance( void )
{
	static SSParticleEmitter instance;
	return instance;
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
SSParticleEmitter::~SSParticleEmitter( )
{
}
//+----------------------------------------------------------------------------+
//|void Startup(void)
//\----------------------------------------------------------------------------+
void SSParticleEmitter::Startup( void )
{
	Subsystem::Startup();
}
//+----------------------------------------------------------------------------+
//|void Shutdown(void)
//\----------------------------------------------------------------------------+
void SSParticleEmitter::Shutdown( void )
{

	Subsystem::Shutdown();
}
//+----------------------------------------------------------------------------+
//|void UpdateUserLayer(const float deltaTime)
//\----------------------------------------------------------------------------+
void SSParticleEmitter::UpdateUserLayer( const float deltaTime )
{
	Subsystem::UpdateUserLayer( deltaTime );
	EntityMask emitterFlag = DenseComponentCollection<ParticleEmitterComponent>::GetInstance().GetComponentTypeFlag();
	int entityID = 0;
	for(auto& entityMask : EntityManager::GetInstance().GetEntityMasks())
	{
		if((entityMask & emitterFlag))
		{
			ParticleEmitterComponent* emitter = GetDenseComponent<ParticleEmitterComponent>(entityID);
			PlacementComponent* placementComponent = GetDenseComponent<PlacementComponent>(entityID);

			///Update position of sfxemitter
			if(placementComponent)
			{
				emitter->Pos = placementComponent->Position;
			}
		}
		entityID++;
	}
}
//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
SSParticleEmitter::SSParticleEmitter( )
	:	Subsystem("SSParticleEmitter")
{
}
//+----------------------------------------------------------------------------+
//|void HandleEvents(const int entityID, SFXEmitterComponent* &sfxEmitter)
//|Check if any events needs to be handled on the emitter
//\----------------------------------------------------------------------------+
void SSParticleEmitter::HandleEvents(const int entityID, ParticleEmitterComponent* &sfxEmitter)
{

}