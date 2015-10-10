/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSMetaDataRecorder.h"
#include <fstream>
#include <utility/Logger.h>
#include <utility/Randomizer.h>
#include <utility/SerializationUtility.h>
#include "ReplayUtility.h"
#include "SSReplayRecorder.h"
#include "../hashing/SSHashGenerator.h"

using namespace SerializationUtility;
using namespace ReplayUtility;

SSMetaDataRecorder& SSMetaDataRecorder::GetInstance()
{
	static SSMetaDataRecorder instance;
	return instance;
}

void SSMetaDataRecorder::UpdateSimLayer( const float timeStep )
{
	if ( g_SSReplayRecorder.IsRecording() && g_SSReplayRecorder.HasNewReplayFrame() )
	{
		// Get the latest replay frame
		ReplayFrame* currentReplayFrame		= g_SSReplayRecorder.GetCurrentReplayFrame();

		// Save the meta data
		currentReplayFrame->Hash			= g_SSHashGenerator.GetHash();
		currentReplayFrame->RandomCounter	= g_Randomizer.GetRandomizationCounter();
	}
}