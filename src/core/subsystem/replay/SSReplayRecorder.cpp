/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSReplayRecorder.h"
#include <fstream>
#include <utility/Logger.h>
#include <utility/SerializationUtility.h>
#include <utility/Randomizer.h>
#include <utility/FileUtility.h>
#include <utility/DataSizes.h>
#include <messaging/GameMessages.h>
#include "../gamelogic/SSAI.h"
#include "../gamelogic/SSSceneLoader.h"
#include "../gui/SSMiniMap.h"
#include "../hashing/SSHashGenerator.h"
#include "../utility/SSMail.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../utility/Alliances.h"

using namespace SerializationUtility;
using namespace ReplayUtility;
using namespace MessageTypes;
using namespace DataSizes;

#define CREATE_DEBUG_REPLAY false // TODODB: Move this to config
#define ONLY_RECORD_RELEVANT_FRAMES true

SSReplayRecorder& SSReplayRecorder::GetInstance()
{
	static SSReplayRecorder instance;
	return instance;
}

void SSReplayRecorder::Startup()
{
	m_ShouldSaveReplay	= false;
	m_HasNewReplayFrame = false;

	m_SimInterests = ORDER_UNITS | ORDER_INVOKE | GAME_OVER | UPGRADE;
	g_SSMail.RegisterSubscriber( this );
	GetFilePath();

	m_StatingAlliances = g_Alliances.GetAlliances();
	
	Subsystem::Startup();
}

void SSReplayRecorder::UpdateSimLayer( const float timeStep )
{
	if ( ( !m_SimMailbox.empty() && m_ShouldSaveReplay ) || !ONLY_RECORD_RELEVANT_FRAMES )
	{
		// Create a new frame
		m_ReplayFrames.push_back( ReplayFrame() );
		ReplayFrame* currentReplayFrame = &m_ReplayFrames[m_ReplayFrames.size() - 1];
		m_HasNewReplayFrame = true;

		// Save the frame count as ID along with the hash and a copy of every simulation message
		currentReplayFrame->ID = g_GameData.GetFrameCount();
		for ( int i = 0; i < m_SimMailbox.size(); ++i )
			currentReplayFrame->Messages.push_back( m_SimMailbox[i]->Clone() );

		if ( CREATE_DEBUG_REPLAY && m_ShouldSaveReplay )
			SerializeToFile( GetFilePath() );
	}
	else
		m_HasNewReplayFrame = false;
}

void SSReplayRecorder::Shutdown()
{
	if ( m_ShouldSaveReplay && m_ReplayFrames.size() > 0 )
		SerializeToFile( GetFilePath() );
	g_SSMail.UnregisterSubscriber( this );

	m_ReplayFrames.clear();
	m_ReplayFilePath	= "";
	m_ShouldSaveReplay	= false;
	m_HasNewReplayFrame = false;

	Subsystem::Shutdown();
}

ReplayFrame* SSReplayRecorder::GetCurrentReplayFrame()
{
	return &m_ReplayFrames[m_ReplayFrames.size() - 1];
}

bool SSReplayRecorder::IsRecording() const
{
	return m_ShouldSaveReplay;
}

bool SSReplayRecorder::HasNewReplayFrame() const
{
	return m_HasNewReplayFrame;
}

