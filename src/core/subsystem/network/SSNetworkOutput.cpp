/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSNetworkOutput.h"
#include <network/PacketPump.h>
#include <messaging/Message.h>
#include <messaging/GameMessages.h>
#include "../utility/SSMail.h"
#include "../../utility/GameData.h"

SSNetworkOutput& SSNetworkOutput::GetInstance()
{
	static SSNetworkOutput instance;
	return instance;
}

void SSNetworkOutput::Startup()
{
	m_UserInterests = MessageTypes::USER_SIGNAL | MessageTypes::CHAT;
	m_SimInterests	= MessageTypes::ORDER_UNITS | MessageTypes::ORDER_INVOKE | MessageTypes::UPGRADE;
	g_SSMail.RegisterSubscriber( this );
}

void SSNetworkOutput::Shutdown()
{
	g_SSMail.UnregisterSubscriber( this );
}

void SSNetworkOutput::UpdateUserLayer( const float deltaTime )
{
	for ( int i = 0; i < m_UserMailbox.size(); ++i )
	{
		const Message* message = m_UserMailbox[i];
		if ( message->CreatedFromPacket && !g_NetworkInfo.AmIHost() )
			continue;

		switch ( message->Type )
		{
			case MessageTypes::USER_SIGNAL:
			{
				const UserSignalMessage* specificMessage = static_cast<const UserSignalMessage*>( message );
				switch ( specificMessage->SignalType )
				{
					case UserSignalType::PAUSE:
					{
						SendConditionally( *message );
					} break;

					default:
						break;
				}
			} break;

			case MessageTypes::CHAT:
			{
				SendConditionally( *message );
			} break;

			default:
				break;
		}
	}
}

void SSNetworkOutput::UpdateSimLayer( const float timeStep )
{
	for ( int i = 0; i < m_SimMailbox.size(); ++i )
	{
		const Message* message = m_SimMailbox[i];
		if ( message->CreatedFromPacket && !g_NetworkInfo.AmIHost() )
			continue;

		switch ( message->Type )
		{
			case MessageTypes::ORDER_UNITS: // TODODB: Remove the stupid copying
			{
				const OrderUnitsMessage* orderUnitsMessage = static_cast<const OrderUnitsMessage*>( message );
				SendConditionally( OrderUnitsMessage( g_GameData.GetFrameCount(), orderUnitsMessage->MissionType, orderUnitsMessage->TeamID, orderUnitsMessage->Squads, orderUnitsMessage->TargetLocation, orderUnitsMessage->TargetEntity,
					orderUnitsMessage->OverridingCommand ) );
			} break;

			case MessageTypes::ORDER_INVOKE:
			{
				const OrderInvokeMessage* orderInvokeMessage = static_cast<const OrderInvokeMessage*>( message );
				SendConditionally( OrderInvokeMessage( g_GameData.GetFrameCount(), orderInvokeMessage->Squads, orderInvokeMessage->EnumCommand, orderInvokeMessage->TeamID, orderInvokeMessage->GenericValue ) );
			} break;

			case MessageTypes::UPGRADE:
			{
				const UpgradeMessage* upgradeMessage = static_cast< const UpgradeMessage* >( message );
				SendConditionally( UpgradeMessage( g_GameData.GetFrameCount(), upgradeMessage->PlayerID, upgradeMessage->Squads, upgradeMessage->UpgradeType ) );
			} break;

			case MessageTypes::WRITE_FILE:
			{
				const WriteFileMessage* upgradeMessage = static_cast<const WriteFileMessage*>( message );
				SendConditionally( WriteFileMessage( upgradeMessage->FileTextStr, upgradeMessage->FilePathStr ) );
			} break;

			default:
				break;
		}
	}
}

void SSNetworkOutput::SendConditionally( const Message& packet )
{
	if ( g_NetworkInfo.AmIHost() )
		g_PacketPump.SendToAll( packet );
	else
		g_PacketPump.Send( packet );
}