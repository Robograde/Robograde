#include "SSEditorToolbox.h"
#include <input/Input.h>
#include <collisionDetection/ICollisionDetection.h>
#include <utility/Logger.h>
#include <utility/Colours.h>
#include <gfx/ModelBank.h>
#include "../gamelogic/SSCamera.h"
#include "../../picking/PickingType.h"
#include "../../camera/Camera.h"
#include "../../EntityFactory.h"
#include "../../datadriven/EntityManager.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/PlacementComponent.h"
#include "../../component/ModelComponent.h"
#include "../../component/CollisionComponent.h"
#include "../../component/ColourComponent.h"
#include "../../input/GameMessages.h"
#include "../../utility/GameData.h"
#include "../utility/SSMail.h"
#include "../network/SSNetworkController.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::mat4;

SSEditorToolbox& SSEditorToolbox::GetInstance()
{
	static SSEditorToolbox instance;
	return instance;
}

void SSEditorToolbox::Startup()
{
	g_SSMail.RegisterSubscriber( this );
	m_UserInterests = MessageTypes::UPDATE_GHOST_ENTITY_POS | MessageTypes::SELECT_ENTITY;
	m_SimInterests	= MessageTypes::MOVE_OBJECT | MessageTypes::UPDATE_GHOST_ENTITY_VISIBLILITY | MessageTypes::UPDATE_GHOST_ENTITY_MODEL;

	// Create placement ghost entities and create entries for selected entities
	const rMap<short, NetworkPlayer>& networkedPlayers = g_SSNetworkController.GetNetworkedPlayers();
	for ( auto networkedPlayer : networkedPlayers )
	{
		// Create placement ghost entities
		Entity ghostPlacementEntity = EntityFactory::CreatePlacementGhostObject();
		ModelComponent* modelCompoent = GetDenseComponent<ModelComponent>( ghostPlacementEntity );
		modelCompoent->ModelHandle = gfx::g_ModelBank.LoadModel( "OldTVShape.robo" );
		modelCompoent->Min = gfx::g_ModelBank.FetchModel( modelCompoent->ModelHandle ).Min;
		modelCompoent->Max = gfx::g_ModelBank.FetchModel( modelCompoent->ModelHandle ).Max;

		m_PlacementGhostEntities.emplace( networkedPlayer.first, ghostPlacementEntity );
		if ( networkedPlayer.first != g_PlayerData.GetPlayerID() )
		{
			ColourComponent* colourComponent = GetDenseComponent<ColourComponent>( m_PlacementGhostEntities.at( networkedPlayer.first ) );
			colourComponent->Colour = g_GameData.GetPlayerColour( networkedPlayer.first );
		}

		// Create mapping for selected entities
		m_SelectedEntities.emplace( networkedPlayer.first, ENTITY_INVALID );
	}
}

void SSEditorToolbox::Shutdown()
{
	g_SSMail.UnregisterSubscriber( this );
	m_SimInterests = 0;

	for( auto placementGhostEntity : m_PlacementGhostEntities )
	{
		g_EntityManager.RemoveEntity( placementGhostEntity.second );
	}
	m_PlacementGhostEntities.clear();
}

