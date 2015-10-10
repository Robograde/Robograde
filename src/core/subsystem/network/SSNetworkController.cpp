/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSNetworkController.h"
#include <functional>
#include <network/PacketPump.h>
#include <utility/Randomizer.h>
#include <network/NetworkEngine.h>
#include <utility/ConfigManager.h>
#include <utility/NeatFunctions.h>
#include "../menu/SSGameLobby.h"
#include "../gui/SSGUIInfo.h"
#include "../hashing/SSHashGenerator.h"
#include "../../input/GameMessages.h"
#include "../../utility/GlobalDefinitions.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../utility/GameModeSelector.h"

SSNetworkController& SSNetworkController::GetInstance()
{
	static SSNetworkController instance;
	return instance;
}

void SSNetworkController::Startup()
{
	g_NetworkEngine.SetMaxClientCount( MAX_PLAYERS - 1 );

	g_Script.Register( "ShareHostFile", [this] ( IScriptEngine* scriptEngine ) -> int
	{
		rString filePath = scriptEngine->PopString();
		ShareHostFile( filePath );

		return 0;
	} );

	ReadConfig();

	g_Script.Register( "KickPlayer", [this] ( IScriptEngine* scriptEngine ) -> int 
	{
		ScriptArg playerToKick = scriptEngine->Pop();
		if ( playerToKick.Type == ScriptPrimitive::Double )
			this->KickPlayerByID( static_cast< short >( playerToKick.asDouble ) );
		else if ( playerToKick.Type == ScriptPrimitive::String )
			this->KickPlayerByName( playerToKick.asString );
		else
			Logger::Log( "KickPlayer received parameter of unknown type from LUA", "SSNetworkController", LogSeverity::WARNING_MSG );

		return 0;
	} );

	// Register callbacks
	int callbackHandle;
	callbackHandle = g_NetworkEngine.RegisterCallback( NetworkCallbackEvent::IncomingConnection, std::bind( &SSNetworkController::EnqueueIncomingConnection, this, std::placeholders::_1 ) );
	if ( callbackHandle != -1 )
		m_NetworkCallbackHandles.push_back( callbackHandle );
	callbackHandle = g_NetworkEngine.RegisterCallback( NetworkCallbackEvent::Disconnection, std::bind( &SSNetworkController::EnqueueDisconnection, this, std::placeholders::_1 ) );
	if ( callbackHandle != -1 )
		m_NetworkCallbackHandles.push_back( callbackHandle );

	RegisterMessages();
}

void SSNetworkController::Shutdown()
{
	m_NetworkPlayers.clear();
	m_IncomingConnectionQueue.Clear();
	m_OutgoingConnectionQueue.Clear();
	m_DisconnectionQueue.Clear();

	for ( int i = 0; i < m_NetworkCallbackHandles.size(); ++i )
		g_NetworkEngine.UnRegisterCallback( m_NetworkCallbackHandles[i] );
}

void SSNetworkController::UpdateUserLayer( const float deltaTime )
{
	CheckCallbacks();
}

void SSNetworkController::UpdateSimLayer( const float timeStep )
{
	if ( g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Multiplayer || g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Editor )
	{
		if ( !g_NetworkInfo.AmIHost() && g_NetworkInfo.GetConnectedPlayerCount() == 0 ) // TODODB: Check if host has disconnected instead
		{
			g_SSGUIInfo.DisplayMessage( "Host disconnected" );
			g_GameModeSelector.SwitchToGameMode( GameModeType::MainMenu );
		}

		if ( g_NetworkInfo.AmIHost() && g_NetworkInfo.GetConnectedPlayerCount() > 0 )
			g_PacketPump.SendToAll( StepMessage( g_GameData.GetFrameCount(), g_SSHashGenerator.GetHash(), g_Randomizer.GetRandomizationCounter() ) );
	}
}

void SSNetworkController::Reset()
{
	m_NetworkPlayers.clear();
	m_IncomingConnectionQueue.Clear();
	m_OutgoingConnectionQueue.Clear();
	m_DisconnectionQueue.Clear();
	m_HostStep		= 0;
	m_FramesToRun	= 0;

	while ( !m_HostHashQueue.empty() )
		m_HostHashQueue.pop();
	while ( !m_HostRandomCountQueue.empty() )
		m_HostRandomCountQueue.pop();
}

