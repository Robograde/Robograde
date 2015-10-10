/**************************************************
2015 Ola Enberg
***************************************************/

#pragma once

#include "../Subsystem.h"

#include <script/ScriptEngine.h>
#include "SSUpgrades.h"

#define g_SSResearch SSResearch::GetInstance()	// Conveniency access function

enum RESEARCH_STATUS_CHANGE
{
	RESEARCH_STATUS_CHANGE_STARTED,
	RESEARCH_STATUS_CHANGE_FINISHED,
};

struct ResearchItem
{
	bool	Done				= false;
	int		PointsResearched	= 0;
};

struct PlayerResearch
{
	pVector<ResearchItem>		ResearchItems;
	int							ActiveResearchItem	= -1;
	pDeque<int>					ResearchQueue;
};

class SSResearch : public Subsystem
{
public:
	static SSResearch&			GetInstance					();

	void						Startup						() override;
	void						UpdateSimLayer				( const float deltaTime ) override;
	void						Shutdown					() override;

	void						SwitchPlayerActiveResearch	( int playerID, UPGRADE_NAME upgrade );
	void						QueuePlayerResearch			( int playerID, UPGRADE_NAME upgrade );
	void						GiveResearchToPlayer		( int playerID, UPGRADE_NAME upgrade );
	void						UnQueuePlayerResearch		( int playerID, UPGRADE_NAME upgrade );

	bool						GetPlayerResearchDone		( int playerID, UPGRADE_NAME upgrade ) const;
	int							GetPlayerResearchProgress	( int playerID, UPGRADE_NAME upgrade ) const;
	UPGRADE_NAME				GetPlayerActiveResearch		( int playerID ) const;
	const pDeque<int>&			GetPlayerResearchQueue		( int playerID ) const;

	void						SetTimeBetweenTicks			( float newTimeBetweenTicks );

private:
								// No external instancing allowed
								SSResearch					() : Subsystem( "Research" ) {}
								SSResearch					( const SSResearch & rhs );
								~SSResearch					() {};
	SSResearch&					operator=					( const SSResearch & rhs );

	void						TickResearch				();
	void						FinishActiveResearch		( int playerID );
	bool						ValidPlayer					( int playerID );
	bool						ValidIndices				( int playerID, UPGRADE_NAME upgrade );
	bool						ValidToResearch				( int playerID, UPGRADE_NAME upgrade );
	void						CreateVariableName			( const rString& in, rString* out );

	void						ResetResearchForPlayer		( int playerID, UPGRADE_NAME upgrade );
	void						GiveAllResearchToPlayer		( int playerID );
	void						QueueAllResearchForPlayer	( int playerID );
	void						ResetAllResearchForPlayer	( int playerID );

	int							ScriptSwitchActiveResearch	( IScriptEngine* scriptEngine );
	int							ScriptQueueResearch			( IScriptEngine* scriptEngine );
	int							ScriptSetResearchSpeed		( IScriptEngine* scriptEngine );
	int							ScriptGiveResearch			( IScriptEngine* scriptEngine );
	int							ScriptResetResearch			( IScriptEngine* scriptEngine );
	int							ScriptGiveAllResearch		( IScriptEngine* scriptEngine );
	int							ScriptQueueAllResearch		( IScriptEngine* scriptEngine );
	int							ScriptResetAllResearch		( IScriptEngine* scriptEngine );

	rVector<PlayerResearch>		m_PlayerResearches;
	float						m_TimeToNextTick;
	float						m_TimeBetweenTicks;
};