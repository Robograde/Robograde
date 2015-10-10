/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSNetworkInput.h"
#include <network/NetworkEngine.h>
#include <network/PacketPump.h>
#include <utility/Randomizer.h>
#include "SSNetworkController.h"
#include "../utility/SSMail.h"
#include "../menu/SSGameLobby.h"
#include "../menu/SSInGameMenu.h"
#include "../../input/GameMessages.h"
#include "../../utility/GameModeSelector.h"
#include "../../utility/PlayerData.h"
#include "../../utility/GameData.h"
#include "../../utility/GameSpeedController.h"

SSNetworkInput& SSNetworkInput::GetInstance( )
{
	static SSNetworkInput instance;
	return instance;
}

void SSNetworkInput::UpdateUserLayer( float deltaTime )
{
	FetchMessages(); // Must be called in both user and simulation in case one of them is disabled

	Message* packet = nullptr;
	while ( !m_UserMessages.empty() )
	{
		packet = m_UserMessages.front();
		m_UserMessages.pop();

		switch ( packet->Type )
		{
			case MessageTypes::USER_SIGNAL:
			{
				UserSignalMessage* signalPacket = static_cast<UserSignalMessage*>( packet );

				switch ( signalPacket->SignalType )
				{
					case UserSignalType::START_GAME:
					{
						g_SSGameLobby.SwitchToMultiplayer( );
					} break;

					case UserSignalType::READY_TOGGLE:
					{
						g_SSGameLobby.ToggleReadyStatus( signalPacket->SenderID );

						if ( g_NetworkInfo.AmIHost() )
							g_PacketPump.SendToAll( *signalPacket );
					} break;

					case UserSignalType::PAUSE:
					{
						ForwardToUserLayer( *packet );
					} break;

					case UserSignalType::FINISHED_LOADING:
					{
						g_SSNetworkController.SetPlayerFinishedLoading( signalPacket->SenderID );

						if ( g_NetworkInfo.AmIHost() )
							g_PacketPump.SendToAll( *signalPacket );
					} break;

					case UserSignalType::SET_EDITOR_LOBBY:
					{
						g_SSGameLobby.SetEditorMode( true );
					} break;

					case UserSignalType::SET_MULTIPLAYER_LOBBY:
					{
						g_SSGameLobby.SetEditorMode( false );
					} break;

					default:
					{
						Logger::Log( "Received signal packet with unknown signal", "SSNetworkInput", LogSeverity::WARNING_MSG );
					} break;
				}
			} break;

			case MessageTypes::STEP:
			{
				if ( !g_NetworkInfo.AmIHost() )
				{
					StepMessage* stepPacket = static_cast<StepMessage*>( packet ); // TODODB: Maybe move some variables out of NetworkInfo now?
					g_SSNetworkController.SetHostStep( stepPacket->Frame );
					g_SSNetworkController.PushHostHash( stepPacket->Hash );
					g_SSNetworkController.PushHostRandomCount( stepPacket->RandomCount );
					g_SSNetworkController.IncrementFramesToRun();

					g_PacketPump.Send( StepResponseMessage( g_PlayerData.GetPlayerID(), g_GameData.GetFrameCount() ) );
				}
				else
					Logger::Log( "Received step message as host", "SSNetworkInput", LogSeverity::WARNING_MSG );
			} break;

			case MessageTypes::STEP_RESPONSE:
			{
				if ( g_NetworkInfo.AmIHost() )
				{
					StepResponseMessage* stepResponsePacket = static_cast<StepResponseMessage*>( packet );
					g_SSNetworkController.GetNetworkedPlayers()[stepResponsePacket->PlayerID].FrameCounter = stepResponsePacket->Frame;
				}
				else
					Logger::Log( "Received stepResponse as client", "SSNetworkInput", LogSeverity::WARNING_MSG );
			} break;

			case MessageTypes::NAME_UPDATE:
			{
				NameUpdateMessage* nameUpdateMessage = static_cast<NameUpdateMessage*>( packet );

				for ( int i = 0; i < nameUpdateMessage->PlayerIDs.size(); ++i ) // TODODB: Either make this message a player register message or make the system wih separate sending work properly without racing
				{
					if ( !g_SSNetworkController.DoesPlayerExist( nameUpdateMessage->PlayerIDs[i] ) )
						g_SSNetworkController.AddPlayer( NetworkPlayer( nameUpdateMessage->PlayerIDs[i], nameUpdateMessage->PlayerIDs[i] ) );
				}

				for ( int i = 0; i < nameUpdateMessage->Names.size(); ++i )
					g_SSNetworkController.SetPlayerName( nameUpdateMessage->PlayerIDs[i], nameUpdateMessage->Names[i] );

				if ( g_NetworkInfo.AmIHost() )
					g_PacketPump.SendToAll( *nameUpdateMessage );
			} break;

			case MessageTypes::WRITE_FILE:
			{
				const WriteFileMessage* msg = static_cast<const WriteFileMessage*>(packet);
				FileUtility::WriteToFile( msg->FileTextStr.c_str(), msg->FileTextStr.size(), msg->FilePathStr );
			} break;

			case MessageTypes::CHAT:
			{
				packet->CreatedFromPacket = true;
				g_SSMail.PushToUserLayer( *packet );
			} break;

			case MessageTypes::CONNECTION_STATUS:
			{
				const ConnectionStatusUpdateMessage* connectionStatusMessage = static_cast<const ConnectionStatusUpdateMessage*>( packet );
				if ( connectionStatusMessage->ConnectionStatus == ConnectionStatusUpdates::CONNECTED )
					g_SSNetworkController.AddPlayer( NetworkPlayer( connectionStatusMessage->NetworkID, connectionStatusMessage->NetworkID ) );
				else if ( connectionStatusMessage->ConnectionStatus == ConnectionStatusUpdates::DISCONNECTED )
					g_SSNetworkController.RemovePlayer( connectionStatusMessage->NetworkID );
				else
					Logger::Log( "Received Connection status update message with unknown status change", "SSNetworkInput", LogSeverity::WARNING_MSG );
				
			} break;

			case MessageTypes::RANDOM_SEED:
			{
				RandomSeedMessage* seedPacket = static_cast<RandomSeedMessage*>( packet );
				g_Randomizer.Seed( seedPacket->Seed );
			} break;
			
			case MessageTypes::LEVEL_SELECTION_CHANGE:
			{
				LevelSelectionChangeMessage* levelSelectionChangeMessage = static_cast<LevelSelectionChangeMessage*>( packet );
				g_SSGameLobby.SetLevel( levelSelectionChangeMessage->LevelName );
			} break;

			case MessageTypes::PLAYER_TYPE:
			{
				PlayerTypeMessage* playerTypeMessage = static_cast<PlayerTypeMessage*>( packet );
				g_SSNetworkController.AddPlayer( NetworkPlayer( playerTypeMessage->PlayerID, playerTypeMessage->NetworkID, static_cast<PlayerType>( playerTypeMessage->PlayerType ) ) );

				rVector<short> playerIDs;
				rVector<rString> names;
				g_PlayerData.SetPlayerID( playerTypeMessage->NetworkID );
				playerIDs.push_back( playerTypeMessage->NetworkID );
				names.push_back( g_PlayerData.GetPlayerName() );
				g_PacketPump.Send( NameUpdateMessage( names, playerIDs ), g_NetworkInfo.GetHostID() );

			} break;

			case MessageTypes::RESERVE_AI_PLAYER:
			case MessageTypes::CHANGE_SPAWN_COUNT:
			case MessageTypes::SPAWN_POINT_CHANGE:
			case MessageTypes::TEAM_CHANGE:
			case MessageTypes::COLOUR_CHANGE:
			case MessageTypes::USER_PING_MESSAGE:
			case MessageTypes::UPDATE_GHOST_ENTITY_POS:
			case MessageTypes::SELECT_ENTITY:
			{
				ForwardToUserLayer( *packet );
			} break;

			default:
				Logger::Log( "Received packet with unrecognized type <Type = " + rToString( packet->Type ) + ">", "SSNetworkInput", LogSeverity::WARNING_MSG );
				break;
		}
		tDelete( packet );
	}
}

