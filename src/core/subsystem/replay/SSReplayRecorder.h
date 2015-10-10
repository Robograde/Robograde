/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <messaging/Subscriber.h>
#include "ReplayUtility.h"

// Conveniency access function
#define g_SSReplayRecorder SSReplayRecorder::GetInstance( )

class SSReplayRecorder : public Subsystem, public Subscriber
{
public:
	static SSReplayRecorder& GetInstance();

	void Startup() override;
	void UpdateSimLayer( const float timeStep ) override;
	void Shutdown() override;

	ReplayFrame*		GetCurrentReplayFrame();
	bool				IsRecording() const;
	bool				HasNewReplayFrame() const;

private:
	// No external instancing allowed
	SSReplayRecorder() : Subsystem( "ReplayRecorder" ), Subscriber( "ReplayRecorder" ) {}
	SSReplayRecorder( const SSReplayRecorder& rhs );
	~SSReplayRecorder() {};
	SSReplayRecorder& operator=( const SSReplayRecorder& rhs );

	void			SerializeToFile( const rString& filePath ) const;
	const rString&	GetFilePath();

	rVector<ReplayFrame>	m_ReplayFrames;
	rString					m_ReplayFilePath;
	bool					m_ShouldSaveReplay; // TODODB: Make sure this is set correctly in all cases
	bool					m_HasNewReplayFrame;

	rVector<rVector<int>>	m_StatingAlliances;

	const rString			BESTWEIGHTS_FILE_PATH	= "../../../asset/ai/bestweights.txt"; // TODODB: Put this in a separate file, they are used from at least 3 files
	const rString			GENES0_FILE_PATH		= "../../../asset/ai/genes0.txt";
	const rString			GENES1_FILE_PATH		= "../../../asset/ai/genes1.txt";
	const rString			GENES2_FILE_PATH		= "../../../asset/ai/genes2.txt";
	const rString			GENES3_FILE_PATH		= "../../../asset/ai/genes3.txt";
};