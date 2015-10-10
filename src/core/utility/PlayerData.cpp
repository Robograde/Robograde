/**************************************************
Copyright 2015 Robograde Development Team
***************************************************/

#include "PlayerData.h"
#include "../CompileFlags.h"
#include "GlobalDefinitions.h"
#include "../datadriven/EntityManager.h"
#include "../datadriven/DenseComponentCollection.h"
#include "../component/OwnerComponent.h"
#include "../component/AgentComponent.h"

PlayerData& PlayerData::GetInstance()
{
	static PlayerData instance;
	return instance;
}

void PlayerData::Reset()
{
	m_SelectedSquads.clear();
	m_SelectedEntities.clear();

	m_PlayerID			= PLAYER_ID_INVALID;
	m_PlayerCount		= MAX_PLAYERS;

	for ( int i = 0; i < MAX_CONTROL_GROUPS; ++i )
		m_ControlGroups[i].clear();

	g_Script.Register( "GE_GetSelectedPlayer",	std::bind( &PlayerData::ScriptGetSelectedPlayer,	this, std::placeholders::_1 ) );
	g_Script.Register( "GE_GetSelectedEntity",	std::bind( &PlayerData::ScriptGetSelectedEntity,	this, std::placeholders::_1 ) );
	g_Script.Register( "GE_GetSelectedSquad",	std::bind( &PlayerData::ScriptGetSelectedSquad,		this, std::placeholders::_1 ) );
	DEV(
		g_Script.Register( "GE_PlayAs",				std::bind( &PlayerData::ScriptSetPlayerID,			this, std::placeholders::_1 ) );
	);
}

void PlayerData::RemoveSquadFromControlGroup( int controlGroupIndex, int squadIndexToRemove )
{
	m_ControlGroups[controlGroupIndex].erase( m_ControlGroups[controlGroupIndex].begin() + squadIndexToRemove );
}

const rString& PlayerData::GetPlayerName() const
{
	return m_PlayerName;
}

const rVector<int>&	PlayerData::GetSelectedSquads() const
{
	return m_SelectedSquads;
}

rVector<int>& PlayerData::GetEditableSelectedSquads()
{
	return m_SelectedSquads;
}

const rVector<Entity>&	PlayerData::GetSelectedEntities() const
{
	return m_SelectedEntities;
}

rVector<Entity>& PlayerData::GetEditableSelectedEntities()
{
	return m_SelectedEntities;
}

ACTION PlayerData::GetLastActionPressed() const
{
	return m_LastActionPressed;
}

short PlayerData::GetPlayerID() const
{
	return m_PlayerID;
}

short PlayerData::GetPlayerCount() const
{
	return m_PlayerCount;
}

const rVector<int>& PlayerData::GetControlGroup( short controlGroup ) const
{
	return m_ControlGroups[controlGroup];
}

fString PlayerData::GetControlGroupStringForSquad( int squadID, int max ) 
{
	fString controlGroups = "";
	int count = 0;
	for( int i = 0; i < MAX_CONTROL_GROUPS; i++ )
	{
		for( int squadInGroup : m_ControlGroups[i] )
		{
			if( squadInGroup == squadID )
			{
				if( controlGroups != "" )
					controlGroups += ",";

				controlGroups += fToString( i + 1 );
				count++;
				if( count == max )
					return controlGroups;
			}
		}
	}
	return controlGroups;
}

void PlayerData::SetPlayerName( const rString& name )
{
	m_PlayerName = name;
}

void PlayerData::SetSelectedSquads( const rVector<int>& newSquadSelection )
{
	m_SelectedSquads = newSquadSelection;
}

void PlayerData::SetSelectedEntities( const rVector<Entity>& newEntitySelection )
{
	m_SelectedEntities = newEntitySelection;
}

void PlayerData::SetPlayerID( const short playerID )
{
	m_PlayerID = playerID;
	g_Script.SetInt( "g_LocalPlayerID", static_cast<int>( playerID ) );
}

void PlayerData::SetControlGroup( short controlGroupID, const rVector<int>& squads )
{
	m_ControlGroups[controlGroupID] = squads;
}

void PlayerData::SetLastActionPressed( ACTION action )
{
	m_LastActionPressed = action;
}

int PlayerData::ScriptGetSelectedPlayer( IScriptEngine* scriptEngine )
{
	int selectedPlayer = this->GetPlayerID();

	if ( this->m_SelectedSquads.empty() )
	{
		if ( !this->m_SelectedEntities.empty() )
		{
			Entity entityID			= this->m_SelectedEntities[0];
			EntityMask entityMask	= g_EntityManager.GetEntityMask( entityID );
			EntityMask ownerFlag	= DenseComponentCollection<OwnerComponent>::GetInstance().GetComponentTypeFlag();

			if ( entityMask & ownerFlag )
			{
				OwnerComponent* ownerComponent = GetDenseComponent<OwnerComponent>( entityID );
				selectedPlayer = ownerComponent->OwnerID;
			}
			else
			{
				selectedPlayer = -1;
			}
		}
	}

	scriptEngine->PushInt( selectedPlayer );

	return 1;	// Number of return values.
}

int PlayerData::ScriptGetSelectedEntity( IScriptEngine* scriptEngine )
{
	int selectedEntity = -1;

	if ( !this->m_SelectedEntities.empty() )
	{
		selectedEntity = this->m_SelectedEntities[0];
	}

	scriptEngine->PushInt( selectedEntity );

	return 1;	// Number of return values.
}

int PlayerData::ScriptGetSelectedSquad( IScriptEngine* scriptEngine )
{
	int selectedSquad = 0;

	if ( !this->m_SelectedSquads.empty() )
	{
		selectedSquad = this->m_SelectedSquads[0];
	}
	else if ( !this->m_SelectedEntities.empty() )
	{
		Entity entityID			= this->m_SelectedEntities[0];
		EntityMask entityMask	= g_EntityManager.GetEntityMask( entityID );
		EntityMask agentFlag	= DenseComponentCollection<AgentComponent>::GetInstance().GetComponentTypeFlag();

		if ( entityMask & agentFlag )
		{
			AgentComponent* agentComponent = GetDenseComponent<AgentComponent>( entityID );
			selectedSquad = agentComponent->Agent->GetSquadID();
		}
	}

	scriptEngine->PushInt( selectedSquad );

	return 1;	// Number of return values.
}

int PlayerData::ScriptSetPlayerID( IScriptEngine* scriptEngine )
{
	int playerID = scriptEngine->PopInt();

	this->SetPlayerID( playerID );

	return 0; // Number of return values.
}