bool SSNetworkController::MakeHost( unsigned short port )
{
	if ( g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::NetworkLobby )
	{
		if ( EnsureEngineReady() )
		{
			if ( !g_NetworkEngine.IsListening() )
			{
				if ( !g_NetworkEngine.StartListening( port == INVALID_PORT ? m_DefaultListeningPort : port ) )
				{
					Logger::Log( "Failed to start listener", "SSNetworkController", LogSeverity::ERROR_MSG );
					return false;
				}
			}
			else
			{
				Logger::Log( "This machine is already host", "SSNetworkController", LogSeverity::WARNING_MSG );
				return false;
			}

			int playerID = g_GameData.IsDedicatedServer() ? PLAYER_ID_INVALID : 0;
			g_NetworkInfo.SetNetworkID( playerID );
			g_NetworkInfo.SetHostID( playerID );
			g_PlayerData.SetPlayerID( playerID );

			if ( !g_GameData.IsDedicatedServer() )
			{
				NetworkPlayer localPlayer = NetworkPlayer( g_NetworkInfo.GetNetworkID(), g_NetworkInfo.GetNetworkID(), PlayerType::Player, g_PlayerData.GetPlayerName() );
				g_SSNetworkController.AddPlayer(localPlayer);
			}
			return true;
		}
		else
		{
			Logger::Log( "Failed to set local machine as host", "SSNetworkController", LogSeverity::ERROR_MSG );
			return false;
		}
	}
	else
		Logger::Log( "Hosting is not allowed in the current game mode", "SSNetworkController", LogSeverity::WARNING_MSG );
	return false;
}

bool SSNetworkController::ConnectToGame( const rString& address, unsigned short port )
{
	bool result;
	if ( EnsureEngineReady() )
	{
		g_NetworkEngine.RequestConnection( address, port );
		result = true;
	}
	else
		result = false;

	return result;
}

void SSNetworkController::AddPlayer( const NetworkPlayer& newPlayer )
{
	if ( m_NetworkPlayers.find( newPlayer.PlayerID ) == m_NetworkPlayers.end() )
		m_NetworkPlayers.emplace( newPlayer.PlayerID, newPlayer );
	else
		Logger::Log( "Attempted to add already existing network player (ID = " + rToString( newPlayer.PlayerID ) + ")", "SSNetworkController", LogSeverity::WARNING_MSG );
}

void SSNetworkController::RemovePlayer( short playerID )
{
	auto playerToRemove = m_NetworkPlayers.find( playerID );
	if ( playerToRemove != m_NetworkPlayers.end() )
	{
		if (g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Multiplayer)
			g_SSGUIInfo.DisplayMessage( m_NetworkPlayers.at( playerID ).Name + " has disconnected." );
		m_NetworkPlayers.erase( playerID );
	}
	else
		Logger::Log( "Attempted to remove non existent player (ID = " + rToString( playerID ) + ")", "SSNetworkController", LogSeverity::WARNING_MSG );
}

bool SSNetworkController::ShareHostFile( const rString& filePath )
{
	if ( g_NetworkInfo.AmIHost() )
	{
		rString fileText = FileUtility::GetFileContentAsString( filePath );

		if ( fileText == "" )
		{
			Logger::Log( "ShareHostFile read an empty file (Path = " + filePath + " )", "SSGameLobby", LogSeverity::WARNING_MSG );
			return false;
		}

		g_PacketPump.SendToAll( WriteFileMessage( fileText, filePath ) );
	}
	return true;
}

rMap<short, NetworkPlayer>& SSNetworkController::GetNetworkedPlayers()
{
	return m_NetworkPlayers;
}

const rString& SSNetworkController::GetPlayerName( short playerID ) const
{
	auto networkedPlayer = m_NetworkPlayers.find( playerID );
	if ( networkedPlayer != m_NetworkPlayers.end() )
		return networkedPlayer->second.Name;
	return INVALID_PLAYER_NAME;
}

unsigned short SSNetworkController::GetDefaultListeningPort() const
{
	return m_DefaultListeningPort;
}

unsigned int SSNetworkController::GetHostStep() const
{
	return m_HostStep;
}

void SSNetworkController::SetPlayerName( short playerID, const rString& newPlayerName )
{
	if ( m_NetworkPlayers.find( playerID ) != m_NetworkPlayers.end() )
		m_NetworkPlayers.at( playerID ).Name = newPlayerName;
	else
		Logger::Log( "Attempted to set name of non existent player (ID = " + rToString( playerID ) + ")", "SSNetworkController", LogSeverity::WARNING_MSG );
}

void SSNetworkController::SetPlayerFinishedLoading( short playerID )
{
	if ( m_NetworkPlayers.find( playerID ) != m_NetworkPlayers.end() )
		m_NetworkPlayers.at( playerID ).HasFinishedLoading = true;
	else
		Logger::Log( "Attempted to set loading finished of non existent player (ID = " + rToString( playerID ) + ")", "SSNetworkController", LogSeverity::WARNING_MSG );
}

