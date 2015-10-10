/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <messaging/Message.h>
#include <messaging/Subscriber.h>
#include "SSReplayRecorder.h"
#include "ReplayUtility.h"

// Conveniency access function
#define g_SSReplayPlayer SSReplayPlayer::GetInstance( )

class SSReplayPlayer : public Subsystem, public Subscriber
{
public:
	static SSReplayPlayer& GetInstance();

	void Startup() override;
	void UpdateSimLayer( const float timeStep ) override;
	void Shutdown() override;

	void			SetReplayIndex( int replayIndex );

	unsigned int	GetHash() const;
	unsigned int	GetRandomCount() const;
	bool			CurrentFrameRecorded() const;

private:
	// No external instancing allowed
	SSReplayPlayer() : Subsystem( "ReplayPlayer" ), Subscriber( "ReplayPlayer" ) {}
	SSReplayPlayer( const SSReplayPlayer& rhs );
	~SSReplayPlayer() {};
	SSReplayPlayer& operator=( const SSReplayPlayer& rhs );

	bool DeSerializeFromFile( const rString& filePath );

	rVector<ReplayFrame>	m_ReplayFrames;
	unsigned int			m_ReplayFramesIndex					= 0;
	bool					m_EndReached						= false;
	bool					m_ReplayLoaded						= false;
	bool					m_ThisFrameRecorded					= false; // TODODB: Refactor stuff related to this variable if possible. Maybe flip subsystems executionorder will help?

	unsigned int			m_CurrentReplayFrameHash			= 0;
	unsigned int			m_CurrentReplayFramRandomCount		= 0;

	int						m_ReplayIndex						= -1;

	const rString			BESTWEIGHTS_FILE_PATH	= "../../../asset/ai/bestweights.txt";
	const rString			GENES0_FILE_PATH		= "../../../asset/ai/genes0.txt";
	const rString			GENES1_FILE_PATH		= "../../../asset/ai/genes1.txt";
	const rString			GENES2_FILE_PATH		= "../../../asset/ai/genes2.txt";
	const rString			GENES3_FILE_PATH		= "../../../asset/ai/genes3.txt";
};