void SSEditorToolbox::UpdateUserLayer( const float deltaTime )
{
	m_MouseOverEntity = ENTITY_INVALID;
	rVector<unsigned int> outEntities;

	PickRelevantEntities( *g_SSCamera.GetActiveCamera(), m_PickingTargetGroups, outEntities );
	CalcTerrainPosUnderMouse( *g_SSCamera.GetActiveCamera(), &m_PickedPosition );
	!outEntities.empty() ? m_MouseOverEntity = outEntities[0] : m_MouseOverEntity = ENTITY_INVALID;	

	// Make the ghost object follow the cursor
	Entity localPlacementGhostEntity = m_PlacementGhostEntities.at( g_PlayerData.GetPlayerID() );
	vec3 newPosition = g_SSEditorToolbox.GetPickedPosition();
	GetDenseComponent<PlacementComponent>( localPlacementGhostEntity )->Position = newPosition;
	GetDenseComponent<CollisionComponent>( localPlacementGhostEntity )->CollisionEntity->SetPosition( newPosition );
	g_SSMail.PushToUserLayer( UpdateGhostEntityPositionMessage( g_PlayerData.GetPlayerID(), newPosition ) );

	// Handle switch to move tool>
	if ( g_Input->MouseDownUp( MOUSE_BUTTON_RIGHT ) && !IsToolSelected(Tool::TerrainEditTool) )
		SelectTool( Tool::SelectionTool );

	// Use tools that aren't specific to a category
	static unsigned int movingEntity = ENTITY_INVALID;
	if ( m_SelectedTool == Tool::MoveTool )
	{
		if ( m_MouseOverEntity != ENTITY_INVALID && g_Input->MouseUpDown( MOUSE_BUTTON_LEFT ) )
			movingEntity = m_MouseOverEntity;

		if ( movingEntity != ENTITY_INVALID )
		{
			MoveObjectMessage moveMessage( g_GameData.GetFrameCount() + 1, movingEntity, m_PickedPosition.x, m_PickedPosition.z );
			g_SSMail.PushToNextFrame( moveMessage );

			if ( g_Input->MouseUp( MOUSE_BUTTON_LEFT ) )
				movingEntity = ENTITY_INVALID;
		}
	}
	else if ( m_SelectedTool == Tool::SelectionTool )
	{
		if ( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT )
			&& m_MouseOverEntity != m_SelectedEntities.at( g_PlayerData.GetPlayerID() ) // Don't select if the object being selected is already selected
			&& !( m_MouseOverEntity == ENTITY_INVALID && m_SelectedEntities.at( g_PlayerData.GetPlayerID() ) == ENTITY_INVALID ) )	// Don't send selection message if nothing is selected and we are deselecting
		{
			g_SSMail.PushToUserLayer( SelectEntityMessage( g_PlayerData.GetPlayerID(), m_MouseOverEntity ) );
		}
	}

	for ( int i = 0; i < m_UserMailbox.size(); ++i )
	{
		const Message* message = m_UserMailbox[i];
		switch ( message->Type )
		{
			case MessageTypes::UPDATE_GHOST_ENTITY_POS:
			{
				const UpdateGhostEntityPositionMessage* ghostEntityUpdateMessage = static_cast<const UpdateGhostEntityPositionMessage*>( message );
				if ( ghostEntityUpdateMessage->OwnerPlayerID != g_PlayerData.GetPlayerID() )
				{
					Entity localPlacementGhostEntity = m_PlacementGhostEntities.at( ghostEntityUpdateMessage->OwnerPlayerID );
					GetDenseComponent<PlacementComponent>( localPlacementGhostEntity )->Position = ghostEntityUpdateMessage->NewPosition;
					GetDenseComponent<CollisionComponent>( localPlacementGhostEntity )->CollisionEntity->SetPosition( ghostEntityUpdateMessage->NewPosition );
				}
			} break;

			case MessageTypes::SELECT_ENTITY:
			{

				// Do not select entites without host authorization // TODODB: Make selectionMEssage a simulated message. Order needs to be correct and this is a hax to get that.
				if ( !g_NetworkInfo.AmIHost() && !message->CreatedFromPacket )
					continue;

				const SelectEntityMessage* selectEntityMessage = static_cast<const SelectEntityMessage*>( message );

				// If we are setting ENTITY_INVALID just do so and continue
				if ( selectEntityMessage->EntityID == ENTITY_INVALID )
				{
					GetDenseComponent<ColourComponent>( m_SelectedEntities.at( selectEntityMessage->SelectingPlayerID ) )->Colour = Colours::WHITE;
					m_SelectedEntities.at( selectEntityMessage->SelectingPlayerID ) = ENTITY_INVALID;
					continue;
				}

				// Check so that no other player already has selected the entity
				bool shouldContinue = false;
				for ( auto playerSelectedEntity : m_SelectedEntities )
				{
					if ( playerSelectedEntity.second == selectEntityMessage->EntityID )
					{
						shouldContinue = true;
						break;
					}
				}

				if ( shouldContinue )
				{
					// Deselct any selected entity if a new selection is blocked
					if ( m_SelectedEntities.at( selectEntityMessage->SelectingPlayerID ) != ENTITY_INVALID )
					{
						GetDenseComponent<ColourComponent>( m_SelectedEntities.at( selectEntityMessage->SelectingPlayerID ) )->Colour = Colours::WHITE;
						m_SelectedEntities.at( selectEntityMessage->SelectingPlayerID ) = ENTITY_INVALID;
					}
					continue;
				}

				// Remove the tint from the previously selected entity
				if ( m_SelectedEntities.at( selectEntityMessage->SelectingPlayerID ) != ENTITY_INVALID )
					GetDenseComponent<ColourComponent>( m_SelectedEntities.at( selectEntityMessage->SelectingPlayerID ) )->Colour = Colours::WHITE;

				// Assign the selected entity to to the selecting player and tint it with the selecting players colour
				m_SelectedEntities.at( selectEntityMessage->SelectingPlayerID ) = selectEntityMessage->EntityID;
				GetDenseComponent<ColourComponent>( selectEntityMessage->EntityID )->Colour = g_GameData.GetPlayerColour( selectEntityMessage->SelectingPlayerID );
			} break;

			default:
				Logger::Log( "SSEditorToolbox received unknown user message type " + rToString( message->Type ), "SSEditorToolbox", LogSeverity::WARNING_MSG );
				break;
		}
	}
}