void SSNetworkController::SetHostStep( unsigned int hostStep )
{
	m_HostStep = hostStep;
}

bool SSNetworkController::DoesPlayerExist( short playerID ) const
{
	return m_NetworkPlayers.find( playerID ) != m_NetworkPlayers.end();
}

bool SSNetworkController::AllFinishedLoading() const
{
	bool toReturn = true;
	for ( auto networkPlayer : m_NetworkPlayers )
	{
		if ( !networkPlayer.second.HasFinishedLoading )
		{
			toReturn = false;
			break;
		}
	}
	return toReturn;
}

void SSNetworkController::IncrementFramesToRun()
{
	++m_FramesToRun;
}

bool SSNetworkController::DecrementFramesToRun()
{
	if ( m_FramesToRun > 0 )
	{
		--m_FramesToRun;
		return true;
	}
	return false;
}

void SSNetworkController::PushHostHash( unsigned int hash )
{
	m_HostHashQueue.push( hash );
}

unsigned int SSNetworkController::PopHostHash( )
{
	unsigned int toReturn;
	if ( m_HostHashQueue.empty() )
	{
		Logger::Log( "Attempted to pop hash from host but there were none left. Are we out of sync?", "NetworkInfo", LogSeverity::WARNING_MSG );
		return 0;
	}
	toReturn = m_HostHashQueue.front();
	m_HostHashQueue.pop();
	return toReturn;
}

void SSNetworkController::PushHostRandomCount( unsigned int randomCount )
{
	m_HostRandomCountQueue.push( randomCount );
}

unsigned int SSNetworkController::PopHostRandomCount( )
{
	unsigned int toReturn;
	if ( m_HostRandomCountQueue.empty() )
	{
		Logger::Log( "Attempted to pop randomCount from host but there were none left. Are we out of sync?", "NetworkInfo", LogSeverity::WARNING_MSG );
		return 0;
	}
	toReturn = m_HostRandomCountQueue.front();
	m_HostRandomCountQueue.pop();
	return toReturn;
}

void SSNetworkController::CheckCallbacks()
{
	short networkID;
	while ( m_IncomingConnectionQueue.Consume( networkID ) )
	{
		if ( g_NetworkInfo.AmIHost() )
		{
			PlayerType playerType;

			short playerID = FindUnusedPlayerID();
			playerID != PLAYER_ID_INVALID ? playerType = PlayerType::Player : playerType = PlayerType::Observer;

			rVector<short>		playerIDs;
			rVector<rString>	names;
			for ( auto& networkPlayer : m_NetworkPlayers )
			{
				playerIDs.push_back( networkPlayer.first );
				names.push_back( networkPlayer.second.Name );
			}

			short hostID = g_NetworkInfo.GetNetworkID();
			NetworkPlayer hostPlayer = m_NetworkPlayers.at( hostID );
			g_PacketPump.Send( PlayerTypeMessage( static_cast<short>( hostPlayer.Type ), hostPlayer.PlayerID, hostID ), networkID );
			g_PacketPump.Send( PlayerTypeMessage( static_cast<short>( playerType ), playerID, networkID ), networkID );
			g_PacketPump.Send( NameUpdateMessage( names, playerIDs ), networkID );
			g_PacketPump.Send( RandomSeedMessage( g_Randomizer.GetSeed() ), networkID );

			if ( !g_SSGameLobby.IsEditorLobby() )
			{
				ShareHostFile( BESTWEIGHTS_FILE_PATH );
				ShareHostFile( GENES0_FILE_PATH );
				ShareHostFile( GENES1_FILE_PATH );
				ShareHostFile( GENES2_FILE_PATH );
				ShareHostFile( GENES3_FILE_PATH );
			}

			g_SSGameLobby.OnNewClientConnect( networkID );

			NetworkPlayer newPlayer = NetworkPlayer( playerID, networkID, playerType );
			m_NetworkPlayers.emplace( networkID, newPlayer );

			for ( auto& networkPlayer : m_NetworkPlayers )
			{
				if ( g_SSGameLobby.IsPlayerReady( networkPlayer.first ) )
					g_PacketPump.Send( UserSignalMessage( UserSignalType::READY_TOGGLE, networkPlayer.first ), networkID );
			}
		}
		else
			Logger::Log( "Received incoming connection as client", "NetworkController", LogSeverity::WARNING_MSG );
	}

	while ( m_DisconnectionQueue.Consume( networkID ) )
	{
		short foundID = -1;
		for ( auto networkPlayer : m_NetworkPlayers )
		{
			if ( networkPlayer.second.NetworkID == networkID )
			{
				foundID = networkPlayer.first;
				break;
			}
		} 

		if ( foundID >= 0 )
		{
			if ( g_NetworkInfo.AmIHost() )
			{
				for ( auto networkPlayer : m_NetworkPlayers )
				{
					if ( networkPlayer.second.NetworkID != g_NetworkInfo.GetHostID() )
						g_PacketPump.Send( ConnectionStatusUpdateMessage( m_NetworkPlayers.at(foundID).PlayerID, ConnectionStatusUpdates::DISCONNECTED ), networkPlayer.second.NetworkID );
				}
			}

			RemovePlayer( foundID );
		}
	}
}

