/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSNetworkInput.h"
#include <network/NetworkEngine.h>
#include <network/PacketPump.h>
#include <utility/Randomizer.h>
#include <messaging/GameMessages.h>
#include "../../utility/GameModeSelector.h"
#include "../../utility/PlayerData.h"
#include "../../utility/GameData.h"
#include "../../utility/GameSpeedController.h"
#include "../utility/SSMail.h"
#include "../menu/SSGameLobby.h"
#include "../menu/SSInGameMenu.h"
#include "SSNetworkController.h"

SSNetworkInput& SSNetworkInput::GetInstance( )
{
	static SSNetworkInput instance;
	return instance;
}

void SSNetworkInput::UpdateUserLayer( float deltaTime )
{
	Message* packet = nullptr;
	while ( g_NetworkEngine.GetUserPacket( packet ) )
	{
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
					g_NetworkInfo.SetHostStep( stepPacket->Frame );
					g_NetworkInfo.PushHostHash( stepPacket->Hash );
					g_NetworkInfo.PushHostRandomCount( stepPacket->RandomCount );
					g_NetworkInfo.IncrementFramesToRun();

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

			case MessageTypes::RESERVE_AI_PLAYER:
			case MessageTypes::CHANGE_SPAWN_COUNT:
			case MessageTypes::SPAWN_POINT_CHANGE:
			case MessageTypes::TEAM_CHANGE:
			case MessageTypes::COLOUR_CHANGE:
			case MessageTypes::USER_PING_MESSAGE:
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
	Message* packet = nullptr;
	while ( g_NetworkEngine.GetSimulationPacket( packet ) )
	{
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

			default:
				Logger::Log( "Received packet of unknown type", "SSNetworkInput", LogSeverity::WARNING_MSG );
			break;
		}
		tDelete( packet );
	}
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
