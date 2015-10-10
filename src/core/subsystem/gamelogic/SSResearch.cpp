/**************************************************
2015 Ola Enberg
***************************************************/

#include "SSResearch.h"
#include "../../CompileFlags.h"

#include <utility/Logger.h>
#include "../../utility/PlayerData.h"
#include "../../input/GameMessages.h"
#include "SSControlPoint.h"
#include "../utility/SSMail.h"

#define RESEARCH_NONE					-1
#define RESEARCH_TIME_BETWEEN_TICKS		1.0f
//#define RESEARCH_DEBUG

SSResearch& SSResearch::GetInstance()
{
	static SSResearch instance;
	return instance;
}

void SSResearch::Startup()
{
	g_Script.Register( "GE_SwitchActiveResearch",	std::bind( &SSResearch::ScriptSwitchActiveResearch,	this, std::placeholders::_1 ) );
	g_Script.Register( "GE_QueueResearch",			std::bind( &SSResearch::ScriptQueueResearch,		this, std::placeholders::_1 ) );
	DEV(
			g_Script.Register( "GE_SetResearchSpeed",		std::bind( &SSResearch::ScriptSetResearchSpeed,		this, std::placeholders::_1 ) );
			g_Script.Register( "GE_GiveResearch",			std::bind( &SSResearch::ScriptGiveResearch,			this, std::placeholders::_1 ) );
			g_Script.Register( "GE_ResetResearch",			std::bind( &SSResearch::ScriptResetResearch,		this, std::placeholders::_1 ) );
			g_Script.Register( "GE_GiveAllResearch",		std::bind( &SSResearch::ScriptGiveAllResearch,		this, std::placeholders::_1 ) );
			g_Script.Register( "GE_QueueAllResearch",		std::bind( &SSResearch::ScriptQueueAllResearch,		this, std::placeholders::_1 ) );
			g_Script.Register( "GE_ResetAllResearch",		std::bind( &SSResearch::ScriptResetAllResearch,		this, std::placeholders::_1 ) );
	   );
	m_TimeBetweenTicks	= RESEARCH_TIME_BETWEEN_TICKS;
	m_TimeToNextTick	= m_TimeBetweenTicks;

	rVector<UpgradeData>& upgradeArray = g_SSUpgrades.GetUpgradeArray();	
	for ( int i = 0; i < upgradeArray.size(); ++i )
	{
		rString consoleName;
		CreateVariableName( upgradeArray[i].Name, &consoleName );
		g_Script.SetInt( consoleName.c_str( ), i );
	}

	m_PlayerResearches.resize( g_PlayerData.GetPlayerCount() );
    for ( int playerID = 0; playerID < m_PlayerResearches.size(); ++playerID )
    {
        m_PlayerResearches[playerID].ResearchItems.resize( upgradeArray.size() );
        for ( int i = 0; i < m_PlayerResearches[playerID].ResearchItems.size(); ++i )
        {
            m_PlayerResearches[playerID].ResearchItems[i].PointsResearched = 0;
			if ( g_SSUpgrades.GetUpgrade( i ).Default )
				GiveResearchToPlayer( playerID, (UPGRADE_NAME) i );
        }
    }

	Subsystem::Startup();
}

void SSResearch::UpdateSimLayer( const float deltaTime )
{
	m_TimeToNextTick -= deltaTime;

	if ( m_TimeToNextTick <= 0.0f )
	{
		m_TimeToNextTick = m_TimeBetweenTicks;
		this->TickResearch();
	}
}

void SSResearch::Shutdown()
{
	m_PlayerResearches.clear();

	Subsystem::Shutdown();
}

void SSResearch::SwitchPlayerActiveResearch( int playerID, UPGRADE_NAME upgrade )
{
	if ( !this->ValidToResearch( playerID, upgrade ) )
	{
		return;
	}

	g_SSMail.PushToNextFrame( ResearchMessage( playerID, upgrade, RESEARCH_STATUS_CHANGE_STARTED ) );
	m_PlayerResearches[playerID].ResearchQueue.clear();
	m_PlayerResearches[playerID].ActiveResearchItem = upgrade;

	#ifdef RESEARCH_DEBUG
		Logger::GetStream() << "RESEARCH_DEBUG defined: [C=BLUE]Player " << playerID << "[C=WHITE] started researching [C=GREEN]" << g_SSUpgrades.GetUpgradeArray()[upgrade].Name << std::endl;
	#endif
}