void SSReplayRecorder::SerializeToFile( const rString& filePath ) const
{
	if ( FileUtility::CreateFolder( FileUtility::GetDirectoryFromFilePath( filePath ) ) )
	{
		std::ofstream outStream( filePath.c_str(), std::ios::binary );
		if ( outStream.is_open() )
		{
			// Get data to be saved

			// Players
			rVector<GameData::Player> players = g_GameData.GetPlayers();
			short playerCount = static_cast<short>( players.size() );

			// Check if there were any AI players
			bool aiExists = false;
			for ( int i = 0; i < playerCount; ++i )
			{
				if ( players[i].IsAI )
				{
					aiExists = true;
				}
			}

			unsigned int bestWeightsSize	= 0;
			unsigned int genes0Size			= 0;
			unsigned int genes1Size			= 0;
			unsigned int genes2Size			= 0;
			unsigned int genes3Size			= 0;

			const char* bestWeightsContent;
			const char* genes0Content;
			const char*	genes1Content;
			const char*	genes2Content;
			const char*	genes3Content;
			
			if ( aiExists )
			{
				bestWeightsSize		= static_cast<unsigned int>( FileUtility::GetFileContentSize( BESTWEIGHTS_FILE_PATH ) );
				genes0Size			= static_cast<unsigned int>( FileUtility::GetFileContentSize( GENES0_FILE_PATH ) );
				genes1Size			= static_cast<unsigned int>( FileUtility::GetFileContentSize( GENES1_FILE_PATH ) );
				genes2Size			= static_cast<unsigned int>( FileUtility::GetFileContentSize( GENES2_FILE_PATH ) );
				genes3Size			= static_cast<unsigned int>( FileUtility::GetFileContentSize( GENES3_FILE_PATH ) );

				bestWeightsContent	= FileUtility::GetFileContent( BESTWEIGHTS_FILE_PATH );
				genes0Content		= FileUtility::GetFileContent( GENES0_FILE_PATH );
				genes1Content		= FileUtility::GetFileContent( GENES1_FILE_PATH );
				genes2Content		= FileUtility::GetFileContent( GENES2_FILE_PATH );
				genes3Content		= FileUtility::GetFileContent( GENES3_FILE_PATH );
			}

			// Alliances
			rVector<rVector<int>> alliances = m_StatingAlliances;
			short allianceCount = static_cast<short>( alliances.size() );
			unsigned int allianceSize = 0;
			for ( int i = 0; i < allianceCount; ++i )
				allianceSize += static_cast<unsigned int>( alliances[i].size() ) * INT_SIZE + SHORT_SIZE;

			// Starting units count
			short startingUnitsCount = static_cast<short>( g_SSAI.GetSpawnCount() );

			// Scene name
			rString sceneName = g_SSSceneLoader.GetSceneName();
			unsigned int sceneNameSize = static_cast<unsigned int>( sceneName.size() );

			// Minimap texture path
			rString minimapTexturePath = g_SSMiniMap.GetMiniMapTexturePath();
			unsigned int minimapTexturePathSize = static_cast<unsigned int>( minimapTexturePath.size() );

			// Spawn points index
			short spawnPointIndices[MAX_PLAYERS];
			for ( int i = 0; i < playerCount; ++i )
				spawnPointIndices[i] = g_GameData.GetPlayerSpawnPoint( players[i].PlayerID );

			// Spawn points
			rVector<glm::vec3> spawnPoints = g_GameData.GetSpawnPoints();
			short spawnPointsSize = static_cast<short>( spawnPoints.size() );

			// Calculate how big a buffer we are going to need and allocate it
			unsigned int bufferSize = 0;
			//							 Replay frame count		+ Randomizer seed	+ Player count	+ player IDs						+ Player IsAI								+ Player Colours					+ Alliance size				+ Starting units count +
			unsigned int headerSize =	INT_SIZE				+ INT_SIZE			+ SHORT_SIZE	+ ( SHORT_SIZE * playerCount )		+ ( BOOL_SIZE * playerCount )				+ ( FLOAT_SIZE * 4 * playerCount )	+ SHORT_SIZE + allianceSize	+ SHORT_SIZE +
			//							scene name char count	+ scene name						+ Minimap texture path char count	+ Minimap texture path						+ spawn points index				+ spawn points count		+ spawn points	
										INT_SIZE				+ ( sceneNameSize * CHAR_SIZE )		+ INT_SIZE							+ ( minimapTexturePathSize * CHAR_SIZE )	+ ( playerCount * SHORT_SIZE )		+ SHORT_SIZE				+ ( spawnPointsSize * FLOAT_SIZE * 3 );
			if ( aiExists )
				headerSize += (INT_SIZE * 5) + bestWeightsSize + genes0Size + genes1Size + genes2Size + genes3Size; // AI weights

			bufferSize += headerSize;

			// Frame size
			for ( auto& replayFrame : m_ReplayFrames )
				bufferSize += replayFrame.GetSerializationSize();

			// Create buffers
			Byte* buffer = static_cast<char*>( fMalloc( bufferSize ) );
			Byte* walker = buffer; // Used to traverse the buffer

			// Number of recorded frames
			unsigned int savedFramesCount = static_cast<unsigned int>( m_ReplayFrames.size() );
			CopyAndIncrementDestination( walker, &savedFramesCount, INT_SIZE );

			// Randomizer seed
			unsigned int randomizerSeed = g_Randomizer.GetSeed();
			CopyAndIncrementDestination( walker, &randomizerSeed, INT_SIZE );

			// Player Count
			CopyAndIncrementDestination( walker, &playerCount, SHORT_SIZE );

			// Players
			for ( short i = 0; i < playerCount; ++i )
			{
				CopyAndIncrementDestination( walker, &players[i].PlayerID, SHORT_SIZE );
				CopyAndIncrementDestination( walker, &players[i].IsAI, BOOL_SIZE );

				// Colour
				glm::vec4 playerColour = g_GameData.GetPlayerColour( players[i].PlayerID );
				CopyAndIncrementDestination( walker, &playerColour, FLOAT_SIZE * 4 );
			}

			// Save weights and genes

			Byte* oldWalker = walker;

			if ( aiExists )
			{
				CopyAndIncrementDestination( walker, &bestWeightsSize, INT_SIZE );
				CopyAndIncrementDestination( walker, bestWeightsContent, bestWeightsSize );
				CopyAndIncrementDestination( walker, &genes0Size, INT_SIZE );
				CopyAndIncrementDestination( walker, genes0Content, genes0Size );
				CopyAndIncrementDestination( walker, &genes1Size, INT_SIZE );
				CopyAndIncrementDestination( walker, genes1Content, genes1Size );
				CopyAndIncrementDestination( walker, &genes2Size, INT_SIZE );
				CopyAndIncrementDestination( walker, genes2Content, genes2Size );
				CopyAndIncrementDestination( walker, &genes3Size, INT_SIZE );
				CopyAndIncrementDestination( walker, genes3Content, genes3Size );

				fFree( bestWeightsContent );
				fFree( genes0Content );
				fFree( genes1Content );
				fFree( genes2Content );
				fFree( genes3Content );
			}

			// Alliances
			CopyAndIncrementDestination( walker, &allianceCount, SHORT_SIZE );
			for ( int i = 0; i < allianceCount; ++i )
			{
				short size = static_cast<short>( alliances[i].size() );
				CopyAndIncrementDestination( walker, &size, SHORT_SIZE );
				CopyAndIncrementDestination( walker, alliances[i].data(), size * INT_SIZE );
			}

			// Starting units count
			CopyAndIncrementDestination( walker, &startingUnitsCount, SHORT_SIZE );

			// Scene name
			CopyAndIncrementDestination( walker, &sceneNameSize, INT_SIZE );
			CopyAndIncrementDestination( walker, sceneName.data(), sceneNameSize );
			
			// Minimap texture path
			CopyAndIncrementDestination( walker, &minimapTexturePathSize, INT_SIZE );
			CopyAndIncrementDestination( walker, minimapTexturePath.data(), minimapTexturePathSize );

			// Spawn points
			for ( int i = 0; i < playerCount; ++i )
				CopyAndIncrementDestination( walker, &spawnPointIndices[i], SHORT_SIZE );

			// Spawn points
			CopyAndIncrementDestination( walker, &spawnPointsSize, SHORT_SIZE );
			CopyAndIncrementDestination( walker, spawnPoints.data(), spawnPointsSize * FLOAT_SIZE * 3 );

			// Frames
			for ( auto& replayFrame : m_ReplayFrames )
				replayFrame.Serialize( walker );

			// Check if the serialization size was correct and dump to file
			if ( walker - buffer == bufferSize )
				outStream.write( buffer, bufferSize );
			else
			{
				Logger::Log( "Failed to save replay - Path = " + filePath, "SSReplayRecorder", LogSeverity::WARNING_MSG );
				#if REPLAY_DEBUG == 1
					int diff =  walker - buffer - bufferSize;
					assert( false );
				#endif
			}

			fFree( buffer ); // Everything is either dumped to file or irrelevant due to an error so we can free the buffer
		}
		else
			Logger::Log( "Failed to open replay file for writing - Path = " + filePath, "SSReplayRecorder", LogSeverity::WARNING_MSG );
	}
	else
		Logger::Log( "Failed to save replay - Could not create replay directory", "SSReplayRecorder", LogSeverity::WARNING_MSG );
}

const rString& SSReplayRecorder::GetFilePath()
{
	if ( m_ReplayFilePath != "" )
		return m_ReplayFilePath;
	else
	{
		m_ReplayFilePath = FileUtility::FindIncrementedPath( DEFAULT_REPLAY_FILE_PATH.c_str(), REPLAY_FILE_EXTENSION.c_str(), 0, MAX_SAVED_REPLAYS );
		if ( m_ReplayFilePath == "" )
		{
			Logger::Log( "Replay folder is full, no replay will be created", "SSReplayRecorder", LogSeverity::WARNING_MSG );
			m_ShouldSaveReplay = false;
		}
		else
			m_ShouldSaveReplay = true;
		return m_ReplayFilePath;
	}
}