void SSEditorToolbox::UpdateSimLayer( float timeStep )
{
	for ( int i = 0; i < m_SimMailbox.size(); ++i )
	{
		const Message* message = m_SimMailbox[i];
		switch ( message->Type )
		{
			case MessageTypes::MOVE_OBJECT:
			{
				const MoveObjectMessage* moveMessage = static_cast< const MoveObjectMessage* >( message );

				PlacementComponent* placementMessage = GetDenseComponent<PlacementComponent>( moveMessage->EntityToMove );
				placementMessage->Position.x = moveMessage->NewPosX;
				placementMessage->Position.z = moveMessage->NewPosZ;
			} break;

			case MessageTypes::UPDATE_GHOST_ENTITY_VISIBLILITY:
			{
				const UpdateGhostEntityVisibilityMessage* visibilityMessage = static_cast<const UpdateGhostEntityVisibilityMessage*>( message );
				ModelComponent* model = GetDenseComponent<ModelComponent>( m_PlacementGhostEntities.at( visibilityMessage->OwnerPlayerID ) );
				model->IsVisible = visibilityMessage->Isvisible;
			} break;

			case MessageTypes::UPDATE_GHOST_ENTITY_MODEL:
			{
				const UpdateGhostEntityModelMessage* modelMessage = static_cast<const UpdateGhostEntityModelMessage*>( message );
				ModelComponent* model = GetDenseComponent<ModelComponent>( m_PlacementGhostEntities.at( modelMessage->OwnerPlayerID ) );
				model->ModelHandle = gfx::g_ModelBank.LoadModel( modelMessage->ModelName.c_str() );
			} break;

			default:
				Logger::Log( "SSEditorToolbox received unknown simulation message type " + rToString( message->Type ), "SSEditorToolbox", LogSeverity::WARNING_MSG );
				break;
		}
	}
}

Tool SSEditorToolbox::GetSelectedTool() const
{
	return m_SelectedTool;
}

const vec3& SSEditorToolbox::GetPickedPosition() const
{
	return m_PickedPosition;
}

Entity SSEditorToolbox::GetMouseOverEntity() const
{
	return m_MouseOverEntity;
}

Entity SSEditorToolbox::GetSelectedEntity() const
{
	return m_SelectedEntities.at( g_PlayerData.GetPlayerID() );
}

void SSEditorToolbox::SetLocalPlacementGhostVisible( bool visible )
{
	g_SSMail.PushToNextFrame( UpdateGhostEntityVisibilityMessage( g_GameData.GetFrameCount() + 1, g_PlayerData.GetPlayerID(), visible ) );
}

void SSEditorToolbox::SetLocalPlacementGhostModel( const rString& modelFileName )
{
	g_SSMail.PushToNextFrame( UpdateGhostEntityModelMessage( g_GameData.GetFrameCount() + 1, g_PlayerData.GetPlayerID(), modelFileName ) );
}

bool SSEditorToolbox::IsToolSelected( Tool toCompare )
{
	return m_SelectedTool == toCompare;
}

bool SSEditorToolbox::IsToolSelected( const rVector<Tool>& toCompare )
{
	for ( int i = 0; i < toCompare.size(); ++i )
	{
		if ( m_SelectedTool == toCompare[i] )
			return true;
	}

	return false;
}