void SSResearch::QueuePlayerResearch( int playerID, UPGRADE_NAME upgrade )
{
	if ( !this->ValidToResearch( playerID, upgrade ) )
	{
		return;
	}
	if ( m_PlayerResearches[playerID].ActiveResearchItem == upgrade )
	{
		Logger::Log( "Player " + rToString(playerID) + " cannot queue the research \"" + g_SSUpgrades.GetUpgradeArray()[upgrade].Name + "\" since it is already in progress.", "SSResearch", LogSeverity::WARNING_MSG );
		return;
	}
	for ( auto& queuedResearch : m_PlayerResearches[playerID].ResearchQueue )
	{
		if ( queuedResearch == upgrade )
		{
			Logger::Log( "Player " + rToString(playerID) + " cannot queue the research \"" + g_SSUpgrades.GetUpgradeArray()[upgrade].Name + "\" since it is already in the queue.", "SSResearch", LogSeverity::WARNING_MSG );
			return;
		}
	}

	if ( m_PlayerResearches[playerID].ActiveResearchItem != RESEARCH_NONE )
	{
		m_PlayerResearches[playerID].ResearchQueue.push_back( upgrade );
	}
	else
	{
		SwitchPlayerActiveResearch( playerID, upgrade );
		return;
	}

	#ifdef RESEARCH_DEBUG
		Logger::GetStream() << "RESEARCH_DEBUG defined: [C=BLUE]Player " << playerID << "[C=WHITE] queued research [C=GREEN]" << g_SSUpgrades.GetUpgradeArray()[upgrade].Name << std::endl;
	#endif
}

void SSResearch::GiveResearchToPlayer( int playerID, UPGRADE_NAME upgrade )
{
	if ( !this->ValidToResearch( playerID, upgrade ) )
	{
		return;
	}
	if ( m_PlayerResearches[playerID].ActiveResearchItem == upgrade )
	{
		this->FinishActiveResearch( playerID );
	}
	else
	{
		for ( int i = static_cast<int>(m_PlayerResearches[playerID].ResearchQueue.size()) - 1; i >= 0; --i )
		{
			if ( m_PlayerResearches[playerID].ResearchQueue[i] == upgrade )
			{
				m_PlayerResearches[playerID].ResearchQueue.erase( m_PlayerResearches[playerID].ResearchQueue.begin() + i );
			}
		}
	}
	m_PlayerResearches[playerID].ResearchItems[upgrade].Done = true;
}

void SSResearch::UnQueuePlayerResearch( int playerID, UPGRADE_NAME upgrade )
{
	for ( int i = static_cast<int>(m_PlayerResearches[playerID].ResearchQueue.size()) - 1; i >= 0; --i )
	{
		if (  m_PlayerResearches[playerID].ResearchQueue[i] == upgrade )
		{
			m_PlayerResearches[playerID].ResearchQueue.erase( m_PlayerResearches[playerID].ResearchQueue.begin() + i );
			return;
		}
	}
}

bool SSResearch::GetPlayerResearchDone( int playerID, UPGRADE_NAME upgrade ) const
{
	return m_PlayerResearches[playerID].ResearchItems[upgrade].Done;
}

int SSResearch::GetPlayerResearchProgress( int playerID, UPGRADE_NAME upgrade ) const
{
	return m_PlayerResearches[playerID].ResearchItems[upgrade].PointsResearched;
}

UPGRADE_NAME SSResearch::GetPlayerActiveResearch( int playerID ) const
{
	int activeResearch = m_PlayerResearches[playerID].ActiveResearchItem;
	return static_cast<UPGRADE_NAME>(activeResearch);
}

const pDeque<int>& SSResearch::GetPlayerResearchQueue( int playerID ) const
{
	return m_PlayerResearches[playerID].ResearchQueue;
}

void SSResearch::SetTimeBetweenTicks( float newTimeBetweenTicks )
{
	m_TimeBetweenTicks = newTimeBetweenTicks;
	m_TimeToNextTick = glm::min( m_TimeToNextTick, newTimeBetweenTicks );
}