bool SSNetworkController::ReadConfig()
{
	CallbackConfig* networkConfig = g_ConfigManager.GetConfig( "network.cfg" );
	if ( networkConfig == nullptr )
		return false;

	m_DefaultListeningPort = static_cast< unsigned short >( networkConfig->GetInt( "DefaultListeningPort", 40606 ) );
	g_PlayerData.SetPlayerName( networkConfig->GetString( "PlayerName", "ANON", "The player's in-game nickname" ) );

	m_HasReadConfig = true;
	return true;
}

bool SSNetworkController::EnsureEngineReady()
{
	if ( !m_HasReadConfig )
	{
		if ( !ReadConfig() )
		{
			Logger::Log( "Failed to read network config", "SSNetworkController", LogSeverity::ERROR_MSG );
			return false;
		}
	}

	if ( !g_NetworkEngine.IsRunning() )
		g_NetworkEngine.Start();

	return true;
}

void SSNetworkController::KickPlayerByName( const rString& playerName )
{
	if ( g_NetworkInfo.AmIHost() )
	{
		for ( auto& networkPlayer : m_NetworkPlayers )
		{
			if ( networkPlayer.second.Name == playerName )
				g_NetworkEngine.RequestDisconnection( networkPlayer.second.NetworkID );
		}
	}
}

void SSNetworkController::KickPlayerByID( short playerID )
{
	g_NetworkEngine.RequestDisconnection( playerID );
}

void SSNetworkController::EnqueueIncomingConnection( const void* const connectionData )
{
	const ConnectionData* const connection = reinterpret_cast<const ConnectionData* const>( connectionData );
	m_IncomingConnectionQueue.Produce( connection->NetworkID );
}

void SSNetworkController::EnqueueOutgoingConnection( const void* const connectionData )
{
	const ConnectionData* const connection = reinterpret_cast<const ConnectionData* const>( connectionData );
	m_OutgoingConnectionQueue.Produce( connection->NetworkID );
}

void SSNetworkController::EnqueueDisconnection( const void* const connectionData )
{
	const ConnectionData* const connection = reinterpret_cast<const ConnectionData* const>( connectionData );
	m_DisconnectionQueue.Produce( connection->NetworkID );
}

int	SSNetworkController::FindUnusedPlayerID() const
{
	int toReturn = PLAYER_ID_INVALID;

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		bool playerIDUsed = false;
		for ( auto networkPlayer : m_NetworkPlayers )
		{
			if ( networkPlayer.second.PlayerID == i )
			{
				playerIDUsed = true;
				break;
			}
		}

		if ( !playerIDUsed )
			return i;
	}

	return PLAYER_ID_INVALID;
}

void SSNetworkController::RegisterMessages()
{
	g_NetworkEngine.RegisterMessage( pNew( OrderUnitsMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( OrderInvokeMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( UpgradeMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( RandomSeedMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( UserSignalMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( StepMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( StepResponseMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( NameUpdateMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( WriteFileMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( ChatMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( ConnectionStatusUpdateMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( LevelSelectionChangeMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( ColourChangeMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( TeamChangeMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( SpawnPointChangeMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( SpawnCountChangeMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( ReserveAIMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( UserPingMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( PlayerTypeMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( PlacePropMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( PlaceResourceMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( PlaceControlPointMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( MoveObjectMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( EditorSFXEmitterMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( UpdateGhostEntityPositionMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( UpdateGhostEntityVisibilityMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( UpdateGhostEntityModelMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( SelectEntityMessage ) );
	g_NetworkEngine.RegisterMessage( pNew( EditorCameraPathsMessage ) );
	g_NetworkEngine.RegisterMessage( pNew(EditorParticleEmitterMessage));
	g_NetworkEngine.RegisterMessage( pNew(EditorTerrainBrushMessage));
}