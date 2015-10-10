/**************************************************
2015 Robograde Development Team
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include <script/ScriptEngine.h>
#include "../datadriven/ComponentTypes.h"
#include "../input/Action.h"
#include "GlobalDefinitions.h"

#define g_PlayerData		PlayerData::GetInstance()
#define PLAYER_ID_INVALID	SHRT_MIN
#define MAX_CONTROL_GROUPS	10

class PlayerData
{
public:
	static PlayerData& GetInstance();

	void Reset();

	short					GetPlayerID()							const;
	short					GetPlayerCount()						const;
	const rVector<int>&		GetControlGroup( short controlGroupID )	const;
	fString					GetControlGroupStringForSquad( int squadID, int max );
	
	void					RemoveSquadFromControlGroup( int controlGroupIndex, int squadIndexToRemove );

	const rString&			GetPlayerName() const;
	const rVector<int>&		GetSelectedSquads() const;
	rVector<int>&			GetEditableSelectedSquads();
	const rVector<Entity>&	GetSelectedEntities() const;
	rVector<Entity>&		GetEditableSelectedEntities();
	ACTION					GetLastActionPressed() const;
	
	void					SetPlayerName( const rString& name );
	void					SetSelectedSquads( const rVector<int>& newSquadSelection );
	void					SetSelectedEntities( const rVector<Entity>& newEntitySelection );
	void					SetPlayerID( const short playerID );
	void					SetControlGroup( short controlGroupID, const rVector<int>& squads );
	void					SetLastActionPressed( ACTION action );
	
private:
							// No external instancing allowed
							PlayerData()			{ }
							PlayerData				( const PlayerData& rhs );
							~PlayerData()			{ }
	PlayerData&				operator=				( const PlayerData& rhs );

	int						ScriptGetSelectedPlayer	( IScriptEngine* scriptEngine );
	int						ScriptGetSelectedEntity	( IScriptEngine* scriptEngine );
	int						ScriptGetSelectedSquad	( IScriptEngine* scriptEngine );
	int						ScriptSetPlayerID		( IScriptEngine* scriptEngine );

	short					m_PlayerID;
	short					m_PlayerCount = MAX_PLAYERS;
	rString					m_PlayerName;
	
	ACTION					m_LastActionPressed;
	rVector<int>			m_SelectedSquads;
	rVector<Entity>			m_SelectedEntities;
	rVector<int>			m_ControlGroups[MAX_CONTROL_GROUPS];
};