void SSEditorToolbox::SelectTool( Tool newTool )
{
	OnToolDeSelected( m_SelectedTool ); // Deselect old tool
	OnToolSelected( newTool );			// Select new tool

	m_SelectedTool = newTool;
}

void SSEditorToolbox::CalcTerrainPosUnderMouse( const Camera& camera, vec3* const outPosition ) const
{
	const ivec2 mousePosition( g_Input->GetMousePosX(), g_Input->GetMousePosY() );
	const ivec2 windowSize( camera.GetLens().WindowWidth, camera.GetLens().WindowHeight );
	const mat4 invViewProj	= glm::inverse( camera.GetViewProjection() );

	Ray ray;
	CalculateRayFromPixel( mousePosition, windowSize, invViewProj, &ray );

	rVector<unsigned int>	unitsSelected;
	rVector<int>			targetGroups;
	targetGroups.push_back( PICKING_TYPE_TERRAIN );

	g_CollisionDetection.PickingWithRay( ray.Position, ray.Direction, targetGroups, unitsSelected, outPosition );
}

void SSEditorToolbox::PickRelevantEntities( const Camera& camera, const rVector<int>& targetGroups, rVector<unsigned int>& pickedEntities )
{
	const ivec2 mousePosition( g_Input->GetMousePosX(), g_Input->GetMousePosY() );
	const ivec2 windowSize( camera.GetLens().WindowWidth, camera.GetLens().WindowHeight );
	const mat4 invViewProj	= glm::inverse( camera.GetViewProjection() );

	Ray ray;
	CalculateRayFromPixel( mousePosition, windowSize, invViewProj, &ray );

	vec3 dummy; // We will never want the outPosition
	g_CollisionDetection.PickingWithRay( ray.Position, ray.Direction, targetGroups, pickedEntities, &dummy );
}

void SSEditorToolbox::CalculateRayFromPixel( const glm::ivec2& pixel, const glm::ivec2& windowSize, const glm::mat4& invViewProj, Ray* outRay ) const
{
	// Clip space coordinates for the pixel. (-1,-1) in lower left corner, (-1,1) upper left corner, (1,-1) lower right corner. 
	const vec2	mousePosNorm	= vec2( -1.0f + 2.0f * ( pixel.x / static_cast<float>( windowSize.x ) ),
		1.0f - 2.0f * ( pixel.y / static_cast<float>( windowSize.y ) ) );

	// Translating pixel at near plane and far plane to world coordinates. Z-coordinate is depth into the screen (values between -1 and 1 are in view of camera).
	const vec4 nearHomogeneous	= invViewProj * vec4( mousePosNorm.x, mousePosNorm.y, 0.0f, 1.0f );
	const vec4 farHomogeneous	= invViewProj * vec4( mousePosNorm.x, mousePosNorm.y, 1.0f, 1.0f );
	const vec3 nearWorld		= vec3( nearHomogeneous ) / nearHomogeneous.w;
	const vec3 farWorld			= vec3( farHomogeneous ) / farHomogeneous.w;

	outRay->Position			= nearWorld;
	outRay->Direction			= glm::normalize( farWorld - nearWorld );
}

void SSEditorToolbox::OnToolSelected( Tool tool )
{
	g_PlayerData.SetLastActionPressed( ACTION::ACTION_SIZE );
	switch ( tool )
	{
		case Tool::None:
		case Tool::PlaceSFXEmitter:
		{} break;

		case Tool::MoveTool:
		{
			AddTargetGroup( PICKING_TYPE_PROP );
			AddTargetGroup( PICKING_TYPE_RESOURCE );
			AddTargetGroup( PICKING_TYPE_CONTROL_POINT );
			AddTargetGroup( PICKING_TYPE_SFX_EMITTER );
			AddTargetGroup( PICKING_TYPE_PARTICLE_EMITTER );
			g_PlayerData.SetLastActionPressed( ACTION::ACTION_EDITOR_MOVE );
		} break;

		case Tool::SelectionTool:
		{
			AddTargetGroup( PICKING_TYPE_PROP );
			AddTargetGroup( PICKING_TYPE_RESOURCE );
			AddTargetGroup( PICKING_TYPE_CONTROL_POINT );
			AddTargetGroup( PICKING_TYPE_SFX_EMITTER );
			AddTargetGroup( PICKING_TYPE_PARTICLE_EMITTER );
		} break;
			
		case Tool::PlaceProp:
		{
			SetLocalPlacementGhostVisible( true );
		} break;
			
		case Tool::PlaceResouce:
		{
			SetLocalPlacementGhostVisible( true );
		} break;
		
		case Tool::PlaceControlPoint:
		{
			SetLocalPlacementGhostVisible( true );
		} break;

		default:
			Logger::Log( "Unknown tool received in OnToolSelected", "SSEditorToolbox", LogSeverity::WARNING_MSG );
			break;
	}
}

