/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "GameModeSelector.h"
#include "../CompileFlags.h"
#include <utility/Logger.h>
#include <utility/Config.h>
#include <utility/NeatFunctions.h>
#include <utility/Randomizer.h>
#include <gui/GUIEngine.h>
#include <network/NetworkEngine.h>
#include <network/NetworkInfo.h>
#include <input/Input.h>
#include "GameSpeedController.h"
#include "../utility/GameData.h"
#include "../utility/PlayerData.h"
#include "../utility/Alliances.h"
#include "../subsystem/SubsystemManager.h"
#include "../subsystem/audio/SSMusicManager.h"
#include "../subsystem/audio/SSAudio.h"
#include "../subsystem/audio/SSSFXNotification.h"

GameModeSelector& GameModeSelector::GetInstance()
{
	static GameModeSelector instance;
	return instance;
}

void GameModeSelector::Initialize()
{
	// Create game modes (Names must be in lower case)											Sim		Net		Menu
	m_GameModes.push_back( GameMode( GameModeType::ConsoleOnly,			"console",				false,	false,	false	) );
	m_GameModes.push_back( GameMode( GameModeType::Singleplayer,		"singleplayer",			true,	false,	false	) );
	m_GameModes.push_back( GameMode( GameModeType::SingleplayerAI,		"singleplayerai",		true,	false,	false	) );
	m_GameModes.push_back( GameMode( GameModeType::AIOnly,				"aionly",				true,	false,	false	) );
	m_GameModes.push_back( GameMode( GameModeType::Multiplayer,			"multiplayer",			true,	true,	false	) );
	m_GameModes.push_back( GameMode( GameModeType::NetworkLobby,		"networklobby",			false,	true,	true	) );
	m_GameModes.push_back( GameMode( GameModeType::SinglePlayerLobby,	"singleplayerlobby",	false,  false,	true	) );
	m_GameModes.push_back( GameMode( GameModeType::Replay,				"replay",				true,	false,	false	) );
	m_GameModes.push_back( GameMode( GameModeType::MainMenu,			"mainmenu",				false,	false,	true	) );
	m_GameModes.push_back( GameMode( GameModeType::DedicatedServer,		"dedicatedserver",		true,	true,	false	) );
	m_GameModes.push_back( GameMode( GameModeType::Editor,				"editor",				true,	true,	false	) );
	
	// Add the game modes to the game mode map
	for ( auto& gameMode : m_GameModes )
		m_GameModeNames.emplace( gameMode.Name, gameMode.Type );

	// Add aliases (Must be lower case)
	m_GameModeNames.emplace( "sp", GameModeType::Singleplayer );
	m_GameModeNames.emplace( "spai", GameModeType::SingleplayerAI );
	m_GameModeNames.emplace( "mp", GameModeType::Multiplayer );
	m_GameModeNames.emplace( "netlob", GameModeType::NetworkLobby );
	m_GameModeNames.emplace( "splob", GameModeType::SinglePlayerLobby );
	m_GameModeNames.emplace( "c", GameModeType::ConsoleOnly );
	m_GameModeNames.emplace( "rep", GameModeType::Replay );
	m_GameModeNames.emplace( "mm", GameModeType::MainMenu );
	m_GameModeNames.emplace( "edit", GameModeType::Editor );

	g_SubsystemManager.Startup();

	SwitchToGameModebyString( g_GameData.GetDefaultGameModeName() );

	DEV(
		g_Script.Register( "GE_Restart", std::bind( &GameModeSelector::ScriptRestart, this, std::placeholders::_1 ) );
		g_Script.Register( "GE_PrintGameModesSubsystems", std::bind( &GameModeSelector::ScriptPrintGameModeSubsystems, this, std::placeholders::_1 ) );
	);
	g_Script.Register( "GE_SwitchGameMode", std::bind( &GameModeSelector::ScriptSwitchGameMode, this, std::placeholders::_1 ) );
}

const GameMode& GameModeSelector::GetGameModeFromString( const rString& name ) const
{
	GameModeType type = GetGameModeTypeFromString( name );
	for ( int i = 0; i < m_GameModes.size(); ++i )
	{
		if ( m_GameModes[i].Type == type )
			return m_GameModes[i];
	}
	return INVALID_GAME_MODE;
}

GameModeType GameModeSelector::GetGameModeTypeFromString( const rString& gameModeName ) const
{
	auto gameModeType = m_GameModeNames.find( gameModeName );
	if ( gameModeType != m_GameModeNames.end() )
		return gameModeType->second;
	else
	{
		Logger::Log( "Attempted to get nonexistent game mode type <" + gameModeName + ">", "GameModeSelector", LogSeverity::WARNING_MSG );
		return GameModeType::Unknown;
	}
}

