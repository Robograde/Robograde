/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSReplayPlayer.h"
#include <fstream>
#include <utility/Logger.h>
#include <utility/SerializationUtility.h>
#include <utility/Randomizer.h>
#include <utility/DataSizes.h>
#include "../gamelogic/SSAI.h"
#include "../gamelogic/SSSceneLoader.h"
#include "../gui/SSMiniMap.h"
#include "../gui/SSGUIInfo.h"
#include "../utility/SSMail.h"
#include "../../utility/GameData.h"
#include "../../utility/Alliances.h"

using namespace SerializationUtility;
using namespace ReplayUtility;
using namespace DataSizes;

#define CREATE_DEBUG_REPLAY false

SSReplayPlayer& SSReplayPlayer::GetInstance()
{
	static SSReplayPlayer instance;
	return instance;
}

void SSReplayPlayer::Startup()
{
	rString replayPath;
	if ( m_ReplayIndex == -1 )
		replayPath = FileUtility::FindLastIncrementationOfPath( DEFAULT_REPLAY_FILE_PATH.c_str(), REPLAY_FILE_EXTENSION.c_str(), 0, MAX_SAVED_REPLAYS );
	else
		replayPath = DEFAULT_REPLAY_FILE_PATH.c_str() + rToString( m_ReplayIndex ) + REPLAY_FILE_EXTENSION.c_str();

	if ( replayPath != "" )
		m_ReplayLoaded = DeSerializeFromFile( replayPath );
	else
	{
		Logger::Log( "Failed to find any replay file (Is the replay folder empty?)", "SSReplayPlayer", LogSeverity::WARNING_MSG );
		m_ReplayLoaded = false;
	}
}

void SSReplayPlayer::UpdateSimLayer( const float timeStep )
{
	if ( m_ReplayLoaded && !m_EndReached ) // If a replay file is loaded and there is replaying to be done!
	{
		if ( g_GameData.GetFrameCount() == m_ReplayFrames[m_ReplayFramesIndex].ID ) // Check if the current frame is a recorded frame
		{
			m_ThisFrameRecorded = true;

			// Read all the data for that frame
			ReplayFrame* currentReplayFrame		= &m_ReplayFrames[m_ReplayFramesIndex];
			m_CurrentReplayFrameHash			= currentReplayFrame->Hash;
			m_CurrentReplayFramRandomCount		= currentReplayFrame->RandomCounter;
			for ( int i = 0; i < m_ReplayFrames[m_ReplayFramesIndex].Messages.size(); ++i ) // Push all the messages of the recorded frame to the mail subsystem
					g_SSMail.PushToCurrentFrame( *currentReplayFrame->Messages[i] );

			// Check if we reached the end of the replay
			if ( m_ReplayFramesIndex + 1 == m_ReplayFrames.size() )
			{
				m_EndReached = true;
				Logger::Log( "End of replay reached", "SSReplayPlayer", LogSeverity::INFO_MSG );
				g_SSGUIInfo.DisplayMessage( "Replay over" );
			}
			else
				++m_ReplayFramesIndex;
		}
		else m_ThisFrameRecorded = false;
	}
	else
		m_ThisFrameRecorded = false;
}

void SSReplayPlayer::Shutdown()
{
	m_ReplayFrames.clear();
	m_ReplayFramesIndex = 0;
	m_EndReached		= false;
	m_ReplayLoaded		= false;
	m_ThisFrameRecorded = false;

	m_CurrentReplayFrameHash		= 0;
	m_CurrentReplayFramRandomCount	= 0;

	m_ReplayIndex = -1;
}

void SSReplayPlayer::SetReplayIndex( int replayIndex )
{
	m_ReplayIndex = replayIndex;
}

unsigned int SSReplayPlayer::GetHash() const
{
	return m_CurrentReplayFrameHash;
}

unsigned int SSReplayPlayer::GetRandomCount() const
{
	return m_CurrentReplayFramRandomCount;
}

bool SSReplayPlayer::CurrentFrameRecorded() const
{
	return m_ThisFrameRecorded;
}

