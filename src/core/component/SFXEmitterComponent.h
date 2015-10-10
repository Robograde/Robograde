/**************************************************
2015 Ola Enberg
***************************************************/

#pragma once
#include <glm/glm.hpp>
#include <memory/Alloc.h>

#include "../subsystem/audio/SSAudio.h"

//----+-------------------------+----------------------
//----| SFXTriggerType		|
//----+-------------------------+----------------------
//----|Describes the action that will trigger the SFX
//----|The action must be implemented by the entity that uses this emitter
//----+-----------------+------------------------------
enum class SFXTriggerType
{
	AT_CREATION,			///Starts when the entity is created		
	AT_DEATH,				///Trigger when it dies						///Currently not supported, entity is destroyed along with its soundEmitter at death
	AT_TAKING_DAMAGE,		///Trigger when taking damge		

	WHILE_MOVING,			///Trigger when entity moves						
	WHILE_EATING,			///Trigger when entity is eating					
	WHILE_ATTACKING,		///Trigger when entity is fireing its weapon	

	AT_MOVE_WALK,			///Trigger when agent moves to an empty location
	AT_MOVE_EAT,			///Trigger when agent moves to eat	a resource	
	AT_MOVE_ATTACK,			///Trigger when agent moves to attack something	
};

//+===+================================================
//----| SFXTrigger	|
//----+-----------------+------------------------------
//----|Contains the information about the sfx that should be played through the soundemitter
//+===+================================================
struct SFXTrigger
{
	SFXTrigger(void)
	{
		Reset();
	}

	void Reset(void)
	{
		Name = "";
		Path = "";
		TriggerType = SFXTriggerType::AT_CREATION;
		Timer = 0.0f;
		TimeInterval = 0.0f;
		DistanceMin = 0.0f;
		DistanceMax = 0.0f;

		Triggered = false;
		Looping = false;
		SFXHandle = 0;
	}

	rString			Name;
	rString			Path;
	SFXTriggerType	TriggerType;

	float			Timer,				
					TimeInterval,
					DistanceMin,
					DistanceMax;

	bool			Triggered,
					Looping;

	SFX_HANDLE		SFXHandle;
};

//+===+================================================
//----| SFXEmitterComponent	|
//----+-----------------+------------------------------
//----|Used as a component to play a SFXs at a certain location
//----|Stores SFXTriggers to know what sound that should be played at a certain condition
//----|The idea is to combine this into an entity to be able to automatically play an arbitary number of sounds at its location
//+===+================================================
struct SFXEmitterComponent
{
	glm::vec3				Position;
	rVector<SFXTrigger>		SFXTriggers;
	rVector<SFXTriggerType>	Events;

	SFXEmitterComponent( )
	{
		Position = glm::vec3( FLT_MIN );
		SFXTriggers.clear( );
		Events.clear( );
	}
};