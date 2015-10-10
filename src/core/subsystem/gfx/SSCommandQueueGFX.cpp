/**************************************************
Copyright 2015 Ola Enberg
***************************************************/

#include "SSCommandQueueGFX.h"

#include <gfx/GraphicsEngine.h>
#include "SSRender.h"
#include "../gamelogic/SSAI.h"
#include "../../utility/PlayerData.h"
#include "../../CompileFlags.h"

SSCommandQueueGFX& SSCommandQueueGFX::GetInstance( )
{
    static SSCommandQueueGFX instance;
    return instance;
}

void SSCommandQueueGFX::Startup( )
{
	m_LineAnimationProgress	= 0.0f;
	m_ShowSquadPath			= false;
	m_ShowUnitPath			= false;

	DEV(
		g_Script.Register( "GE_SetShowSquadPath",	std::bind( &SSCommandQueueGFX::ScriptSetShowSquadPath,	this, std::placeholders::_1 ) );
		g_Script.Register( "GE_SetShowUnitPath",	std::bind( &SSCommandQueueGFX::ScriptSetShowUnitPath,	this, std::placeholders::_1 ) );
	);

    Subsystem::Startup();
}

void SSCommandQueueGFX::Shutdown( )
{
    Subsystem::Shutdown();
}

void SSCommandQueueGFX::UpdateUserLayer( const float deltaTime )
{
	m_LineAnimationProgress += deltaTime;

	for ( auto& squadID : g_PlayerData.GetSelectedSquads() )
	{
		Squad* squad = g_SSAI.GetSquadWithID( g_PlayerData.GetPlayerID(), squadID );

		glm::vec3 segmentStart;
		glm::vec3 segmentEnd;
		if ( m_ShowUnitPath )
		{
			for ( auto& agent : squad->GetAgents() )
			{
				glm::vec3 agentPos = agent->GetWorldPos();
				segmentEnd = glm::vec3( agentPos.x, gfx::g_GFXTerrain.GetHeightAtWorldCoord( agentPos.x, agentPos.z ), agentPos.z );
				for ( int i = static_cast<int>(agent->GetPath().size()) - 1; i >= 0; --i )
				{
					const Goal& wayPoint = agent->GetPath()[i];
					segmentStart	= segmentEnd;
					segmentEnd.x	= static_cast<float>(wayPoint.Tile->X);
					segmentEnd.z	= static_cast<float>(wayPoint.Tile->Y);
					segmentEnd.y	= gfx::g_GFXTerrain.GetHeightAtWorldCoord( segmentEnd.x, segmentEnd.z );
					g_SSRender.RenderLine( segmentStart, segmentEnd, glm::vec4( 0.5f, 0.5f, 0.5f, 1.0f ), true, m_LineAnimationProgress );
				}
			}
		}

		if ( m_ShowSquadPath )
		{
			segmentEnd = glm::vec3( squad->GetPosition().x, gfx::g_GFXTerrain.GetHeightAtWorldCoord( squad->GetPosition().x, squad->GetPosition().y ), squad->GetPosition().y );
			for ( int i = static_cast<int>(squad->GetPath().size()) - 1; i >= 0; --i )
			{
				const Goal& wayPoint = squad->GetPath()[i];
				segmentStart	= segmentEnd;
				segmentEnd.x	= static_cast<float>(wayPoint.Tile->X);
				segmentEnd.z	= static_cast<float>(wayPoint.Tile->Y);
				segmentEnd.y	= gfx::g_GFXTerrain.GetHeightAtWorldCoord( segmentEnd.x, segmentEnd.z );
				g_SSRender.RenderLine( segmentStart, segmentEnd, glm::vec4( 1.0f ), true, m_LineAnimationProgress );
			}
		}

		if ( squad->GetMissions().size() <= 1 )
		{
			continue;
		}

		glm::vec3 lineStart;
		glm::vec3 lineEnd( squad->GetPosition().x, gfx::g_GFXTerrain.GetHeightAtWorldCoord( squad->GetPosition().x, squad->GetPosition().y ), squad->GetPosition().y );
		glm::vec4 colour( 1.0f, 1.0f, 0.0f, 1.0f );

		for ( int i = 0; i < squad->GetMissions().size(); ++i )
		{
			const Squad::Mission& mission = squad->GetMissions()[i];

			if ( mission.Type == Squad::MissionType::MISSION_TYPE_HOLD )
				continue;

			lineStart = lineEnd;

			glm::vec2 missionGoal( static_cast<float>(mission.Goal->X), static_cast<float>(mission.Goal->Y) );
			lineEnd = glm::vec3( missionGoal.x, gfx::g_GFXTerrain.GetHeightAtWorldCoord( missionGoal.x, missionGoal.y ), missionGoal.y );

			switch ( mission.Type )
			{
				case Squad::MissionType::MISSION_TYPE_MOVE:
					colour = glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f );
					break;
				case Squad::MissionType::MISSION_TYPE_MINE:
					colour = glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );
					break;
				case Squad::MissionType::MISSION_TYPE_ATTACK:
					colour = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
					break;
				case Squad::MissionType::MISSION_TYPE_ATTACK_MOVE:
					colour = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
					break;
				default:
					colour = glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f );
					break;
			}

			if ( i == 0 )
				colour *= 0.6f;

			g_SSRender.RenderLine( lineStart, lineEnd, colour, true, m_LineAnimationProgress );
		}
	}
}

int SSCommandQueueGFX::ScriptSetShowSquadPath( IScriptEngine* scriptEngine )
{
	bool activation = scriptEngine->PopBool();

	m_ShowSquadPath = activation;

	return 0; // Number of return values.
}

int SSCommandQueueGFX::ScriptSetShowUnitPath( IScriptEngine* scriptEngine )
{
	bool activation = scriptEngine->PopBool();

	m_ShowUnitPath = activation;

	return 0; // Number of return values.
}