bool SSReplayPlayer::DeSerializeFromFile( const rString& filePath )
{
	bool result = false;

	Byte* buffer = FileUtility::GetFileContent( filePath, std::ios::binary );

	if ( buffer != nullptr )
	{
		unsigned int			savedFramesCount;
		unsigned int			randomizerSeed;
		short					playerCount;
		short					playerIDs[MAX_PLAYERS];
		bool					playerISAI[MAX_PLAYERS];
		glm::vec4				playerColours[MAX_PLAYERS];
		short					allianceCount;
		rVector<rVector<int>>	alliances;
		short					startingUnitsCount;
		unsigned int			sceneNameSize;
		rString					sceneName;
		unsigned int			minimapTexturePathSize;
		rString					minimapTexturePath;
		short					spawnIndices[MAX_PLAYERS];
		short					spawnPointSize;
		rVector<glm::vec3>		spawnPoints;

		const Byte* walker = buffer;

		// Replay frame count
		CopyAndIncrementSource( &savedFramesCount, walker, INT_SIZE );

		// Randomizer seed
		CopyAndIncrementSource( &randomizerSeed, walker, INT_SIZE );

		// Player count
		CopyAndIncrementSource( &playerCount, walker, SHORT_SIZE );

		// Player data
		for ( int i = 0; i < playerCount; ++i )
		{
			CopyAndIncrementSource( &playerIDs[i], walker, SHORT_SIZE );
			CopyAndIncrementSource( &playerISAI[i], walker, BOOL_SIZE );
			CopyAndIncrementSource( &playerColours[i], walker, FLOAT_SIZE * 4 );
		}

		// Check if there is an AI player present i the replay
		bool aiExists = false;
		for ( int i = 0; i < playerCount; ++i )
		{
			if ( playerISAI[i] )
				aiExists = true;
		}

		unsigned int	bestWeightsSize;
		unsigned int	genes0Size;
		unsigned int	genes1Size;
		unsigned int	genes2Size;
		unsigned int	genes3Size;

		char*			bestWeightsContent;
		char*			genes0Content;
		char*			genes1Content;
		char*			genes2Content;
		char*			genes3Content;

		// Read AI parameters
		if ( aiExists )
		{
			CopyAndIncrementSource( &bestWeightsSize, walker, INT_SIZE );
			bestWeightsContent = static_cast<char*>( fMalloc( bestWeightsSize ) );
			CopyAndIncrementSource( bestWeightsContent, walker, bestWeightsSize );

			CopyAndIncrementSource( &genes0Size, walker, INT_SIZE );
			genes0Content = static_cast<char*>( fMalloc( genes0Size ) );
			CopyAndIncrementSource( genes0Content, walker, genes0Size );

			CopyAndIncrementSource( &genes1Size, walker, INT_SIZE );
			genes1Content = static_cast<char*>( fMalloc( genes1Size ) );
			CopyAndIncrementSource( genes1Content, walker, genes1Size );

			CopyAndIncrementSource( &genes2Size, walker, INT_SIZE );
			genes2Content = static_cast<char*>( fMalloc( genes2Size ) );
			CopyAndIncrementSource( genes2Content, walker, genes2Size );

			CopyAndIncrementSource( &genes3Size, walker, INT_SIZE );
			genes3Content = static_cast<char*>( fMalloc( genes3Size ) );
			CopyAndIncrementSource( genes3Content, walker, genes3Size );
		}

		// Alliances
		CopyAndIncrementSource( &allianceCount, walker, SHORT_SIZE );
		for ( int i = 0; i < allianceCount; ++i )
		{
			short size;
			CopyAndIncrementSource( &size, walker, SHORT_SIZE );

			rVector<int> allianceEntries;
			allianceEntries.reserve( size );
			allianceEntries.insert( allianceEntries.begin(), reinterpret_cast<const int*>( walker ), reinterpret_cast<const int*>( walker ) + size );
			walker += size * INT_SIZE;
			alliances.push_back( allianceEntries ); // TODODB: Is this pushed in the corrrect order now?
		}

		// Starting units count
		CopyAndIncrementSource( &startingUnitsCount, walker, SHORT_SIZE );

		// Scene name
		CopyAndIncrementSource( &sceneNameSize, walker, INT_SIZE );
		sceneName.reserve( sceneNameSize );
		char* sceneNameData = fAlloc( char, sceneNameSize + 1 );
		CopyAndIncrementSource( sceneNameData, walker, sceneNameSize * CHAR_SIZE );
		sceneNameData[sceneNameSize] = '\0';
		sceneName = rString( sceneNameData );
		fFree( sceneNameData );

		// Minimap texture path
		CopyAndIncrementSource( &minimapTexturePathSize, walker, INT_SIZE );
		minimapTexturePath.reserve( minimapTexturePathSize );
		char* minimapTexturePathData = fAlloc( char, minimapTexturePathSize * CHAR_SIZE + 1 );
		CopyAndIncrementSource( minimapTexturePathData, walker, minimapTexturePathSize * CHAR_SIZE );
		minimapTexturePathData[minimapTexturePathSize] = '\0';
		minimapTexturePath = rString( minimapTexturePathData );
		fFree( minimapTexturePathData );

		// Spawn point indices
		for ( int i = 0; i < playerCount; ++i )
			CopyAndIncrementSource( &spawnIndices[i], walker, SHORT_SIZE );

		// Spawn points
		CopyAndIncrementSource( &spawnPointSize, walker, SHORT_SIZE );
		spawnPoints.reserve( spawnPointSize );
		spawnPoints.insert( spawnPoints.begin(), reinterpret_cast<const glm::vec3*>( walker ), reinterpret_cast<const glm::vec3*>( walker ) + spawnPointSize );
		walker += spawnPointSize * FLOAT_SIZE * 3;

		// Replay frames
		if ( savedFramesCount != 0 )
		{
			m_ReplayFrames.reserve( savedFramesCount );
			for ( unsigned int i = 0; i < savedFramesCount; ++i )
			{
				ReplayFrame replayFrame;
				replayFrame.Deserialize( walker );
				m_ReplayFrames.push_back( replayFrame );
			}
		}
		else
		{
			m_EndReached = true;
			Logger::Log( "Replay contained no recorded frames", "SSReplayPlayer", LogSeverity::WARNING_MSG );
		}

#if REPLAY_DEBUG == 1
		size_t size = FileUtility::GetFileContentSize( filePath, std::ios::binary );
		if ( walker - buffer != size )
		{
			int diff = walker - buffer - size;
			assert( false );
		}
#endif

		// Seed the randomizer
		g_Randomizer.Seed( randomizerSeed );

		// Create players
		for ( int i = 0; i < playerCount; ++i )
			g_GameData.AddPlayer( playerIDs[i], playerISAI[i] );

		// Write AI player parameters
		if ( aiExists )
		{
			FileUtility::WriteToFile( bestWeightsContent, bestWeightsSize, BESTWEIGHTS_FILE_PATH );
			FileUtility::WriteToFile( genes0Content, genes0Size, GENES0_FILE_PATH );
			FileUtility::WriteToFile( genes1Content, genes1Size, GENES1_FILE_PATH );
			FileUtility::WriteToFile( genes2Content, genes2Size, GENES2_FILE_PATH );
			FileUtility::WriteToFile( genes3Content, genes3Size, GENES3_FILE_PATH );

			fFree( bestWeightsContent );
			fFree( genes0Content );
			fFree( genes1Content );
			fFree( genes2Content );
			fFree( genes3Content );
		}

		// Set player colours
		for ( int i = 0; i < playerCount; ++i )
			g_GameData.SetPlayerColour( playerIDs[i], playerColours[i] );

		// Set alliances
		g_Alliances.SetAlliances( alliances );

		// Set starting units count
		g_SSAI.SetSpawnCount( startingUnitsCount );

		// Set scene name
		g_SSSceneLoader.SetSceneToBeLoaded( sceneName );

		// Set minimap texture path
		g_SSMiniMap.SetMiniMapTexture( minimapTexturePath );

		// Set spawn points
		for ( int i = 0; i < spawnPointSize; ++i )
			g_GameData.AddSpawnPoint( spawnPoints[i] );

		// Set spawn points indices
		for ( int i = 0; i < playerCount; ++i )
			g_GameData.SetPlayerSpawnPoint( spawnIndices[i], i );

		Logger::Log( "Replay file loaded - Path = " + filePath, "SSReplayPlayer", LogSeverity::INFO_MSG );
		result = true;
		fFree( buffer ); // The buffer has been copied into relevant structures so we can free the buffer
	}
	else
		Logger::Log( "Failed to open file - Path = " + filePath, "SSReplayPlayer", LogSeverity::WARNING_MSG );

	return result;
}