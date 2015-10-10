/**************************************************
Copyright 2015 Jens Stjernkvist
***************************************************/

#include "SSSFXEmitter.h"

#include "../../datadriven/DenseComponentCollection.h"
#include "../../datadriven/EntityManager.h"
#include "../../component/PlacementComponent.h"
#include "../gamelogic/SSCamera.h"
#include "SSAudio.h"

//+----------------------------------------------------------------------------+
//|AudioSubsystem& GetInstance(void)
//\----------------------------------------------------------------------------+
SSSFXEmitter& SSSFXEmitter::GetInstance( void )
{
	static SSSFXEmitter instance;
	return instance;
}
//+----------------------------------------------------------------------------+
//|Destructor
//\----------------------------------------------------------------------------+
SSSFXEmitter::~SSSFXEmitter( )
{
}
//+----------------------------------------------------------------------------+
//|void Startup(void)
//\----------------------------------------------------------------------------+
void SSSFXEmitter::Startup( void )
{
	Subsystem::Startup();
}
//+----------------------------------------------------------------------------+
//|void Shutdown(void)
//\----------------------------------------------------------------------------+
void SSSFXEmitter::Shutdown( void )
{
	EntityMask sfxEmitterFlag = DenseComponentCollection<SFXEmitterComponent>::GetInstance().GetComponentTypeFlag();

	int entityID = 0;
	for(auto& entityMask : EntityManager::GetInstance().GetEntityMasks())
	{
		if((entityMask & sfxEmitterFlag))
		{
			SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(entityID);

			///Stop the SFXs of the activated triggers if they're still active
			for(std::vector<SFXTrigger>::iterator it = sfxEmitter->SFXTriggers.begin(); it != sfxEmitter->SFXTriggers.end(); it++)
			{
				SFXTrigger* trigger = &(*it);
				g_SSAudio.SetSFXStop(trigger->SFXHandle);
			}

			break;
		}
		entityID++;
	}

	Subsystem::Shutdown();
}
//+----------------------------------------------------------------------------+
//|void UpdateUserLayer(const float deltaTime)
//\----------------------------------------------------------------------------+
void SSSFXEmitter::UpdateUserLayer( const float deltaTime )
{
	Subsystem::UpdateUserLayer( deltaTime );
	EntityMask sfxEmitterFlag = DenseComponentCollection<SFXEmitterComponent>::GetInstance().GetComponentTypeFlag();
	int entityID = 0;
	for(auto& entityMask : EntityManager::GetInstance().GetEntityMasks())
	{
		if((entityMask & sfxEmitterFlag))
		{
			SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(entityID);
			PlacementComponent* placementComponent = GetDenseComponent<PlacementComponent>(entityID);

			///Handle all incoming events
			HandleEvents(entityID, sfxEmitter);

			///Update the activated triggers
			for(std::vector<SFXTrigger>::iterator it = sfxEmitter->SFXTriggers.begin(); it != sfxEmitter->SFXTriggers.end(); it++)
			{
				SFXTrigger* trigger = &(*it);
	
				UpdateTriggeredSFX(deltaTime, entityID, sfxEmitter, trigger);
			}

			///Update position of sfxemitter
			if(placementComponent)
			{
				sfxEmitter->Position = placementComponent->Position;
			}
		}
		entityID++;
	}
}
//+----------------------------------------------------------------------------+
//|Constructor Default
//\----------------------------------------------------------------------------+
SSSFXEmitter::SSSFXEmitter( )
	:	Subsystem("SFXEmitter")
{
}
//+----------------------------------------------------------------------------+
//|void HandleEvents(const int entityID, SFXEmitterComponent* &sfxEmitter)
//|Check if any events needs to be handled on the emitter
//\----------------------------------------------------------------------------+
void SSSFXEmitter::HandleEvents(const int entityID, SFXEmitterComponent* &sfxEmitter)
{
	for(std::vector<SFXTriggerType>::iterator it = sfxEmitter->m_Events.begin(); it != sfxEmitter->m_Events.end(); it++)
	{
		for(std::vector<SFXTrigger>::iterator jt = sfxEmitter->SFXTriggers.begin(); jt != sfxEmitter->SFXTriggers.end(); jt++)
		{
			if((*it) == (*jt).TriggerType)
			{
				SFXTrigger* trigger = &(*jt);
				SFXTriggerActivation(entityID, sfxEmitter, trigger);
			}
		}
	}
	sfxEmitter->m_Events.clear();
}
//+----------------------------------------------------------------------------+
//|void AddSFXTrigger(SFXEmitterComponent* &sfxEmitter, const rString &name, SFXTriggerType triggerType, const unsigned int Loop, const float timeInterval, const float distanceMin, const float distanceMax)
//\----------------------------------------------------------------------------+
void SSSFXEmitter::AddSFXTrigger(SFXEmitterComponent* &sfxEmitter, const rString &name, SFXTriggerType triggerType, const unsigned int Loop,
	const float timeInterval, const float distanceMin, const float distanceMax)
{
	SFXTrigger trigger;
	trigger.DistanceMin = distanceMin;
	trigger.DistanceMax = distanceMax;
	trigger.TimeInterval = timeInterval;
	trigger.Loops = Loop;
	trigger.TriggerType = triggerType;
	trigger.Name = name;
	sfxEmitter->SFXTriggers.push_back(trigger);
}
//+----------------------------------------------------------------------------+
//|bool ModifySFXTriggerName(SFXEmitterComponent* &sfxEmitter, const rString &toModify, const rString &newName)
//\----------------------------------------------------------------------------+
bool SSSFXEmitter::ModifySFXTriggerName(SFXEmitterComponent* &sfxEmitter, const rString &toModify, const rString &newName)
{
	for(std::vector<SFXTrigger>::iterator it = sfxEmitter->SFXTriggers.begin(); it != sfxEmitter->SFXTriggers.end(); it++)
	{
		if(toModify == (*it).Name)
		{
			SFXTrigger* trigger = &(*it);
			trigger->Name = newName;
			return true;
		}
	}

	return false;
}
//+----------------------------------------------------------------------------+
//|void RemoveSFXTrigger(SFXEmitterComponent* &sfxEmitter, const rString &name)
//\----------------------------------------------------------------------------+
bool SSSFXEmitter::RemoveSFXTrigger(SFXEmitterComponent* &sfxEmitter, const rString &name)
{
	///Locate the trigger and remove it
	for(std::vector<SFXTrigger>::iterator it = sfxEmitter->SFXTriggers.begin(); it != sfxEmitter->SFXTriggers.end(); it++)
	{
		if(name == (*it).Name)
		{
			if((*it).SFXHandle > 0)
				g_SSAudio.SetSFXStop((*it).SFXHandle);

			sfxEmitter->SFXTriggers.erase(it);
			return true;
		}
	}
	return false;
}
//+----------------------------------------------------------------------------+
//|void AddEvent(SFXEmitterComponent* &sfxEmitter, const SFXTriggerType &type)
//\----------------------------------------------------------------------------+
void SSSFXEmitter::AddEvent(SFXEmitterComponent* &sfxEmitter, const SFXTriggerType &type)
{
	sfxEmitter->m_Events.push_back(type);
}
//+----------------------------------------------------------------------------+
//|void SFXTriggerActivation(const int entityID, SFXEmitterComponent* &sfxEmitter, SFXTrigger* &trigger)
//|Check if the assigned SFX on the trigger should be activated
//\----------------------------------------------------------------------------+
void SSSFXEmitter::SFXTriggerActivation(const int entityID, SFXEmitterComponent* &sfxEmitter, SFXTrigger* &trigger )
{
	switch(trigger->TriggerType)
	{
		case SFXTriggerType::AT_CREATION:
		{
			//if(trigger->Loops == 0)
			//{
				trigger->Triggered = true;
			//}
			//else if(trigger->Triggered == false && trigger->LoopCount < trigger->Loops)
			//{
			//	trigger->Triggered = true;
			//}
			
			break;
		}

		case SFXTriggerType::AT_DEATH:
		{
			trigger->Triggered = true;
			break;
		}

		case SFXTriggerType::AT_MOVE_EAT:
		{
			trigger->Triggered = true;
			break;
		}

		case SFXTriggerType::AT_MOVE_ATTACK:
		{
			break;
		}

		case SFXTriggerType::AT_TAKING_DAMAGE:
		{

			break;
		}

		case SFXTriggerType::WHILE_EATING:
		{
			trigger->Triggered = true;
			break;
		}

		case SFXTriggerType::WHILE_MOVING:
		{
			trigger->Triggered = true;
			break;
		}

		case SFXTriggerType::WHILE_ATTACKING:
		{
			trigger->Triggered = true;
			break;
		}

		default:
		break;
	}
}
//+----------------------------------------------------------------------------+
//|void UpdateTriggeredSFX(const float deltaTime, const int entityID, SFXEmitterComponent* &sfxEmitter, SFXTrigger* &trigger)
//\----------------------------------------------------------------------------+
void SSSFXEmitter::UpdateTriggeredSFX( const float deltaTime, int entityID, SFXEmitterComponent* &sfxEmitter, SFXTrigger* &trigger )
{
	if(trigger->Triggered)
	{	
		if(trigger->TimeInterval > 0.0f)
		{
			trigger->Timer += deltaTime;

			if(trigger->Timer >= trigger->TimeInterval)
			{
				trigger->Timer = 0.0f;
				PlaySFXTrigger(sfxEmitter, trigger);
			}
		}
		else
		{
			PlaySFXTrigger(sfxEmitter, trigger);
		}

		if(trigger->SFXHandle != SFX_HANDLE_INVALID)
			g_SSAudio.SetSFXPosition(trigger->SFXHandle, sfxEmitter->Position);
	}
}
//+----------------------------------------------------------------------------+
//|void PlaySFXTrigger(SFXEmitterComponent* &sfxEmitter, SFXTrigger* &trigger)
//|Play a SFX if the camera is within the distance
//\----------------------------------------------------------------------------+
void SSSFXEmitter::PlaySFXTrigger( SFXEmitterComponent* &sfxEmitter, SFXTrigger* &trigger )
{
	///Only trigger sfx if its wihtin range to be heard
	const float distanceMax = trigger->DistanceMax * trigger->DistanceMax;
	const glm::vec3 diff = sfxEmitter->Position - g_SSCamera.GetActiveCamera()->GetPosition();
	const float distanceCamera = diff.x * diff.x + diff.y * diff.y + diff.z + diff.z;
	
	if(distanceCamera > distanceMax)
		return;

	///Play sfx
	SFXEvent event;
	event.Name = trigger->Name;
	event.AtBeat = BeatType::NONE;
	event.Info3D.Is3D = true;
	event.Info3D.Position = sfxEmitter->Position;
	event.Info3D.DistMin = trigger->DistanceMin;
	event.Info3D.DistMax = trigger->DistanceMax;
	trigger->SFXHandle = g_SSAudio.PostEventSFX(event);
	trigger->Triggered = false;
	trigger->LoopCount++;
}