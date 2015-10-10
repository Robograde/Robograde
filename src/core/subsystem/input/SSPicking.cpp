/**************************************************
Copyright 2015 Ola Enberg & Daniel Bengtsson
***************************************************/

#include "SSPicking.h"
#include "../../CompileFlags.h"

#include <memory/Alloc.h>
#include <network/NetworkInfo.h>
#include <network/PacketPump.h>
#include <input/Input.h>
#include <glm/gtx/transform.hpp>
#include <gui/GUIEngine.h>
#include <messaging/GameMessages.h>
#include "../gamelogic/SSCamera.h"
#include "../gamelogic/SSAI.h"
#include "../utility/SSMail.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/PlacementComponent.h"
#include "../../component/SelectionComponent.h"
#include "../../datadriven/EntityManager.h"

SSPicking& SSPicking::GetInstance()
{
	static SSPicking instance;
	return instance;
}

void SSPicking::Startup()
{
	DEV(
		g_Script.Register( "GE_SetShowTerrainPicking", std::bind( &SSPicking::ScriptSetShowTerrainPicking, this, std::placeholders::_1 ) );
	);

	m_ShowTerrainPicking		= false;

	m_BoxSprite.Colour			= glm::vec4( 0.0f, 1.0f, 0.0f, 0.2f );
	m_BoxSprite.Texture			= "";
	m_BoxSprite.BorderSize		= 1;
	m_BoxSprite.BorderColour	= glm::vec4( 0.0f, 1.0f, 0.0f, 0.6f );

	m_DebugRenderer.Initialize();

	Subsystem::Startup();
}

void SSPicking::UpdateUserLayer( const float deltaTime )
{
	PickingAction pickingAction;
	m_Picking.Update( *g_SSCamera.GetActiveCamera(), pickingAction );

	rVector<int>&		selectedSquads		= g_PlayerData.GetEditableSelectedSquads();
	rVector<Entity>&	selectedEntities	= g_PlayerData.GetEditableSelectedEntities();
	switch ( pickingAction )
	{
		case PickingAction::None:
			// Do nothing.
			break;

		case PickingAction::UnitSelection:
		{
			if ( PICKING_ENTITY_DEBUG && !m_Picking.GetUnitsSelected().empty() )
			{
				Logger::GetStream() << "Picked EntityID(s): ";
				for ( auto& entity : m_Picking.GetUnitsSelected() )
				{
					Logger::GetStream() << entity << " ";
				}
				Logger::GetStream() << std::endl;
			}

			selectedEntities.clear();
			int teamID = -1;
			rVector<int> pickedSquads = g_SSAI.GetSquadsFromSelection( m_Picking.GetUnitsSelected(), &teamID, g_PlayerData.GetEditableSelectedEntities() );
			if ( selectedEntities.empty() )
				selectedEntities = m_Picking.GetUnitsSelected();

			if ( pickedSquads.size() > 0 && teamID == g_PlayerData.GetPlayerID() )
			{
				selectedEntities.clear();

				if ( g_Input->KeyUp( SDL_SCANCODE_LSHIFT ) )
				{
					selectedSquads = pickedSquads;
				}
				else // If shift is held we should apend or remove according to the new slection
				{
					//Look for copies
					for ( int i = 0; i < pickedSquads.size(); ++i )
					{	
						int copyIndex = -1;
						for ( int j = 0; j < selectedSquads.size(); ++j )
						{
							if ( pickedSquads[i] == selectedSquads[j] )
							{
								copyIndex = j;
								break;
							}
						}
						if ( m_Picking.WasLastPickBox() && copyIndex == -1 )		 // Append the selection if it was a box select and not copy
							selectedSquads.push_back( pickedSquads[i] );
						else if ( !m_Picking.WasLastPickBox() && copyIndex == -1 )	 // Append the selection if it was picked and not a copy
							selectedSquads.push_back( pickedSquads[i] );
						else if ( !m_Picking.WasLastPickBox() && copyIndex != -1 )	 // Remove the selection from the current selection if it was picked and not a copy
							selectedSquads.erase( selectedSquads.begin() + copyIndex );
					}
				}
			}
			if ( teamID != g_PlayerData.GetPlayerID() )
				selectedSquads.clear();
		} break;

		case PickingAction::UnitDeselect:
			selectedSquads.clear();
			selectedEntities.clear();
			break;

		default:
		{
			if ( selectedSquads.size() > 0 )
			{
				int					missionType			= TranslateToMissionType( pickingAction );
				int					teamID				= g_PlayerData.GetPlayerID();
				const glm::vec3&	targetLocation		= m_Picking.GetClickedPosition();
				Entity				targetEntity		= m_Picking.GetUnitsSelected().empty() ? ENTITY_INVALID : m_Picking.GetUnitsSelected()[0];
				bool				overridingCommand	= g_Input->KeyUp( SDL_SCANCODE_LSHIFT );

				OrderUnitsMessage message = OrderUnitsMessage( g_GameData.GetFrameCount(), missionType, teamID, selectedSquads, targetLocation, targetEntity, overridingCommand );
				g_SSMail.PushToCurrentFrame( message );
			}
		} break;
	}

	g_SSCamera.SetBlockMovement( m_Picking.GetSelectionBox() != nullptr );

	PrepairSelectionRender();
	RenderBoxSelection();

	if ( m_ShowTerrainPicking )
	{
		RenderClickedTerrainPos();
	}
}

