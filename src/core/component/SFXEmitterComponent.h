/**************************************************
Copyright 2015 Ola Enberg
***************************************************/

#pragma once
#include <glm/glm.hpp>
#include <memory/Alloc.h>

#include "../subsystem/audio/SSAudio.h"

//----+-------------------------+----------------------
//----| SFXTriggerType		|
//----+-------------------------+----------------------
//----|Describes the effect that will trigger the SFX											TODOJS: Add all the trigger event types when the information needed is available
//----+-----------------+------------------------------
enum class SFXTriggerType
{
	AT_CREATION,			///Starts when the entity is created							(SOMEWHAT_IMPLEMENTED)
	AT_MOVE_WALK,			///Trigger when agent moves to an empty location				(NOT_IMPLEMENTED)
	AT_MOVE_EAT,			///Trigger when agent moves to eat	a resource					(SOMEWHAT_IMPLEMENTED)
	AT_MOVE_ATTACK,			///Trigger when agent moves to attack something					(NOT_IMPLEMENTED)

	AT_DEATH,				///Trigger when squad dies										(NOT_IMPLEMENTED)
	AT_TAKING_DAMAGE,		///Trigger when the squad takes damge							(NOT_IMPLEMENTED)

	WHILE_MOVING,			///Trigger when agent moves										(SOMEWHAT_IMPLEMENTED)
	WHILE_EATING,			///Trigger when agent is eating									(SOMEWHAT_IMPLEMENTED)
	WHILE_ATTACKING,		///Trigger when agent is fireing its weapon						(SOMEWHAT_IMPLEMENTED)
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
		Timer = 0.0f;
		TimeInterval = 0.0f;
		DistanceMin = 0.0f;
		DistanceMax = 0.0f;

		Triggered = false;
		Loops = 1;
		LoopCount = 0;
		SFXHandle = 0;
	}

	rString			Name;
	SFXTriggerType	TriggerType;

	int				Loops,			///0 == infinitive, 1 = once etc.
					LoopCount;

	float			Timer,				
					TimeInterval,
					DistanceMin,
					DistanceMax;

	bool			Triggered;
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
	rVector<SFXTriggerType>	m_Events;

	SFXEmitterComponent( )
	{
		Position = glm::vec3( FLT_MIN );
		SFXTriggers.clear( );
		m_Events.clear( );
	}
};