void SSEditorToolbox::OnToolDeSelected( Tool tool )
{
	switch ( tool )
	{
		case Tool::None:
		case Tool::PlaceSFXEmitter:
		{} break;

		case Tool::MoveTool:
		{
			RemoveTargetGroup( PICKING_TYPE_PROP );
			RemoveTargetGroup( PICKING_TYPE_RESOURCE );
			RemoveTargetGroup( PICKING_TYPE_CONTROL_POINT );
			RemoveTargetGroup( PICKING_TYPE_SFX_EMITTER );
			RemoveTargetGroup( PICKING_TYPE_PARTICLE_EMITTER );
		} break;

		case Tool::SelectionTool:
		{
			RemoveTargetGroup( PICKING_TYPE_PROP );
			RemoveTargetGroup( PICKING_TYPE_RESOURCE );
			RemoveTargetGroup( PICKING_TYPE_CONTROL_POINT );
			RemoveTargetGroup( PICKING_TYPE_PARTICLE_EMITTER );
			
			// Deselect if anything is selected
			if ( m_SelectedEntities.at( g_PlayerData.GetPlayerID() ) != ENTITY_INVALID )
				g_SSMail.PushToUserLayer( SelectEntityMessage( g_PlayerData.GetPlayerID(), ENTITY_INVALID ) ); 
		} break;
			
		case Tool::PlaceProp:
		{
			SetLocalPlacementGhostVisible( false );
		} break;
			
		case Tool::PlaceResouce:
		{
			SetLocalPlacementGhostVisible( false );
		} break;

		case Tool::PlaceControlPoint:
		{
			SetLocalPlacementGhostVisible( false );
		} break;
			
		default:
			Logger::Log( "Unknown tool received in OnToolSelected", "SSEditorToolbox", LogSeverity::WARNING_MSG );
			break;
	}
}

void SSEditorToolbox::AddTargetGroup( ENTITY_TYPE targetGroup )
{
	// Check against duplicates
	for ( int i = 0; i < m_PickingTargetGroups.size(); ++i )
	{
		if ( m_PickingTargetGroups[i] == targetGroup )
		{
			Logger::Log( "Attempted to add already added picking target group " + rToString( targetGroup ), "SSEditorToolbox", LogSeverity::WARNING_MSG );
			return;
		}
	}

	// add target group
	m_PickingTargetGroups.push_back( targetGroup );
}

void SSEditorToolbox::RemoveTargetGroup( ENTITY_TYPE targetGroup )
{
	for ( int i = 0; i < m_PickingTargetGroups.size(); ++i )
	{
		if ( m_PickingTargetGroups[i] == targetGroup )
		{
			m_PickingTargetGroups.erase( m_PickingTargetGroups.begin() + i );
			return;
		}
	}

	Logger::Log( "Attempted to remove unexisting picking target group " + rToString( targetGroup ), "SSEditorToolbox", LogSeverity::WARNING_MSG );
}

glm::ivec2 SSEditorToolbox::WorldToScreen( const glm::vec3& worldPos )
{
	glm::vec4 pos( worldPos, 1.0f );
	glm::vec4 screenPosUnProjected = g_SSCamera.GetActiveCamera()->GetViewProjection() * pos;
	glm::vec3 screenPos = glm::vec3( screenPosUnProjected ) / screenPosUnProjected.w;
	screenPos = screenPos * 0.5f + 0.5f;
	screenPos.y = 1.0f - screenPos.y;
	
	return glm::ivec2( screenPos.x * g_GameData.GetWindowWidth(), screenPos.y * g_GameData.GetWindowHeight() );
}