void SSNetworkInput::UpdateSimLayer( const float timeStep )
{
	FetchMessages(); // Must be called in both user and simulation in case one of them is disabled

	Message* packet = nullptr;
	while ( !m_SimMessages.empty() )
	{
		packet = m_SimMessages.front();
		m_SimMessages.pop();

		switch ( packet->Type )
		{
			case MessageTypes::ORDER_UNITS:
			{
				OrderUnitsMessage* orderPacket = static_cast<OrderUnitsMessage*>( packet );
				ForwardToSimLayer( *orderPacket, orderPacket->ExecutionFrame );
			} break;

			case MessageTypes::ORDER_INVOKE:
			{
				OrderInvokeMessage* orderInvokePacket = static_cast<OrderInvokeMessage*>( packet );
				ForwardToSimLayer( *orderInvokePacket, orderInvokePacket->ExecutionFrame );
			} break;

			case MessageTypes::UPGRADE:
			{
				UpgradeMessage* upgradePacket = static_cast<UpgradeMessage*>( packet );
				ForwardToSimLayer( *upgradePacket, upgradePacket->ExecutionFrame );
			} break;

			case MessageTypes::PLACE_PROP:
			{
				PlacePropMessage* propMessage = static_cast<PlacePropMessage*>( packet );
				ForwardToSimLayer( *propMessage, propMessage->ExecutionFrame );
			} break;

			case MessageTypes::PLACE_RESOUCE:
			{
				PlaceResourceMessage* resouceMessage = static_cast<PlaceResourceMessage*>( packet );
				ForwardToSimLayer( *resouceMessage, resouceMessage->ExecutionFrame );
			} break;

			case MessageTypes::PLACE_CONTROL_POINT:
			{
				PlaceControlPointMessage* controlPointMessage = static_cast<PlaceControlPointMessage*>( packet );
				ForwardToSimLayer( *controlPointMessage, controlPointMessage->ExecutionFrame );
			} break;

			case MessageTypes::MOVE_OBJECT:
			{
				MoveObjectMessage* moveObjectMessage = static_cast<MoveObjectMessage*>( packet );
				ForwardToSimLayer( *moveObjectMessage, moveObjectMessage->ExecutionFrame );
			} break;

			case MessageTypes::EDITOR_SFXEMITTER:
			{
				EditorSFXEmitterMessage* sfxEmitterMessage = static_cast<EditorSFXEmitterMessage*>( packet );
				ForwardToSimLayer( *sfxEmitterMessage, sfxEmitterMessage->ExecutionFrame );
			} break;

			case MessageTypes::UPDATE_GHOST_ENTITY_VISIBLILITY:
			{
				UpdateGhostEntityVisibilityMessage* updateGhostVisibilityMessage = static_cast<UpdateGhostEntityVisibilityMessage*>( packet );
				ForwardToSimLayer( *updateGhostVisibilityMessage, updateGhostVisibilityMessage->ExecutionFrame );
			} break;

			case MessageTypes::UPDATE_GHOST_ENTITY_MODEL:
			{
				UpdateGhostEntityModelMessage* updateGhostModelMessage = static_cast<UpdateGhostEntityModelMessage*>( packet );
				ForwardToSimLayer( *updateGhostModelMessage, updateGhostModelMessage->Executionframe );
			} break;
			
			case MessageTypes::EDITOR_CAMERA_PATHS:
			{
				EditorCameraPathsMessage* cameraPathsMessage = static_cast<EditorCameraPathsMessage*>( packet );
				ForwardToSimLayer( *cameraPathsMessage, cameraPathsMessage->ExecutionFrame );
			} break;

			case MessageTypes::EDITOR_PARTICLE_EMITTER:
			{
				EditorParticleEmitterMessage* emitterMessage = static_cast<EditorParticleEmitterMessage*>( packet );
				ForwardToSimLayer( *emitterMessage, emitterMessage->ExecutionFrame );
			} break;

			case MessageTypes::EDITOR_TERRAIN_BRUSH:
			{
				EditorTerrainBrushMessage* brushMessage = static_cast<EditorTerrainBrushMessage*>(packet);
				ForwardToSimLayer(*brushMessage, brushMessage->ExecutionFrame);
			} break;

			default:
				Logger::Log( "Received packet of unknown type", "SSNetworkInput", LogSeverity::WARNING_MSG );
			break;
		}
		tDelete( packet );
	}
}

void SSNetworkInput::FetchMessages()
{
	Message* packet = nullptr;
	while ( g_NetworkEngine.GetPacket( packet ) )
		packet->IsSimulation ? m_SimMessages.push( packet ) : m_UserMessages.push( packet );
}

void SSNetworkInput::ForwardToUserLayer( Message& message ) const
{
	message.CreatedFromPacket = true;
	g_SSMail.PushToUserLayer( message );
}

void SSNetworkInput::ForwardToSimLayer( Message& message ) const
{
	message.CreatedFromPacket = true;
	g_SSMail.PushToCurrentFrame( message );
}

void SSNetworkInput::ForwardToSimLayer( Message& message, unsigned int executionFrame ) const
{
	message.CreatedFromPacket = true;
	if ( g_NetworkInfo.AmIHost() )
		g_SSMail.PushToCurrentFrame( message );
	else
		g_SSMail.PushToFrame( message, executionFrame );
}