void GameModeSelector::Update()
{
	if ( m_NextGameMode.Type != GameModeType::Unknown )
	{
		const GameMode& gameMode = m_NextGameMode;
		if ( gameMode != INVALID_GAME_MODE )
		{
			StopCurrentMode();
			StartMode( gameMode );
			m_NextGameMode = INVALID_GAME_MODE;
		}
		else
			Logger::Log( "Attempted to start illegal game mode", "GameModeSelector", LogSeverity::WARNING_MSG );
	}
}

void GameModeSelector::AddSubsystemToGameModes( Subsystem* subsystem, std::initializer_list<GameModeType> gameModeTypes )
{
	for ( auto gameModeType : gameModeTypes )
	{
		if ( gameModeType != GameModeType::Unknown )
		{
			GameMode* gameMode = GetEditableGameModeFromType( gameModeType );
			if ( gameMode != nullptr )
				gameMode->Subsystems.push_back( subsystem );
			else
				Logger::Log( "Failed to register subsystem since no game mode was found for type " + rToString( static_cast<int>( gameModeType ) ), "GameModeSelector", LogSeverity::WARNING_MSG );
		}
		else
			Logger::Log( "Attempted to register subsystem to illegal game mode", "GameModeSelector", LogSeverity::WARNING_MSG );
	}

	// Add the subsystem to the subsystem collection
	g_SubsystemManager.AddSubsystem( subsystem ); // TODODB: Check for duplicates
}

void GameModeSelector::SwitchToGameMode( GameModeType gameMode )
{
	m_NextGameMode = GetGameModeFromType( gameMode );
}

bool GameModeSelector::SwitchToGameModebyString( const rString& gameModeName )
{
	GameModeType gameMode = GetGameModeTypeFromString( gameModeName );
	if ( gameMode != GameModeType::Unknown )
	{
		m_NextGameMode = GetGameModeFromType( gameMode );
		return true;
	}
	else
	{
		Logger::Log( "Unable to start game mode <" + gameModeName + ">. No such game mode exists", "GameModeSelector", LogSeverity::WARNING_MSG );
		return false;
	}
}

const GameMode& GameModeSelector::GetCurrentGameMode() const
{
	return m_CurrentGameMode;
}

const GameMode& GameModeSelector::GetGameModeFromType( GameModeType gameModeType ) const
{
	for ( int i = 0; i < m_GameModes.size(); ++i )
	{
		if ( m_GameModes[i].Type == gameModeType )
		{
			return m_GameModes[i];
		}
	}
	Logger::Log( "No game mode of type " + rToString( static_cast< int >( gameModeType ) ) + " could be found", "GameModeSelector", LogSeverity::WARNING_MSG ); // TODODB: Get game mode name as string and output
	return INVALID_GAME_MODE;
}

GameMode* GameModeSelector::GetEditableGameModeFromType( GameModeType gameModeType )
{
	for ( int i = 0; i < m_GameModes.size(); ++i )
	{
		if ( m_GameModes[i].Type == gameModeType )
		{
			return &m_GameModes[i];
		}
	}
	Logger::Log( "No game mode of type " + rToString( static_cast< int >( gameModeType ) ) + " could be found", "GameModeSelector", LogSeverity::WARNING_MSG ); // TODODB: Get game mode name as string and output
	return nullptr;
}

void GameModeSelector::StopCurrentMode()
{
	rVector<Subsystem*> subsystemsToStop;
	if ( m_CurrentGameMode != m_NextGameMode )
	{
		if ( m_CurrentGameMode.IsNetworked && !m_NextGameMode.IsNetworked ) // TODODB: Handle the network engine in SSNetworkController instead of exceptions in here
		{
			g_NetworkEngine.Stop();
			g_NetworkInfo.Reset();
		}

		// Get the subsystems that run in the current game mode but not in the next
		subsystemsToStop = NeatFunctions::GetVectorDiff( m_CurrentGameMode.Subsystems, m_NextGameMode.Subsystems );
	}
	else
		subsystemsToStop = m_CurrentGameMode.Subsystems;

	if ( m_CurrentGameMode.Type == GameModeType::MainMenu && m_NextGameMode.Type == GameModeType::MainMenu )
	{
		for ( int i = 0; i < subsystemsToStop.size(); ++i )
		{
			if ( subsystemsToStop[i] == &g_SSMusicManager || subsystemsToStop[i] == &g_SSSFXNotification || subsystemsToStop[i] == &g_SSAudio )
			{
				subsystemsToStop.erase( subsystemsToStop.begin() + 1 );
				--i;
			}
		}
	}

	g_SubsystemManager.StopSubsystems( subsystemsToStop );
	if ( m_CurrentGameMode.IsSimulation && !m_NextGameMode.IsSimulation )
	{
		g_GameData.Reset();
		g_PlayerData.Reset();
		g_GameSpeedController.SetGameStarted( false );
		g_Alliances.Clear();
		g_SSMusicManager.Reset();
	}

	if ( !m_CurrentGameMode.IsSimulation && m_NextGameMode.IsSimulation )
	{
		g_SSMusicManager.Reset();
	}

	if ( m_CurrentGameMode.IsSimulation ) // TODODB: Shouldn't this also check if the next mode is not simulated?
		g_GameSpeedController.Reset();
}

