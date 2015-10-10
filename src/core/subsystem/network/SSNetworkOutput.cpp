/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSNetworkOutput.h"
#include <network/PacketPump.h>
#include <messaging/Message.h>
#include "../utility/SSMail.h"
#include "../../input/GameMessages.h"
#include "../../utility/GameData.h"

SSNetworkOutput& SSNetworkOutput::GetInstance()
{
	static SSNetworkOutput instance;
	return instance;
}

void SSNetworkOutput::Startup()
{
	m_UserInterests = MessageTypes::USER_SIGNAL | MessageTypes::CHAT | MessageTypes::UPDATE_GHOST_ENTITY_POS | MessageTypes::SELECT_ENTITY;
	m_SimInterests	= MessageTypes::ORDER_UNITS | MessageTypes::ORDER_INVOKE | MessageTypes::UPGRADE |MessageTypes::PLACE_PROP | MessageTypes::PLACE_RESOUCE | MessageTypes::PLACE_CONTROL_POINT |
		MessageTypes::MOVE_OBJECT | MessageTypes::EDITOR_SFXEMITTER | MessageTypes::UPDATE_GHOST_ENTITY_VISIBLILITY | MessageTypes::UPDATE_GHOST_ENTITY_MODEL | MessageTypes::EDITOR_PARTICLE_EMITTER | MessageTypes::EDITOR_TERRAIN_BRUSH;

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
			case MessageTypes::UPDATE_GHOST_ENTITY_POS:
			case MessageTypes::SELECT_ENTITY:
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

			case MessageTypes::PLACE_PROP:
			{
				const PlacePropMessage* propMessage = static_cast<const PlacePropMessage*>( message );
				SendConditionally( PlacePropMessage( g_GameData.GetFrameCount(), propMessage->PosX, propMessage->PosZ, propMessage->Scale, propMessage->Orientation, propMessage->BlockPath, propMessage->ModelPath, propMessage->Radius, propMessage->IsTree ) );
			} break;

			case MessageTypes::PLACE_RESOUCE:
			{
				const PlaceResourceMessage* resouceMessage = static_cast<const PlaceResourceMessage*>( message );
				SendConditionally( PlaceResourceMessage( g_GameData.GetFrameCount(), resouceMessage->PosX, resouceMessage->PosZ, resouceMessage->Scale, resouceMessage->Orientation, resouceMessage->ModelFileName ) );
			} break;

			case MessageTypes::PLACE_CONTROL_POINT:
			{
				const PlaceControlPointMessage* controlPointMessage = static_cast<const PlaceControlPointMessage*>( message );
				SendConditionally( PlaceControlPointMessage( g_GameData.GetFrameCount(), controlPointMessage->PosX, controlPointMessage->PosZ, controlPointMessage->Scale, controlPointMessage->Orientation, controlPointMessage->ModelFileName ) );
			} break;

			case MessageTypes::MOVE_OBJECT:
			{
				const MoveObjectMessage* moveObjectMessage = static_cast<const MoveObjectMessage*>( message );
				SendConditionally( MoveObjectMessage( g_GameData.GetFrameCount(), moveObjectMessage->EntityToMove, moveObjectMessage->NewPosX, moveObjectMessage->NewPosZ ) );
			} break;

			case MessageTypes::EDITOR_SFXEMITTER:
			{
				const EditorSFXEmitterMessage* sfxEmitterMessage = static_cast<const EditorSFXEmitterMessage*>( message );
				SendConditionally( EditorSFXEmitterMessage( g_GameData.GetFrameCount(), sfxEmitterMessage->EntityID, sfxEmitterMessage->PlayerId, sfxEmitterMessage->Type, sfxEmitterMessage->Pos, sfxEmitterMessage->Name, sfxEmitterMessage->Path,
					sfxEmitterMessage->TimeInterval, sfxEmitterMessage->DistanceMin, sfxEmitterMessage->DistanceMax ) );
			} break;

			case MessageTypes::UPDATE_GHOST_ENTITY_VISIBLILITY:
			{
				const UpdateGhostEntityVisibilityMessage* ghostEntityVisibilityMessage = static_cast<const UpdateGhostEntityVisibilityMessage*>( message );
				SendConditionally( UpdateGhostEntityVisibilityMessage( g_GameData.GetFrameCount(), ghostEntityVisibilityMessage->OwnerPlayerID, ghostEntityVisibilityMessage->Isvisible ) );
			} break;

			case MessageTypes::UPDATE_GHOST_ENTITY_MODEL:
			{
				const UpdateGhostEntityModelMessage* ghostEntityModelMessage = static_cast<const UpdateGhostEntityModelMessage*>( message );
				SendConditionally( UpdateGhostEntityModelMessage( g_GameData.GetFrameCount(), ghostEntityModelMessage->OwnerPlayerID, ghostEntityModelMessage->ModelName ) );
			} break;
			
			case MessageTypes::EDITOR_CAMERA_PATHS:
			{
				const EditorCameraPathsMessage* cameraPathsMessage = static_cast<const EditorCameraPathsMessage*>( message );
				Message* copy = cameraPathsMessage->Clone();
				static_cast< EditorCameraPathsMessage* >( copy )->ExecutionFrame = g_GameData.GetFrameCount();
				SendConditionally( *copy );
				tDelete( copy );
			} break;

			case MessageTypes::EDITOR_PARTICLE_EMITTER:
			{
				const EditorParticleEmitterMessage* emitterMessage = static_cast<const EditorParticleEmitterMessage*>( message );
				SendConditionally( EditorParticleEmitterMessage( g_GameData.GetFrameCount(), emitterMessage->EntityID, emitterMessage->PlayerId, emitterMessage->Type, emitterMessage->ParticleType, emitterMessage->Pos, emitterMessage->Col, emitterMessage->Name) );
			} break;

			case MessageTypes::EDITOR_TERRAIN_BRUSH:
			{
				const EditorTerrainBrushMessage* brushMessage = static_cast<const EditorTerrainBrushMessage*>(message);
				SendConditionally(EditorTerrainBrushMessage(g_GameData.GetFrameCount(), brushMessage->Center, brushMessage->Strength, brushMessage->Hardness, brushMessage->Radius));
			} break;

			default:
				break;
		}
	}
}

void SSNetworkOutput::SendConditionally( const Message& packet )
{
	if ( g_NetworkInfo.GetConnectedPlayerCount() > 0 )
	{
		if ( g_NetworkInfo.AmIHost() )
			g_PacketPump.SendToAll( packet );
		else
			g_PacketPump.Send( packet );
	}
}