void SSResearch::TickResearch()
{
	rVector<UpgradeData>& upgradeArray = g_SSUpgrades.GetUpgradeArray();

	for ( int i = 0; i < m_PlayerResearches.size(); ++i )
	{
		if ( m_PlayerResearches[i].ActiveResearchItem == RESEARCH_NONE )
		{
			continue;
		}

		ResearchItem* researchItem		= &m_PlayerResearches[i].ResearchItems[ m_PlayerResearches[i].ActiveResearchItem ];
		const int researchSpeed			= g_SSControlPoint.GetOwnedControlPointsCount( i );
		researchItem->PointsResearched += researchSpeed;

		const int pointsToCompletion = upgradeArray[ m_PlayerResearches[i].ActiveResearchItem ].PointsToCompletion;
		if ( researchItem->PointsResearched >= pointsToCompletion )
		{
			this->FinishActiveResearch( i );
		}
		#ifdef RESEARCH_DEBUG
			else if ( researchSpeed != 0 )
			{
				Logger::GetStream() << "RESEARCH_DEBUG defined: [C=BLUE]Player " << i << "[C=WHITE] has researched [C=RED]" << researchItem->PointsResearched << "/" << pointsToCompletion << "[C=WHITE] of [C=GREEN]" << upgradeArray[m_PlayerResearches[i].ActiveResearchItem].Name << std::endl;
			}
		#endif
	}
}

void SSResearch::FinishActiveResearch( int playerID )
{
	rVector<UpgradeData>&	upgradeArray		= g_SSUpgrades.GetUpgradeArray();
	ResearchItem*			researchItem		= &m_PlayerResearches[playerID].ResearchItems[ m_PlayerResearches[playerID].ActiveResearchItem ];
	const int				pointsToCompletion	= upgradeArray[ m_PlayerResearches[playerID].ActiveResearchItem ].PointsToCompletion;

	g_SSMail.PushToNextFrame( ResearchMessage( playerID, static_cast<UPGRADE_NAME>( m_PlayerResearches[playerID].ActiveResearchItem ), RESEARCH_STATUS_CHANGE_FINISHED ) );

	#ifdef RESEARCH_DEBUG
		Logger::GetStream() << "RESEARCH_DEBUG defined: [C=BLUE]Player " << playerID << "[C=WHITE] finished researching [C=GREEN]" << upgradeArray[m_PlayerResearches[playerID].ActiveResearchItem].Name << std::endl;
	#endif

	researchItem->Done				= true;
	researchItem->PointsResearched	= pointsToCompletion;
	m_PlayerResearches[playerID].ActiveResearchItem	= RESEARCH_NONE;

	if ( m_PlayerResearches[playerID].ResearchQueue.size() > 0 )
	{
		m_PlayerResearches[playerID].ActiveResearchItem = m_PlayerResearches[playerID].ResearchQueue[0];
		m_PlayerResearches[playerID].ResearchQueue.pop_front();

		#ifdef RESEARCH_DEBUG
			Logger::GetStream() << "RESEARCH_DEBUG defined: [C=BLUE]Player " << playerID << "[C=WHITE] switched to queued research [C=GREEN]" << upgradeArray[m_PlayerResearches[playerID].ActiveResearchItem].Name << std::endl;
		#endif
	}
}

bool SSResearch::ValidPlayer( int playerID )
{
	if ( playerID < 0 || playerID >= g_PlayerData.GetPlayerCount() )
	{
		Logger::Log( "Recieved PlayerID(" + rToString( playerID ) + ") was outside of expected range 0-" + rToString( g_PlayerData.GetPlayerCount() - 1 ), "SSResearch", LogSeverity::WARNING_MSG );
		return false;
	}
	return true;
}

bool SSResearch::ValidIndices( int playerID, UPGRADE_NAME upgrade )
{
	if ( !this->ValidPlayer( playerID ) )
	{
		return false;
	}
	if ( upgrade < 0 || upgrade >= UPGRADE_COUNT )
	{
		Logger::Log( "Recieved Upgrade Index(" + rToString(upgrade) + ") was outside of expected range 0-" + rToString(UPGRADE_COUNT-1), "SSResearch", LogSeverity::WARNING_MSG );
		return false;
	}
	return true;
}

bool SSResearch::ValidToResearch( int playerID, UPGRADE_NAME upgrade )
{
	//already researching this
	if (m_PlayerResearches[playerID].ActiveResearchItem == upgrade)
	{
		return false;
	}
	if ( !ValidIndices( playerID, upgrade ) )
	{
		return false;
	}
	if ( m_PlayerResearches[playerID].ResearchItems[upgrade].Done )
	{
		Logger::Log( "Player " + rToString(playerID) + " has already researched " + g_SSUpgrades.GetUpgradeArray()[upgrade].Name, "SSResearch", LogSeverity::WARNING_MSG );
		return false;
	}
	return true;
}

void SSResearch::CreateVariableName( const rString& in, rString* out )
{
	*out = in;
	for ( int i = 0; i < out->size(); ++i )
	{
		if ( (*out)[i] == ' ' )
		{
			(*out)[i] = '_';
		}
	}
}