void GameModeSelector::StartMode( const GameMode& gameMode )
{
	if ( !m_CurrentGameMode.IsNetworked && m_NextGameMode.IsNetworked )
	{
		if ( !g_NetworkEngine.IsInitialized() )
		{
			if ( !g_NetworkEngine.Initialize() )
			{
				Logger::Log( "Failed to switch to game mode <" + gameMode.Name + "> since the network engine failed to initialize", "GameModeSelector", LogSeverity::ERROR_MSG );
				return;
			}
		}
		if ( !g_NetworkEngine.IsRunning() )
			g_NetworkEngine.Start();
	}

	if ( !m_CurrentGameMode.IsSimulation && m_NextGameMode.IsSimulation )
		g_GameSpeedController.SetGameStarted( true );

	if ( g_NetworkEngine.IsListening() && m_NextGameMode.IsNetworked && m_NextGameMode.IsSimulation )
		g_NetworkEngine.StopListening();

	rVector<Subsystem*> subsystemsToStart;
	if ( m_CurrentGameMode != m_NextGameMode )
		subsystemsToStart = NeatFunctions::GetVectorDiff( gameMode.Subsystems, m_CurrentGameMode.Subsystems ); // Start only the subsystems that exist in the upcoming game mode but not in the current
	else
		subsystemsToStart = m_NextGameMode.Subsystems;

	if ( m_CurrentGameMode.Type == GameModeType::MainMenu && m_NextGameMode.Type == GameModeType::MainMenu )
	{
		for ( int i = 0; i < subsystemsToStart.size(); ++i )
		{
			if ( subsystemsToStart[i] == &g_SSMusicManager || subsystemsToStart[i] == &g_SSSFXNotification || subsystemsToStart[i] == &g_SSAudio )
			{
				subsystemsToStart.erase( subsystemsToStart.begin() + 1 );
				--i;
			}
		}
	}

	m_CurrentGameMode = gameMode;

	if ( !m_CurrentGameMode.IsNetworked && m_CurrentGameMode.IsSimulation )
		g_Randomizer.Seed( static_cast<unsigned int>( SDL_GetPerformanceCounter() ) );
	
	switch ( m_NextGameMode.Type ) //TODODB: Remove obsolete special cases
	{
		case GameModeType::Unknown:
		{
			Logger::Log( "Attempted to switch to game mode UNKNOWN", "GameModeSelector", LogSeverity::WARNING_MSG );
		} break;

		case GameModeType::ConsoleOnly:
		{
			g_SubsystemManager.StartSubsystems( subsystemsToStart );
			g_GUI.OpenWindow( "ConsoleWindow" );
		} break;

		case GameModeType::SingleplayerAI:
		case GameModeType::Singleplayer:
		{
			g_PlayerData.SetPlayerID( 0 );
			g_SubsystemManager.StartSubsystems( subsystemsToStart );
		} break;

		case GameModeType::Multiplayer:
		{
			g_PlayerData.SetPlayerID( g_NetworkInfo.GetNetworkID() );
			g_SubsystemManager.StartSubsystems( subsystemsToStart );
		} break;

		case GameModeType::NetworkLobby:
		{
			g_SubsystemManager.StartSubsystems( subsystemsToStart );
		} break;

		default:
			g_SubsystemManager.StartSubsystems( subsystemsToStart );
			break;
	}
}

int GameModeSelector::ScriptRestart( IScriptEngine* scriptEngine )
{
	SwitchToGameMode( m_CurrentGameMode.Type );
	return 0; // Number of return values.
}

int GameModeSelector::ScriptPrintGameModeSubsystems( IScriptEngine* scriptEngine )
{
	rString gameModeName = scriptEngine->PopString();

	const GameMode& gameMode = GetGameModeFromString( gameModeName );

	int index = 0;
	for ( auto& subsystem : gameMode.Subsystems )
	{
		Logger::GetStream() << index++ << ": " << subsystem->GetName() << std::endl;
	}

	return 0; // Number of return values.
}

int GameModeSelector::ScriptSwitchGameMode(IScriptEngine* scriptEngine)
{
	rString gameModeName = scriptEngine->PopString();
	SwitchToGameMode(  GetGameModeFromString( gameModeName ).Type );
	
	return 0; // Number of return values.
}
