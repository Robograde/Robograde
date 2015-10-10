/**************************************************
Copyright 2015 Robograde Development Team
***************************************************/

#include "GameData.h"
#include <glm/glm.hpp>
#include <utility/ConfigManager.h>
#include <utility/Logger.h>
#include "GameModeSelector.h"

GameData& GameData::GetInstance()
{
	static GameData instance;
	return instance;
}

void GameData::Reset()
{
	m_FrameCount		= 1; // Should be 1 or else clients will be 2 frames behind instead of 1

	m_FieldWidth		= 0;
	m_FieldHeight		= 0;
	m_SpawnPoints.clear();
	m_Players.clear();
	
	ReadGraphicsConfig();
}

bool GameData::ReadGameSettingsConfig()
{
	CallbackConfig* config = g_ConfigManager.GetConfig( "gamesettings.cfg" );
	if ( config == nullptr )
	{
		Logger::Log( "Failed to read game settings config", "GameData", LogSeverity::ERROR_MSG );
		return false;
	}

	m_DefaultGameModeName	= config->GetString( "DefaultGameMode", "mainmenu" );

	return true;
}

bool GameData::ReadGraphicsConfig()
{
	CallbackConfig* gfxConfig = g_ConfigManager.GetConfig( "graphics.cfg" );
	if ( gfxConfig == nullptr )
		assert(false);
	int windowWidth 		= gfxConfig->GetInt( "width", 1600, " Width of the game window" );
	int windowHeight		= gfxConfig->GetInt( "height", 900, " Height of the game window" );
	const bool vsync		= gfxConfig->GetBool( "vsync", false, " Vertical syncronisation" );
	const bool fullscreen	= gfxConfig->GetBool( "fullscreen", false, " Fullscreen" );
	const int msaa			= gfxConfig->GetInt( "msaa", 1, " Multi Sample Anti-Aliasing" );

	SetWindowSize( windowWidth, windowHeight );
	m_Vsync = vsync;
	m_Fullscreen = fullscreen;
	m_MSAA = msaa;

	return true;
}

void GameData::AddSpawnPoint( glm::vec3 spawn )
{
	m_SpawnPoints.push_back( spawn );
}

void GameData::AddPlayer( short playerID, bool isAI )
{
	assert( m_Players.size() < MAX_PLAYERS ); // Assert if the list isn't cleared
	m_Players.push_back( Player { playerID, isAI } );
}

void GameData::RemovePlayer( short playerID )
{
	for ( auto it = m_Players.begin(); it != m_Players.end(); ++it )
	{
		if ( it->PlayerID == playerID )
			m_Players.erase( it );
	}
}

void GameData::ClearPlayers()
{
	m_Players.clear();
}

void GameData::IncrementFrameCount()
{
	++m_FrameCount;
}

void GameData::ResetSpawnPoints( )
{
	m_PlayerSpawnPoints.clear( );
}

unsigned int GameData::GetFrameCount() const
{
	return m_FrameCount;
}

int GameData::GetWindowHeight() const
{
	return m_WindowHeight;
}

int GameData::GetWindowWidth() const
{
	return m_WindowWidth;
}

int GameData::GetFieldHeight() const
{
	return m_FieldHeight;
}

int GameData::GetFieldWidth() const
{
	return m_FieldWidth;
}

const rVector<glm::vec3>& GameData::GetSpawnPoints() const
{
	return m_SpawnPoints;
}

const rString& GameData::GetDefaultGameModeName() const
{
	return m_DefaultGameModeName;
}

const rString& GameData::GetCurrentMapName() const
{
	return m_CurrentMap;
}

const glm::vec4& GameData::GetPlayerColour( short playerID ) const
{
	assert( playerID < MAX_PLAYERS );
	assert( playerID < static_cast<short>( m_PlayerColours.size() ) );
	if ( playerID == -1 )
		return m_DefaultColour;
	return m_PlayerColours.at( playerID );
}

int GameData::GetPlayerSpawnPoint( short playerID )
{
	int firstFree = -1;
	for ( unsigned int i = 0; i < m_PlayerSpawnPoints.size(); ++i )
	{
		if ( m_PlayerSpawnPoints.at( i ) == -1 && firstFree == -1 )
		{
			firstFree = static_cast<int>( i );
		}
		else if ( m_PlayerSpawnPoints.at( i ) == playerID )
		{
			return i;
		}
	}
	if ( firstFree == -1 )
		firstFree = static_cast<int>( m_PlayerSpawnPoints.size() );

	SetPlayerSpawnPoint( firstFree, playerID );
	return firstFree;
}

const rVector<GameData::Player>& GameData::GetPlayers() const
{
	return m_Players;
}

void GameData::SetWindowSize( int width, int height )
{
	m_WindowWidth	= width;
	m_WindowHeight	= height;
}

void GameData::SetFieldHeight( const int val )
{
	m_FieldHeight = val;
}

void GameData::SetFieldWidth( const int val )
{
	m_FieldWidth = val;
}


void GameData::SetCurrentMapName( const rString& currentMap )
{
	m_CurrentMap = currentMap;
}

void GameData::SetPlayerColour( short playerID, const glm::vec4& colour )
{
	assert( playerID >= 0 && playerID < MAX_PLAYERS );
	if ( m_PlayerColours.size() <= static_cast<size_t>(playerID) )
		m_PlayerColours.resize( playerID + 1);
	m_PlayerColours.at( playerID ) = colour;
}

void GameData::SetPlayerSpawnPoint( int spawnPointIndex, short playerID )
{
	// Reset old position
	for ( auto& spawn : m_PlayerSpawnPoints )
	{
		spawn = spawn == playerID ? -1 : spawn;
	}
	if ( spawnPointIndex != -1)
	{
		if ( static_cast<int>( m_PlayerSpawnPoints.size( ) ) <= spawnPointIndex )
		{
			size_t prevSize = m_PlayerSpawnPoints.size( );
			m_PlayerSpawnPoints.resize( spawnPointIndex + 1 );
			std::fill( m_PlayerSpawnPoints.begin() + prevSize, m_PlayerSpawnPoints.end(), -1 );
		}
		m_PlayerSpawnPoints.at( spawnPointIndex ) = playerID;
	}
}

bool GameData::IsDedicatedServer() const
{
	return m_DefaultGameModeName == "dedicatedserver";  //TODODB: Stop comparing strings!
}