void SSResearch::ResetResearchForPlayer( int playerID, UPGRADE_NAME upgrade )
{
	if ( !this->ValidIndices( playerID, upgrade ) )
	{
		return;
	}
	m_PlayerResearches[playerID].ResearchItems[upgrade].Done				= false;
	m_PlayerResearches[playerID].ResearchItems[upgrade].PointsResearched	= 0;
}

void SSResearch::GiveAllResearchToPlayer( int playerID )
{
	if ( !this->ValidPlayer( playerID ) )
	{
		return;
	}
	m_PlayerResearches[playerID].ActiveResearchItem = RESEARCH_NONE;
	m_PlayerResearches[playerID].ResearchQueue.clear();
	for ( int i = 0; i < m_PlayerResearches[playerID].ResearchItems.size(); ++i )
	{
		m_PlayerResearches[playerID].ResearchItems[i].Done = true;
	}
}

void SSResearch::QueueAllResearchForPlayer( int playerID )
{
	if ( !this->ValidPlayer( playerID ) )
	{
		return;
	}

	int		originalQueueSize	= static_cast<int>(m_PlayerResearches[playerID].ResearchQueue.size());
	bool	queueResearch;

	for ( int i = 0; i < UPGRADE_COUNT; ++i )
	{
		queueResearch = true;
		if ( m_PlayerResearches[playerID].ResearchItems[i].Done == true )
		{
			queueResearch = false;
		}
		else if ( m_PlayerResearches[playerID].ActiveResearchItem == i )
		{
			queueResearch = false;
		}
		else
		{
			for ( int j = 0; j < originalQueueSize; ++j )
			{
				if ( m_PlayerResearches[playerID].ResearchQueue[j] == i )
				{
					queueResearch = false;
					break;
				}
			}
		}
		if ( queueResearch )
		{
			this->QueuePlayerResearch( playerID, static_cast<UPGRADE_NAME>( i ) );
		}
	}
}

void SSResearch::ResetAllResearchForPlayer( int playerID )
{
	if ( !this->ValidPlayer( playerID ) )
	{
		return;
	}
	for ( auto& researchItem : m_PlayerResearches[playerID].ResearchItems )
	{
		researchItem.Done				= false;
		researchItem.PointsResearched	= 0;
	}
}

int SSResearch::ScriptSwitchActiveResearch( IScriptEngine* scriptEngine )
{
	int playerID	= scriptEngine->PopInt();
	int upgrade		= scriptEngine->PopInt();

	this->SwitchPlayerActiveResearch( playerID, static_cast<UPGRADE_NAME>( upgrade ) );

	return 0;	// Number of return values.
}

int SSResearch::ScriptQueueResearch( IScriptEngine* scriptEngine )
{
	int playerID	= scriptEngine->PopInt();
	int upgrade		= scriptEngine->PopInt();

	this->QueuePlayerResearch( playerID, static_cast<UPGRADE_NAME>( upgrade ) );

	return 0;	// Number of return values.
}


int SSResearch::ScriptSetResearchSpeed( IScriptEngine* scriptEngine )
{
	float researchSpeed = scriptEngine->PopFloat();

	this->SetTimeBetweenTicks( 1.0f / researchSpeed );

	return 0;	// Number of return values.
}

int SSResearch::ScriptGiveResearch( IScriptEngine* scriptEngine )
{
	int playerID	= scriptEngine->PopInt();
	int upgrade		= scriptEngine->PopInt();

	this->GiveResearchToPlayer( playerID, static_cast<UPGRADE_NAME>( upgrade ) );

	return 0;	// Number of return values.
}

int SSResearch::ScriptResetResearch( IScriptEngine* scriptEngine )
{
	int playerID	= scriptEngine->PopInt();
	int upgrade		= scriptEngine->PopInt();

	this->ResetResearchForPlayer( playerID, static_cast<UPGRADE_NAME>( upgrade ) );

	return 0;	// Number of return values.
}

int SSResearch::ScriptGiveAllResearch( IScriptEngine* scriptEngine )
{
	int playerID	= scriptEngine->PopInt();

	this->GiveAllResearchToPlayer( playerID );

	return 0;	// Number of return values.
}

int SSResearch::ScriptQueueAllResearch( IScriptEngine* scriptEngine )
{
	int playerID	= scriptEngine->PopInt();

	this->QueueAllResearchForPlayer( playerID );

	return 0;	// Number of return values.
}

int SSResearch::ScriptResetAllResearch( IScriptEngine* scriptEngine )
{
	int playerID	= scriptEngine->PopInt();

	this->ResetAllResearchForPlayer( playerID );

	return 0;	// Number of return values.
}