void SSPicking::Shutdown()
{
	Subsystem::Shutdown();
}

bool SSPicking::IsBoxSelecting() const
{
	return m_Picking.IsBoxSelecting();
}

void SSPicking::SetShowTerrainPicking( bool showPicking )
{
	m_ShowTerrainPicking = showPicking;
}

glm::ivec2 SSPicking::WorldToScreen( const glm::vec3& worldPos )
{
	glm::vec4 pos( worldPos, 1.0f );
	glm::vec4 screenPosUnProjected = g_SSCamera.GetActiveCamera()->GetViewProjection() * pos;
	glm::vec3 screenPos = glm::vec3( screenPosUnProjected ) / screenPosUnProjected.w;
	screenPos = screenPos * 0.5f + 0.5f;
	screenPos.y = 1.0f - screenPos.y;
	
	return glm::ivec2( screenPos.x * g_GameData.GetWindowWidth(), screenPos.y * g_GameData.GetWindowHeight() );
}

int SSPicking::TranslateToMissionType( PickingAction pickingAction )
{
	switch ( pickingAction )
	{
		case PickingAction::Move:
			return Squad::MissionType::MISSION_TYPE_MOVE;
			break;

		case PickingAction::AttackMove:
			return Squad::MissionType::MISSION_TYPE_ATTACK_MOVE;
			break;

		case PickingAction::Attack:
			return Squad::MissionType::MISSION_TYPE_ATTACK;
			break;

		case PickingAction::Gather:
			return Squad::MissionType::MISSION_TYPE_MINE;
			break;

		case PickingAction::Capture:
			return Squad::MissionType::MISSION_TYPE_MOVE;	// TODOOE: Change this to capture mission when it is implemented.
			break;

		default:
			assert( false );	// Mapping from specified PickingAction to a MissionType::MISSION_TYPE not implemented.
			return -1;
			break;
	}
}

void SSPicking::PrepairSelectionRender()
{
	EntityMask selectionFlag = DenseComponentCollection<SelectionComponent>::GetInstance().GetComponentTypeFlag();

	m_MouseOveredEntities.clear();

	// Calculate mouse over targets.
	if ( !m_Picking.GetUnitsSelected().empty() )
	{
		int teamID = -1;
		g_SSAI.GetSquadsFromSelection( m_Picking.GetUnitsSelected(), &teamID, m_MouseOveredEntities );

		if ( m_MouseOveredEntities.empty() )
		{
			for ( auto& entityID : m_Picking.GetUnitsSelected() )
			{
				m_MouseOveredEntities.push_back( entityID );
			}
		}
	}

	// Mark mouse overed units for mouse over rendering.
	for ( auto& entityID : m_MouseOveredEntities )
	{
		EntityMask entityMask = g_EntityManager.GetEntityMask( entityID );
		if ( entityMask & selectionFlag )
		{
			GetDenseComponent<SelectionComponent>( entityID )->MouseOvered = true;
		}
	}

	// Mark selected units for selection rendering
	if ( g_PlayerData.GetSelectedSquads().empty() )
	{
		for ( auto& entityID : g_PlayerData.GetSelectedEntities() )
		{
			EntityMask entityMask = g_EntityManager.GetEntityMask( entityID );
			if ( entityMask & selectionFlag )
			{
				GetDenseComponent<SelectionComponent>( entityID )->Selected = true;
			}
		}
	}
	else
	{
		for ( auto& squadID : g_PlayerData.GetSelectedSquads() )
		{
			for ( auto& agent : g_SSAI.GetSquadWithID( g_PlayerData.GetPlayerID(), squadID )->GetAgents() )
			{
				EntityMask entityMask = g_EntityManager.GetEntityMask( agent->GetEntityID() );
				if ( entityMask & selectionFlag )
				{
					GetDenseComponent<SelectionComponent>( agent->GetEntityID() )->Selected = true;
				}
			}
		}
	}
}

void SSPicking::RenderBoxSelection()
{
	const SelectionBox* selectionBox = m_Picking.GetSelectionBox();
	if ( selectionBox )
	{
		m_BoxSprite.Position.x	= selectionBox->Position.x;
		m_BoxSprite.Position.y	= selectionBox->Position.y;
		m_BoxSprite.Width		= selectionBox->Size.x;
		m_BoxSprite.Height		= selectionBox->Size.y;

		g_GUI.EnqueueSprite( &m_BoxSprite );
	}
}

void SSPicking::RenderClickedTerrainPos()
{
	glm::mat4 world = glm::translate( m_Picking.GetClickedPosition() );

	m_DebugRenderer.SetViewProj( g_SSCamera.GetActiveCamera()->GetView(), g_SSCamera.GetActiveCamera()->GetProjection() );
	m_DebugRenderer.RenderBox( world, glm::vec3( 1.0f ) );
}

int SSPicking::ScriptSetShowTerrainPicking( IScriptEngine* scriptEngine )
{
	bool activation = scriptEngine->PopBool();

	this->SetShowTerrainPicking( activation );

	return 0; // Number of return values.
}
