/**************************************************
Viktor Kelkkanen & Ola Enberg
***************************************************/

#include "Squad.h"
#include "Terrain.h"
#include <utility/Logger.h>
#include <float.h>
#include <glm/gtx/vector_angle.hpp>
#include <gfx/GraphicsEngine.h>
#include <messaging/GameMessages.h>
#include "../component/ResourceComponent.h"
#include "../subsystem/audio/SSAudio.h"
#include "../subsystem/gamelogic/SSUpgrades.h"
#include "../subsystem/gamelogic/SSAI.h"
#include "../ai/PathFinder.h"
#include "../utility/Alliances.h"
#include "../utility/PlayerData.h"
#include "../subsystem/gui/SSGUIInfo.h"
#include "core/subsystem/utility/SSMail.h"

struct ResourceInfo
{
	Entity		EntityID;
	Tile*		Tile;
	int			Miners		= 0;

	ResourceInfo( Entity entityID, struct Tile* tile ) { this->EntityID = entityID; this->Tile = tile; }
};

Squad::Squad(int _team, int _id)
{
	m_Team					= _team;
	m_ID					= _id;

	m_Goal					= nullptr;

	m_Forward				= glm::vec2( 0.0f );
	m_FormationForward		= m_Forward;
	m_Target				= glm::vec3( 0.0f );
	m_Position				= glm::vec2( 0.0f );
	m_MinPos				= glm::vec3( 0.0f );
	m_MaxPos				= glm::vec3( 0.0f );

	m_Formation				= FORMATION::FORMATION_COLUMN;

	m_AttackMode			= false;
	m_UseAutoAttack			= true;
	m_UseHoldPosition		= false;
	m_IsUnderAttack			= false;
	m_HasMoved				= false;
	m_CanMine				= true;

	m_AttackRepathTimer		= 0.0f;
	m_UpgradeTimer			= 0.0f;
	m_UpgradeParticleSystem	= nullptr;

	m_SpacingStep			= 2;
	m_Target.x				= 0;
	m_Target.y				= 0;
	m_Forward.x				= 0;
	m_Forward.y				= 1;
	m_Resources				= 0;
	m_TargetSquad			= -1;
	m_TargetTeam			= -1;
	m_Radius				= 1;
	m_NextGoalTag			= 0;

	m_FormationHedgehogSpacing[0]	= SQUAD_HEDGEHOG_SPACING_1;
	m_FormationHedgehogSpacing[1]	= SQUAD_HEDGEHOG_SPACING_2;
	m_FormationHedgehogSpacing[2]	= SQUAD_HEDGEHOG_SPACING_3;

	m_FormationPocketSpacing[0]		= SQUAD_POCKET_SPACING_1;
	m_FormationPocketSpacing[1]		= SQUAD_POCKET_SPACING_2;
	m_FormationPocketSpacing[2]		= SQUAD_POCKET_SPACING_3;

	//m_AIDrawOnScreen		= AIDrawOnScreen();

}

Squad::~Squad()
{
}

void Squad::AddAgent( Agent* _a )
{
	if(m_Agents.empty())
	{
		m_Position					= _a->GetPos();
		m_MissionProgressPosition	= _a->GetPos();
	}

	m_Agents.push_back( _a );
	m_Agents.back()->SetSquadID(m_ID);
	_a->ClearPath();
	FindBuddies();
}

Agent* Squad::GetLeader()
{
	if (m_Agents.size() == 0)
	{
		Logger::Log("GetLeader on empty squad returned nullptr", "SSAI", LogSeverity::WARNING_MSG);
		return nullptr;
	}

	unsigned int safe = 0;
	for (int i = 0; i < m_Agents.size();i++)
	{
		if (m_Agents[i] != nullptr)
		{
			safe = i;
			if(!GetDenseComponent<HealthComponent>(m_Agents[i]->GetEntityID())->IsDead)
				return m_Agents[i];
		}
	}

	Logger::Log("GetLeader returned agent that may not be alive", "SSAI", LogSeverity::WARNING_MSG);

	return m_Agents[safe];
}

void Squad::Update(float dt)
{
	if (m_Agents.size() == 0)
		return;

	if (m_Missions.size() > 0)
	{
		m_HasMoved = true;

		if (m_Missions[0].Progress == MissionState::START)
			(this->*m_Missions[0].Start)();
		else if (m_Missions[0].Progress == MissionState::RUN)
			(this->*m_Missions[0].Run)(dt);
		else if (m_Missions[0].Progress == MissionState::END)
			(this->*m_Missions[0].End)();
		else
			MissionAccomplished();
	}
	else if (GetLeader()->IsIdle() && !m_UseAutoAttack && !m_UseHoldPosition)
		UseAutoAttack(true, false);


	m_Position = glm::vec2( 0.0f );

	for ( auto& agent : m_Agents )
	{
		m_Position += agent->GetPos();
	}
	m_Position /= static_cast<float>( m_Agents.size() );
}

void Squad::CalculateFormationPositions( rVector<glm::vec2>& outPosition, const glm::vec2& forward, int allowedCost )
{
	outPosition.clear();

	if ( m_Formation == FORMATION::FORMATION_COLUMN )
		this->ColumnFormation( outPosition, allowedCost );
	else if ( m_Formation == FORMATION::FORMATION_HEDGEHOG )
		this->HedgeHogFormation( outPosition, allowedCost );
	else if ( m_Formation == FORMATION::FORMATION_LINE )
		this->LineFormation( outPosition, allowedCost );
	else if ( m_Formation == FORMATION::FORMATION_LINE_OFFENSIVE )
		this->LineFormationOffensive( outPosition, allowedCost );
	//else if ( m_Formation == Formation::BREAK )
	//	this->BreakFormation( outPosition );
	else
	{
		for ( auto& agent : m_Agents )
		{
			outPosition.push_back( agent->GetPos() );
		}
		return;
	}

	const glm::vec2	right = glm::vec2( -forward.y, forward.x );

	for ( auto& formationPosition : outPosition )
	{
		formationPosition = (formationPosition.x * right) + (formationPosition.y * forward);
	}
}

// Agent* Squad::GetAgentWithID(int _id) const
// {
// 	for (auto & agent : m_Agents)
// 	{
// 		if (agent->GetID() == _id)
// 		{
// 			return agent;
// 		}
// 	}
// 	return nullptr;
// }

void Squad::SetFormation(FORMATION _form)
{
	if ( _form != m_Formation )
	{
		m_Formation = _form;

		if ( !m_Agents.empty() )
		{
			// TODOOE: Do we still need to find buddies when chaning formation?
			FindBuddies();
			this->RepathFormation();
		}
	}
}

void Squad::SetSpacingStep(int val)
{
	if ( val != m_SpacingStep )
	{
		m_SpacingStep = val;

		if ( !m_Agents.empty() )
			this->RepathFormation();
	}
}

void Squad::AddSpacingStep( int _step )
{
	int newSpacingStep = m_SpacingStep + _step;
	newSpacingStep = glm::max( newSpacingStep, 1 );
	newSpacingStep = glm::min( newSpacingStep, SQUAD_SPACING_NR_OF_STEPS );
	SetSpacingStep(newSpacingStep);
}

void Squad::FindBuddies()
{
	//	Buddy System
	//
	//		<-[1]<-[3]<-[5]
	//
	//	 [0]<-[2]<-[4]<-[6]
	//
	//	leader[0] has no buddy, co-leader[1] and [2] are buddies with the leader.
	//	co-leader can form his own columns in some formations

	for (unsigned int i = 0; i < m_Agents.size(); i++)
	{
		bool unEven = (i % 2) != 0;
		m_Agents[i]->SetBuddy(nullptr);
		if (i > 1)
			m_Agents[i]->SetBuddy(m_Agents[i - 2]);
	}

	if (m_UseAutoAttack)
		m_Agents.back()->AddBehaviour(Agent::BehaviourType::AUTO_ATTACK);

	m_Agents.back()->AddBehaviour(Agent::BehaviourType::FIND_GOALS);

	if (m_Missions.size() > 0)
	{
		if (m_Missions[0].Type == Squad::MissionType::MISSION_TYPE_MINE)
		{
			m_Agents.back()->AddBehaviour(Agent::BehaviourType::MINING);
			m_Agents.back()->SetTarget(Terrain::GetInstance()->GetClosestResource(m_Agents.back()->GetTile(), 20));
		}
	}

	if (m_Agents.size() > 0)
	{
		if (m_Agents.size() > 1)
		{
			m_Agents[1]->SetBuddy(m_Agents[0]);
		}

		m_Goal = m_Agents[0]->GetTile();
	}


}

void Squad::ColumnFormation( rVector<glm::vec2>& outRelativePosition, int allowedCost )
{
	const int defaultSpacing = m_SpacingStep * AGENT_SIZE_IN_TILES;
	int	unitsPerRow	= static_cast<int>( sqrtf( static_cast<float>( m_Agents.size() ) ) );
	int	spacing		=  unitsPerRow <= 1 ? defaultSpacing : glm::min( defaultSpacing, (allowedCost - AGENT_SIZE_IN_TILES) / (unitsPerRow - 1) );

	if ( spacing < SQUAD_SPACING_DYNAMIC_MINI )
	{
		spacing		= SQUAD_SPACING_DYNAMIC_MINI;
		unitsPerRow	= glm::max( 
			1 + (allowedCost - AGENT_SIZE_IN_TILES) / SQUAD_SPACING_DYNAMIC_MINI, 
			static_cast<int>( sqrtf( static_cast<float>(m_Agents.size()) / 2 ) ) );

	}
	this->ColumnFormationSpecified( outRelativePosition, unitsPerRow, glm::ivec2( spacing, defaultSpacing ) );
}

void Squad::ColumnFormationSpecified( rVector<glm::vec2>& outRelativePosition, int unitsPerRow, const glm::ivec2& spacing )
{
	const glm::vec2	halfFormationSize	= (0.5f * spacing.x) * glm::vec2( unitsPerRow, m_Agents.size() / unitsPerRow );

	glm::vec2 formationPosition;
	for ( int i = 0; i < m_Agents.size(); ++i )
	{
		formationPosition.x = ((i % unitsPerRow) - (-0.5f + (0.5f * unitsPerRow)))						*  spacing.x;
		formationPosition.y = ((i / unitsPerRow) - (-0.5f + (0.5f * m_Agents.size() / unitsPerRow)))	* -spacing.y;
		outRelativePosition.push_back( formationPosition );
	}
}

void Squad::HedgeHogFormation( rVector<glm::vec2>& outRelativePosition, int allowedCost )
{
	int agentsLeft	= static_cast<int>(m_Agents.size());
	int agentIndex	= 0;
	int radius		= m_FormationHedgehogSpacing[ m_SpacingStep - 1 ];

	while ( agentsLeft > 0 )
	{
		const float	circumference	= static_cast<float>(radius) * 2.0f * PI;
		const int	agentsInCircle	= glm::min( agentsLeft, static_cast<int>( circumference / AGENT_SIZE_IN_TILES ) );

		const float angleSlice		= 360.0f / agentsInCircle;
		float		angle			= 0.0f;

		glm::vec2	position		= glm::vec2( 0.0f );

		for ( int i = 0; i < agentsInCircle; ++i )
		{
			const float angle = i * angleSlice;
	
			position.x = -(float)cos(angle * PI / 180.0f) * radius;
			position.y = (float)sin(angle * PI / 180.0f) * radius;
			outRelativePosition.push_back( position );
		}
		
		agentsLeft	-= agentsInCircle;
		agentIndex	+= agentsInCircle;
		radius		+= AGENT_SIZE_IN_TILES;
	}
}

void Squad::LineFormation( rVector<glm::vec2>& outRelativePosition, int allowedCost )
{
	const int defaultSpacing = m_SpacingStep * AGENT_SIZE_IN_TILES;

	//int spacing = glm::min( defaultSpacing, (allowedCost - AGENT_SIZE_IN_TILES) / static_cast<int>(m_Agents.size()) );
	//if ( spacing < SQUAD_SPACING_DYNAMIC_MINI )
	//{
	//	spacing = SQUAD_SPACING_DYNAMIC_MINI;
	//	int unitsPerRow = glm::max( SQUAD_LINEWIDTH_DYNAMIC_MINI, (allowedCost - AGENT_SIZE_IN_TILES) / spacing );
	//	this->ColumnFormationSpecified( outRelativePosition, unitsPerRow, glm::ivec2( spacing ) );
	//	return;
	//}

	glm::vec2 position = glm::vec2( -0.5f * defaultSpacing * (static_cast<int>(m_Agents.size()) - 0.5f ), 0.0f );

	for ( auto& agent : m_Agents )
	{
		outRelativePosition.push_back( position );
		position.x += defaultSpacing;
	}
}

void Squad::LineFormationOffensive( rVector<glm::vec2>& outRelativePosition, int allowedCost )
{
	int agentsLeft	= static_cast<int>(m_Agents.size());
	int agentIndex	= 0;
	int radius		= m_FormationPocketSpacing[ m_SpacingStep - 1 ];

	while ( agentsLeft > 0 )
	{
		const float	circumference	= static_cast<float>(radius) * PI;
		const int	agentsInCircle	= glm::min( agentsLeft, static_cast<int>( circumference / AGENT_SIZE_IN_TILES ) );

		const float angleSlice		= 180.0f / agentsInCircle;
		float		angle			= 0.0f;

		glm::vec2	position		= glm::vec2( 0.0f );

		for ( int i = 0; i < agentsInCircle; ++i )
		{
			const float angle = i * angleSlice;
	
			position.x = -(float)cos(angle * PI / 180.0f) * radius;
			position.y = -(float)sin(angle * PI / 180.0f) * radius;
			outRelativePosition.push_back( position );
		}
		
		agentsLeft	-= agentsInCircle;
		agentIndex	+= agentsInCircle;
		radius		+= AGENT_SIZE_IN_TILES;
	}
}

// glm::vec3 Squad::DrawnFormation(Agent* _agent)
// {
// 	m_AIDrawOnScreen.SetAmountOfAgents(static_cast<int>(m_Agents.size()));
// 	float space = m_AIDrawOnScreen.GetSpacing();
// 	glm::vec2 front = m_AIDrawOnScreen.GetFront();
// 	glm::vec2 pos = m_AIDrawOnScreen.GetPos(_agent->GetID());
// 
// 	pos.x = GetLeader()->GetX() + (front.y * space * pos.x);
// 	pos.y = GetLeader()->GetY() - (front.x * space * pos.y);
// 
// 	return glm::vec3(pos.x,pos.y,0);
// }

//tries to solve agents not finding a slot in the formation
//glm::vec3 Squad::BreakFormation(Agent* _agent)
//{
//	glm::vec3 pos = glm::vec3(0, 0, 0);
//
//	//buddy killed?
//	if ((*_agent).GetBuddy() == nullptr)
//	{
//		FindBuddies();
//		return FormationToAgent(_agent);
//	}
//	else
//	{
//		//attempt to solve by decreased spacing
//		if (!m_NewOrdersThisFrame)
//		{
//			int space = _agent->GetSpacing();
//			if (space > 3)
//			{
//				_agent->SetSpacing(space - 1);
//				_agent->SetFormation(m_Formation);
//			}
//		}
//
//		//attempt lower space and column formation
//
//		pos = ColumnFormation(_agent, -1);
//
//		Tile* t = Terrain::GetInstance()->GetTile(pos.x, pos.y);
//
//		Terrain* terrain = Terrain::GetInstance();
//
//		//invalid tile
//		if (terrain->OutOfBounds(pos.x, pos.y) || terrain->IsUnpathableTerrain(t) || terrain->IsOccupiedByUnitExclude(t, _agent->GetEntityID()))
//		{
//			pos = ColumnFormation(_agent, 1);
//
//			//no solution
//			if (terrain->OutOfBounds(pos.x, pos.y) || terrain->IsUnpathableTerrain(t) || terrain->IsOccupiedByUnitExclude(t, _agent->GetEntityID()))
//			{
//// 				Tile* t = terrain->GetMovableTile((float)pos.x, (float)pos.y, _agent->GetEntityID());
//// 				pos.x = (float)t->X;
//// 				pos.y = (float)t->Y;
//				OrderFormation();
//			}
//			else
//				_agent->SetFormation(Formation::COLUMN);
//		}
//		else
//			_agent->SetFormation(Formation::COLUMN);
//	}
//	return pos;
//}

// glm::vec3 Squad::ShiftFormation(Agent* _agent)
// {
// 	glm::vec3 pos = glm::vec3(0, 0, 0);
// 
// 	int partsSpace = 1;
// 
// 	//coleader stays with his group until leader has moved
// 	if (_agent->IsCoLeader())
// 	{
// 		partsSpace = 3;
// 		//stay if leader is not idle
// 		if (!_agent->GetBuddy()->IsIdle() || m_newOrdersThisFrame)
// 		{
// 			return _agent->GetPos();
// 		}
// 	}
// 
// 	return pos;
// }

int Squad::CalculateFormationWidthInTiles( FORMATION formation, int spacingStep )
{
	if ( m_Formation == FORMATION::FORMATION_COLUMN )
		return this->CalcColumnWidthInTiles( spacingStep );
	else if ( m_Formation == FORMATION::FORMATION_HEDGEHOG )
		return this->CalcHedgeHogWidthInTiles( spacingStep );
	else if ( m_Formation == FORMATION::FORMATION_LINE )
		return this->CalcLineWidthInTiles( spacingStep );
	else if ( m_Formation == FORMATION::FORMATION_LINE_OFFENSIVE )
		return this->CalcLineOffensiveWidthInTiles( spacingStep );
	else
		return 9;
}

int Squad::CalcColumnWidthInTiles( int spacingStep )
{
	const int spacing = spacingStep * AGENT_SIZE_IN_TILES;
	const int unitsPerRow = static_cast<int>( sqrtf( static_cast<float>( m_Agents.size() ) ) );
	return AGENT_SIZE_IN_TILES + ( spacing * (unitsPerRow - 1) );
}

int Squad::CalcHedgeHogWidthInTiles( int spacingStep )
{
	return AGENT_SIZE_IN_TILES + 2 * m_FormationHedgehogSpacing[ m_SpacingStep - 1 ];
}

int Squad::CalcLineWidthInTiles( int spacingStep )
{
	return 1;
}

int Squad::CalcLineOffensiveWidthInTiles( int spacingStep )
{
	return AGENT_SIZE_IN_TILES + 2 * m_FormationPocketSpacing[ m_SpacingStep - 1 ];
}

void Squad::CalculateFormationAssignments( const rVector<glm::vec2>& agentPositions, const rVector<glm::vec2>& formationPositions, const glm::vec2& offset, rVector<int>& outAssignments )
{
	outAssignments.clear();

	rVector<Entity> agentsToPlaceInFormation;
	for ( int i = 0; i < agentPositions.size(); ++i )
	{
		agentsToPlaceInFormation.push_back( i );
	}

	for ( auto& formationPosition : formationPositions )
	{
		int closestAgentIndex		= 0;
		float closestDistanceSqrd	= FLT_MAX;

		for ( int j = 0; j < agentsToPlaceInFormation.size(); ++j )
		{
			const glm::vec2& pos	= agentPositions[agentsToPlaceInFormation[j]];
			float distanceSqrd		= glm::length2( pos - (offset + formationPosition) );

			if ( distanceSqrd < closestDistanceSqrd )
			{
				closestAgentIndex	= j;
				closestDistanceSqrd	= distanceSqrd;
			}
		}

		outAssignments.push_back( agentsToPlaceInFormation[closestAgentIndex] );
		agentsToPlaceInFormation.erase( agentsToPlaceInFormation.begin() + closestAgentIndex );
	}
}

void Squad::CycleFormation()
{
	FORMATION form = m_Formation;

	form = FORMATION((unsigned int)m_Formation + 1);

	if (form >= FORMATION::FORMATION_MAX)
		form = FORMATION((unsigned int)FORMATION::FORMATION_NONE + 1);

	SetFormation(form);
}

void Squad::FormationGoals(float dt)
{
	if (m_UseAutoAttack)
	{
		bool respondToAttack = false;
		Entity target = ENTITY_INVALID;

		for (auto & agent : m_Agents)
		{
			target = agent->GetTarget();
			if (GetAgentPointer(target) != nullptr)
			{
				respondToAttack = true;
				break;
			}
		}

		if (respondToAttack)
		{
			Agent* enemy = GetAgentPointer(target);
			Tile* targetTile = nullptr;

			if (enemy != nullptr)
				targetTile = enemy->GetTile();

			for (auto & agent : m_Agents)
			{
				agent->ClearPath();
				if (targetTile != nullptr)
					agent->AddGoal(targetTile);
			}

			Tile* oldGoal = m_Goal;
			SetTargetSquad(target);
			AddMission(targetTile, MISSION_TYPE_ATTACK, true);
			AddMission(oldGoal, MISSION_TYPE_MOVE, false);
			return;
		}
	}

	if ( m_Path.empty() )
	{
		return;
	}

	Tile* goalTile						= m_Path.back().Tile;
	const glm::vec2 goalPosition		= glm::vec2(goalTile->X, goalTile->Y);
	const glm::vec2 distanceToGoal		= goalPosition - m_MissionProgressPosition;
	const float		metersLeftToGoal	= glm::length( distanceToGoal );
	
	if ( metersLeftToGoal > 0.01f )
	{
		m_Forward					= distanceToGoal / metersLeftToGoal;
		const float movement		= glm::min( metersLeftToGoal, m_Agents[0]->GetSpeed() * dt );
		m_MissionProgressPosition	+= movement * m_Forward;
	}
	else
	{
		m_Path.pop_back();
		if ( m_Path.empty() )
		{
			return;
		}
	}

	for ( auto& agent : m_Agents )
	{
		agent->KeepDistance( metersLeftToGoal, m_Path.back().Tag );
	}
}


void Squad::BuildSquadPath()
{
	Tile*		startTile	= Terrain::GetInstance()->GetMovableTile(m_Position.x, m_Position.y, ENTITY_INVALID, Terrain::GetInstance()->GetTile( m_Position.x, m_Position.y ));
	if ( Terrain::GetInstance()->IsUnpathableTerrain( startTile ) )
	{
		startTile = this->GetLeader()->GetTile();
	}
	m_Position				= glm::vec2( startTile->X, startTile->Y );
	Pathfinder* pathFinder	= Pathfinder::GetInstance();

	pathFinder->SetStart(startTile);
	pathFinder->SetGoal(m_Goal);

	const int allowedCost = 1 + (this->CalculateFormationWidthInTiles( m_Formation, m_SpacingStep ) / 2);
	pathFinder->SetStartFloat2( m_Position );
	m_Path = pathFinder->AStar(startTile, false, allowedCost,true);

	BuildFormationPath();
}

void Squad::BuildFormationPath()
{
	for ( auto& agent : m_Agents )
	{
		agent->ClearPath();
	}

	rVector<glm::vec2> agentPositions;
	agentPositions.reserve( m_Agents.size() );
	for ( auto& agent : m_Agents )
	{
		agentPositions.push_back( agent->GetPos() );
	}

	int nextRepath = static_cast<int>(m_Path.size()) - 1;
	for ( int i = static_cast<int>(m_Path.size()) - 1; i >= 0; --i )
	{
						m_Path[i].Tag		= m_NextGoalTag;
		const Tile*		goalTile			= m_Path[i].Tile;
		const glm::vec2 goalPosition		= glm::vec2(goalTile->X, goalTile->Y);

		const Tile*		previousTile		= ((i < static_cast<int>(m_Path.size()) - 1) ) ? m_Path[i+1].Tile : Terrain::GetInstance()->GetTile( m_Position.x, m_Position.y );
		const glm::vec2 previousPosition	= glm::vec2(previousTile->X, previousTile->Y);

		const glm::vec2 distanceToGoal		= goalPosition - previousPosition;
		const float		metersLeftToGoal	= glm::length( distanceToGoal );

		const glm::vec2 forward				= (metersLeftToGoal > 0.0f) ? (distanceToGoal / metersLeftToGoal) : m_Forward;
		const glm::vec2 basePosition		= goalPosition;

		const float metersToGetInFormation	= 10.0f;

		const bool agentAmountChanged		= m_FormationAssignments.size() != m_Agents.size();

		bool		repath				= false;
		glm::vec2	formationForward	= forward;
		if ( i <= nextRepath )
		{
			repath = true;

			float		distance	= 0.0f;
			glm::vec2	prevPos		= previousPosition;
			glm::vec2	nextPos		= previousPosition;

			nextRepath = i;
			for ( int j = i; j >= 0; --j )
			{
				nextPos		= glm::vec2(m_Path[j].Tile->X, m_Path[j].Tile->Y);
				distance	+= glm::length(nextPos - prevPos);
				prevPos		= nextPos;

				if ( distance >= metersToGetInFormation )
				{
					nextRepath = j;
					break;
				}
			}
			if ( nextPos != previousPosition )
				formationForward = glm::normalize(nextPos - previousPosition);
		}

		if ( repath || agentAmountChanged )
		{
			this->CalculateFormationPositions( m_FormationPositions, formationForward, goalTile->Cost );
			if ( agentAmountChanged || glm::angle( formationForward, m_FormationForward ) > 1.2f )
			{
				this->CalculateFormationAssignments( agentPositions, m_FormationPositions, basePosition, m_FormationAssignments );
			}
			m_FormationForward = formationForward;
		}

		if ( metersToGetInFormation < metersLeftToGoal )
		{
			for ( int j = 0; j < m_FormationAssignments.size(); ++j )
			{
				Agent*		closestAgent		= m_Agents[ m_FormationAssignments[j] ];
				glm::vec2	agentGoalPosition	= (previousPosition + metersToGetInFormation * forward)+  m_FormationPositions[j];
				Tile* t = Terrain::GetInstance()->GetMovableTile(agentGoalPosition.x, agentGoalPosition.y, closestAgent->GetEntityID(), Terrain::GetInstance()->GetTile(agentGoalPosition.x, agentGoalPosition.y));
				//t->DebugInfo = 12;
				closestAgent->AddGoal( t );
			}
		}

		for ( int j = 0; j < m_FormationAssignments.size(); ++j )
		{
			agentPositions[m_FormationAssignments[j]] = basePosition + m_FormationPositions[j];
		}

 		for ( int j = 0; j < agentPositions.size(); ++j )
 		{
 			Tile* t = Terrain::GetInstance()->GetMovableTile(agentPositions[j].x, agentPositions[j].y, m_Agents[j]->GetEntityID(), Terrain::GetInstance()->GetTile(agentPositions[j].x, agentPositions[j].y));
 			m_Agents[j]->AddGoal( Goal( t, m_Path[i].Tag ) );
 			//t->DebugInfo = 11;
 		}

		++m_NextGoalTag;
	}
}

void Squad::RepathFormation()
{
	m_MissionProgressPosition = m_Position;
	if ( this->HasMission() )
	{
		m_Missions[0] = Mission(m_Missions[0].Type, m_Missions[0].Goal, ENTITY_INVALID);
	}
	else
	{
		m_Goal = Terrain::GetInstance()->GetTile( m_Position.x, m_Position.y );
		this->BuildSquadPath();
		m_Missions.push_back( Mission( MISSION_TYPE_MOVE, m_Goal,ENTITY_INVALID ) );
	}
}

void Squad::SpawnUnit(glm::vec3 _host)
{
	m_UnitsToSpawnFrom.push_back(_host);
}

void Squad::PopHostUnit()
{
	m_UnitsToSpawnFrom.pop_back();
}

rVector<Agent*> Squad::SplitSquad()
{
	int squadSize = static_cast<int>(m_Agents.size());
	rVector<Agent*> outAgents;

	for (int i = squadSize -1; i >= static_cast<int>(squadSize*0.5f); i--)
	{
		outAgents.push_back(m_Agents[i]);
		m_Agents.pop_back();
	}

	FindBuddies();

	return outAgents;
}

void Squad::DeleteAgent(Agent* _a)
{
	int agentToRemove = -1;
	for (int i = 0; i < m_Agents.size(); i++)
	{
		if (m_Agents[i]->GetBuddy() == _a)
		{
			m_Agents[i]->SetBuddy(nullptr);
		}
		if (m_Agents[i] == _a)
		{
			agentToRemove = i;
		}
	}
	if (agentToRemove > -1)
	{
		PlaySFX("../../../asset/audio/collection/robot/robot_death.sfxc", m_Agents[agentToRemove]->GetWorldPos(), 100.0f, 300.0f);
		PlaySFX("../../../asset/audio/collection/misc/explosion.sfxc", m_Agents[agentToRemove]->GetWorldPos(), 100.0f, 300.0f);
		tDelete( m_Agents[agentToRemove] );
		m_Agents.erase(m_Agents.begin() + agentToRemove);
	}
	else
	{
		Logger::Log("Agent to delete not found in supposed squad", "SSAI", LogSeverity::WARNING_MSG);
	}

	if (m_Agents.size() == 0)
	{
		PopAllMissions();
		return;
	}

	FindBuddies();
}

bool Squad::AllIdle() const
{
	for (auto& agent : m_Agents)
	{
		if (!agent->IsIdle())
			return false;
	}
	return true;
}

void Squad::ProgressMission()
{
	m_Missions[0].Progress = Squad::MissionState( ( unsigned int )m_Missions[0].Progress + 1 );
}

void Squad::AddMission( Tile* _t, MissionType _m, bool _override )
{
	if (m_Agents.size() == 0)
		return;

	if (HasMission())
	{
		if (_t == m_Goal && _m == m_Missions[0].Type)
			return;
	}

	if ( _override && !m_Missions.empty() && m_Missions[0].Type == MISSION_TYPE_UPGRADE )
	{
		this->PopQueuedMissions();
		_override = false;
	}

	if (_t == nullptr)
		_t = GetLeader()->GetTile();

	Terrain* terrain = Terrain::GetInstance();

	Entity target = ENTITY_INVALID;

	if (_m == MissionType::MISSION_TYPE_FOCUS_FIRE)
	{
		Entity e = terrain->WhoIsOnTile(_t);
		Agent* a = GetAgentPointer(e);

		if (a != nullptr)
		{
			if (!g_Alliances.IsAllied(m_Team, a->GetTeam()))
			{
				target = e;
			}
		}
	}

	if (_override)
		PopAllMissions();

	Tile* t = terrain->GetTile(m_Position.x, m_Position.y);//GetLeader()->GetTile();

	float dx = static_cast<float>(_t->X - t->X);
	float dy = static_cast<float>(_t->Y - t->Y);

	if (dx != 0 || dy != 0)
	{
		float lngth = sqrt(dx*dx + dy*dy);

		float ldx = dx / lngth;
		float ldy = dy / lngth;

		int multiplyLim = 15;

		dx = std::abs(dx);
		dy = std::abs(dy);

		if (dx > multiplyLim)
			dx = static_cast<float>(multiplyLim);
		if (dy > multiplyLim)
			dy = static_cast<float>(multiplyLim);


		t = terrain->GetTile(m_Position.x + ldx * dx, m_Position.y + ldy * dy);

		if (!terrain->IsUnpathableTerrain(t))
		{
			m_Position.x = static_cast<float>(t->X);
			m_Position.y = static_cast<float>(t->Y);
		}
	}

	m_Missions.push_back( Mission( _m, _t, target ) );
}

void Squad::MoveMissionStart()
{
	if (m_Missions[0].Type == MissionType::MISSION_TYPE_MOVE || m_Missions[0].Type == MissionType::MISSION_TYPE_MINE)
	{
		UseAutoAttack(false, false);
	}
	else if (m_Missions[0].Type == MissionType::MISSION_TYPE_ATTACK_MOVE)
	{
		UseAutoAttack(true, false);
	}

	m_Goal = m_Missions[0].Goal;
	this->BuildSquadPath();
	
	UseHoldPosition(false);

	AllAgentsAddBehaviour(Agent::BehaviourType::FIND_GOALS);

	SetTarget(ENTITY_INVALID);
	ProgressMission();
}

void Squad::AllAgentsAddBehaviour(Agent::BehaviourType _behaviour)
{
	for (auto& agent : m_Agents)
	{
		agent->AddBehaviour(_behaviour);
	}
}

void Squad::AllAgentsPopBehaviour(Agent::BehaviourType _behaviour)
{
	for (auto& agent : m_Agents)
	{
		agent->PopBehaviour(_behaviour);
	}
}

void Squad::PopAllMissions()
{
	m_Missions.clear();
	//UseAutoAttack(false,false);
	
	for (int i = 0; i < m_Agents.size(); i++)
	{
		m_Agents[i]->PopBehaviour(Agent::BehaviourType::MINING);
		m_Agents[i]->PopBehaviour(Agent::BehaviourType::FIND_GOALS);
		m_Agents[i]->PopBehaviour(Agent::BehaviourType::NO_CLIP);
		//m_Agents[i]->PopBehaviour(Agent::BehaviourType::AUTO_ATTACK);
	}
}

void Squad::PopQueuedMissions()
{
	if ( m_Missions.size() < 2 )
		return;

	m_Missions.erase( m_Missions.begin() + 1, m_Missions.end() );
}

void Squad::MoveMissionRun(float dt)
{
	FormationGoals(dt);

	if (m_Path.empty())
		ProgressMission();
}

bool Squad::CheckCanMine()
{
	bool shouldMine = m_Agents.size() < SQUAD_MAXIMUM_UNIT_COUNT;
	if (!shouldMine)
	{
		for (auto& agent : m_Agents)
		{
			HealthComponent* healthComponent = GetDenseComponent<HealthComponent>(agent->GetEntityID());
			if (healthComponent->Health < healthComponent->MaxHealth)
			{
				shouldMine = true;
				break;
			}
		}
	}

	m_CanMine = shouldMine;

	return shouldMine;
}

void Squad::MiningMissionRun(float dt)
{
	FormationGoals(dt);

	if ( m_Path.size() > 0 )
	{
		return;
	}
	
	for (int i = 0; i < m_Agents.size(); i++)
	{
		m_Agents[i]->ClearPath();
		m_Agents[i]->AddBehaviour(Agent::BehaviourType::MINING);
	}

	ProgressMission();
}

void Squad::StandardMissionEnd()
{
	if ( m_Missions.size() == 1 )
	{
		Tile* squadTile = Terrain::GetInstance()->GetTile( m_MissionProgressPosition.x, m_MissionProgressPosition.y );

		if ( m_Missions[0].Type != MISSION_TYPE_MOVE || m_Missions[0].Goal != squadTile )
		{
			m_Goal = squadTile;
			this->BuildSquadPath();
			m_Missions.insert(m_Missions.begin() + 1, Mission(MISSION_TYPE_MOVE, m_Goal, ENTITY_INVALID));
		}
	}

	ProgressMission();
}

void Squad::MiningMissionEnd()
{
	bool keepMining = true;
	int miners = 0;

	rVector<Entity> closest = Terrain::GetInstance()->GetResourcesInRadius(Terrain::GetInstance()->GetTile(m_Position.x, m_Position.y), 25);
	int index = 0;
	int loops = 0;
	if (closest.size() > 0)
	{
		int resourcesCollected = 0;

		for (int i = 0; i < m_Agents.size();i++)
		{

			if (loops < SQUAD_MINERS_PER_RESOURCE)
			{
				if (m_Agents[i]->GetTarget() == ENTITY_INVALID)
					m_Agents[i]->SetTarget(closest[index]);

				index++;

				if (index > closest.size() - 1)
				{
					index = 0;
					loops++;
				}
			}

			if (m_Agents[i]->HasBehaviour(Agent::BehaviourType::MINING))
				miners++;

			resourcesCollected	+= m_Agents[i]->GetMinedResources( );

			if ( m_Agents.size( ) < SQUAD_MAXIMUM_UNIT_COUNT )
				m_Resources		+= m_Agents[i]->GetMinedResources( );

			m_Agents[i]->SetMinedResources( 0 );

			if ( m_Resources > g_SSUpgrades.GetUnitValue(m_Agents[i]->GetEntityID()))
			{
				m_Resources -= g_SSUpgrades.GetUnitValue(m_Agents[i]->GetEntityID());
				SpawnUnit(m_Agents[i]->GetWorldPos());
			}
		}

		// Heal units
		fVector<Agent*> agents{ m_Agents.begin( ), m_Agents.end( ) };
		std::sort( agents.begin( ), agents.end( ), [] ( Agent* a, Agent* b ) -> bool
		{
			return GetDenseComponent<HealthComponent>( a->GetEntityID( ) )->Health > GetDenseComponent<HealthComponent>( b->GetEntityID( ) )->Health;
		} );

		HealthComponent* healthComponent = GetDenseComponent<HealthComponent>( agents.back( )->GetEntityID( ) );

		while ( resourcesCollected > 0 )
		{
			if ( healthComponent->Health < healthComponent->MaxHealth )
			{
				healthComponent->Health += healthComponent->Regen;
				--resourcesCollected;
			}
			else
			{
				agents.pop_back( );
				if ( !agents.empty( ) )
				{
					healthComponent = GetDenseComponent<HealthComponent>( agents.back( )->GetEntityID( ) );
				}
				else
				{
					keepMining = m_Agents.size( ) < SQUAD_MAXIMUM_UNIT_COUNT;

					if (!keepMining && GetTeam() == g_PlayerData.GetPlayerID())
						g_SSGUIInfo.DisplayMessage("Squad too big and units too healthy. Split squad to keep multiplying.");
					break;
				}
			}
		}
	}

	if ( miners > 0 && keepMining )
		return;

	for ( auto& agent : m_Agents )
	{
		agent->PopBehaviour( Agent::BehaviourType::MINING );
		agent->PopBehaviour( Agent::BehaviourType::FIND_GOALS );
		agent->SetTarget( ENTITY_INVALID );
		agent->ClearPath();
	}

	StandardMissionEnd();
}

void Squad::AttackMissionStart()
{
	m_AttackRepathTimer = 0.0f;

	AllAgentsAddBehaviour(Agent::BehaviourType::FIND_GOALS);
	AllAgentsAddBehaviour(Agent::BehaviourType::ATTACK);
	
	UseHoldPosition(false);

	ProgressMission();
}

void Squad::AttackMissionRun(float dt)
{
	Squad* targetSquad = g_SSAI.GetSquadWithID( m_TargetTeam, m_TargetSquad );
	if ( !targetSquad )
	{
		ProgressMission();
		return;
	}

	rVector<Entity> closest = Terrain::GetInstance()->GetEnemiesInRadius(Terrain::GetInstance()->GetTile(m_Position.x, m_Position.y), std::max(static_cast<int>(GetLeader()->GetWeapon()->Range),40),m_Team);
	int index = 0;
	int loops = 0;
	if (closest.size() > 0)
	{
		for (int i = 0; i < m_Agents.size(); i++)
		{

			if (loops < 4)
			{
				if (m_Agents[i]->GetTarget() == ENTITY_INVALID)
					m_Agents[i]->SetTarget(closest[index]);

				index++;

				if (index > closest.size() - 1)
				{
					index = 0;
					loops++;
				}
			}
		}
	}
	else
		ProgressMission();
// 	Entity savedTarget = ENTITY_INVALID;
// 	for (auto& agent : m_Agents)
// 	{
// 		Entity agentTarget = agent->GetTarget();
// 		if (agentTarget != ENTITY_INVALID)
// 		{
// 			savedTarget = agentTarget;
// 		}
// 	}
// 
// 	if (savedTarget != ENTITY_INVALID)
// 	{
// 		for (auto& agent : m_Agents)
// 		{
// 			Entity agentTarget = agent->GetTarget();
// 			if (agentTarget == ENTITY_INVALID)
// 			{
// 				agent->SetTarget(savedTarget);
// 			}
// 		}
// 	}
// 	else
// 	{
// 		//no targets on agents, but maybe there are still enemies not discovered
// 		Terrain::EnemyZone ez = Terrain::GetInstance()->GetClosestEnemy(Terrain::GetInstance()->GetTile(m_Position.x, m_Position.y), 30, m_Team, 0);
// 		if (ez.Closest == ENTITY_INVALID)
// 		{
// 			ProgressMission();
// 		}
// 		else
// 		{
// 			GetLeader()->SetTarget(ez.Closest);
// 		}
// 	}

// 	m_AttackRepathTimer -= dt;
// 	if ( m_AttackRepathTimer <= 0.0f )
// 	{
// 		m_AttackRepathTimer = SQUAD_ATTACK_REPATH_TIME;
// 		m_Goal = Terrain::GetInstance()->GetTile( targetSquad->GetPosition().x, targetSquad->GetPosition().y );
// 		this->BuildSquadPath();
// 	}
// 
// 	for ( auto& agent : m_Agents )
// 	{
// 		if ( agent->GetTarget() == ENTITY_INVALID )
// 		{
// 			Entity	closestEnemyID	= ENTITY_INVALID;
// 			float	closestDistance	= FLT_MAX;
// 			for ( auto& enemy : targetSquad->GetAgents() )
// 			{
// 				const float dist = glm::length2( agent->GetPos() - enemy->GetPos() );
// 				if ( dist < closestDistance )
// 				{
// 					closestEnemyID	= enemy->GetEntityID();
// 					closestDistance	= dist;
// 				}
// 			}
// 			if ( closestDistance < ATTACK_RANGE_AGENT_SQRD )
// 			{
// 				agent->AddBehaviour(Agent::BehaviourType::ATTACK);
// 				agent->ClearPath();
// 				agent->SetTarget( closestEnemyID );
// 				break;
// 			}
// 		}
// 	}

//	FormationGoals(dt);
}

void Squad::AttackMissionEnd()
{
	for ( auto& agent : m_Agents )
	{
		agent->PopBehaviour(Agent::BehaviourType::FIND_GOALS);
		agent->PopBehaviour(Agent::BehaviourType::ATTACK);
	}
	ProgressMission();
}

void Squad::HoldMissionStart()
{
	m_Goal = GetLeader()->GetTile();
	SetTarget(ENTITY_INVALID);
	UseHoldPosition(true);
	ProgressMission();
}

void Squad::HoldMissionEnd()
{
	ProgressMission();
	PopAllMissions();
}

void Squad::EmptyMission(float dt)
{
	ProgressMission();
}

void Squad::EmptyMission()
{
	ProgressMission();
}

void Squad::UseAutoAttack(bool _attack, bool _toggle)
{
	if (_toggle)
		m_UseAutoAttack = !m_UseAutoAttack;
	else
		m_UseAutoAttack = _attack;

	if (m_UseAutoAttack)
	{
		for (int i = 0; i < m_Agents.size(); i++)
		{
			m_Agents[i]->AddBehaviour(Agent::BehaviourType::AUTO_ATTACK);
		}
	}
	else
	{
		for (int i = 0; i < m_Agents.size(); i++)
		{
			m_Agents[i]->PopBehaviour(Agent::BehaviourType::AUTO_ATTACK);
		}
	}
}

Agent* Squad::IsUnderAttack() const
{
	for (auto & agent : m_Agents)
	{
		if (agent->IsUnderAttack())
			return agent;
	}
	return nullptr;
}

const bool Squad::HasMission() const
{
	return (m_Missions.size() > 0);
}

void Squad::PlaySFX(const rString &path, glm::vec3 pos, const float distMin, const float distMax)
{
	SFXEvent event;
	event.AtBeat = BeatType::NONE;
	event.Name = path;
	event.Info3D.DistMin = distMin;
	event.Info3D.DistMax = distMax;
	event.Info3D.Is3D = true;
	event.Info3D.Position = pos;
	g_SSAudio.PostEventSFX(event);
}

void Squad::SetTarget(Entity _target)
{
	for (auto & agent : m_Agents)
	{
		agent->SetTarget(_target);
	}
}

void Squad::SetTargetSquad(Entity _target)
{
	if ( HasComponent<AgentComponent>(_target) )
	{
		Agent* targetAgent	= GetDenseComponent<AgentComponent>( _target )->Agent;
		m_TargetSquad		= targetAgent->GetSquadID();
		m_TargetTeam		= targetAgent->GetTeam();

		//to clear mining targets
		SetTarget(_target);
	}
}

void Squad::UseHoldPosition(bool _hold)
{
	for (auto & agent : m_Agents)
	{
		if (_hold)
			agent->AddBehaviour(Agent::BehaviourType::HOLD_POSITION);
		else
			agent->PopBehaviour(Agent::BehaviourType::HOLD_POSITION);
	}

	m_UseHoldPosition = _hold;
}

void Squad::UpgradeMissionStart()
{
	float squadValue = (float) g_SSUpgrades.GetSquadValue( m_Team, m_ID );
	float unitValue = squadValue / m_Agents.size( );
	float newValue  = unitValue + g_SSUpgrades.GetUpgrade( m_UpgradeTo ).Value;
	float unitCount	= squadValue / newValue;

	if ( newValue > squadValue )
	{
		m_Missions[0].Progress = MissionState::REMOVE;

		if (g_PlayerData.GetPlayerID() == m_Team)
			g_SSGUIInfo.DisplayMessage( "Not enough units in squad to complete upgrade." );

		return;
	}

	gfx::ParticleSystem particleSystem;
	particleSystem.Colour				= glm::vec4( 1.0f );
	particleSystem.EmitterPosition		= glm::vec3( m_Position.x, gfx::g_GFXTerrain.GetHeightAtWorldCoord( m_Position.x, m_Position.y ), m_Position.y );
	particleSystem.Size					= glm::vec2( 3.0f );
	particleSystem.ParticlesTimeToLive	= 0.5f;
	particleSystem.TimeToLive			= FLT_MAX;
	m_UpgradeParticleSystem = g_SSParticle.SpawnParticleSystem( PARTICLE_TYPE_SMOKE_CLOUD, particleSystem );

	for ( auto& agent : m_Agents )
	{
		agent->AddBehaviour( Agent::BehaviourType::FIND_GOALS );
		agent->AddBehaviour( Agent::BehaviourType::NO_CLIP );
		agent->SetEvasion(0);
		agent->OverrideGoal( Goal( Terrain::GetInstance()->GetTile( m_Position.x, m_Position.y ), m_NextGoalTag ) );
		agent->KeepDistance( 0.0f, m_NextGoalTag );
		++m_NextGoalTag;
	}

	m_UpgradeTimer = SQUAD_UPGRADE_TIME;

	UseHoldPosition(false);
	this->UseAutoAttack( false, false );
	SetTarget(ENTITY_INVALID);

	ProgressMission();
}

void Squad::UpgradeMissionRun(float dt)
{
	m_UpgradeTimer -= dt;

	//can be null if particle system was flooded
	if (m_UpgradeParticleSystem != nullptr)
		m_UpgradeParticleSystem->TimeToLive = SQUAD_UPGRADE_SMOKE_TIME_OUT;

	if ( m_UpgradeTimer <= 0.0f )
		ProgressMission();
}

void Squad::UpgradeMissionEnd()
{
	//can be null if particle system was flooded
	if (m_UpgradeParticleSystem != nullptr)
		m_UpgradeParticleSystem->TimeToLive = SQUAD_UPGRADE_SMOKE_TIME_OUT;

	float squadValue = (float) g_SSUpgrades.GetSquadValue( m_Team, m_ID );
	float unitValue = squadValue / m_Agents.size( );
	float newValue  = unitValue + g_SSUpgrades.GetUpgrade( m_UpgradeTo ).Value;
	float unitCount	= squadValue / newValue;

	int unitNr = 0;

	fVector<Agent*> agents( m_Agents.begin( ), m_Agents.end( ) );
	std::sort( agents.begin( ), agents.end( ), [] ( Agent* a, Agent* b ) -> bool 
	{
		// Sort from highest health to lowest
		return GetDenseComponent<HealthComponent>( b->GetEntityID( ) )->Health < GetDenseComponent<HealthComponent>( a->GetEntityID( ) )->Health;
	} );

	Agent* agentToClone = nullptr;
	for (auto& agent : agents)
	{
		++unitNr;
		if ( unitNr > (int) unitCount )
		{
			HealthComponent* hc = GetDenseComponent<HealthComponent>(agent->GetEntityID());
			hc->IsDead = true;
		}
		else
		{
			agentToClone = agent;
			agent->PopBehaviour( Agent::BehaviourType::NO_CLIP );
			g_SSUpgrades.GiveUnitUpgrade(agent->GetEntityID(), m_UpgradeTo);
		}
	}

	float intpart;
	m_Resources += (int) (modff( unitCount, &intpart ) * newValue);
	if ( m_Resources > g_SSUpgrades.GetUnitValue( m_Agents[0]->GetEntityID( ) ) )
	{
		m_Resources -= g_SSUpgrades.GetUnitValue( m_Agents[0]->GetEntityID( ) );

		if ( agentToClone != nullptr )
			SpawnUnit( agentToClone->GetWorldPos( ) );
		else if ( !agents.empty( ) )
		{
			HealthComponent* hc = GetDenseComponent<HealthComponent>( agents[0]->GetEntityID( ) );
			hc->IsDead = false;

			agents[0]->PopBehaviour( Agent::BehaviourType::NO_CLIP );
			g_SSUpgrades.GiveUnitUpgrade( agents[0]->GetEntityID( ), m_UpgradeTo );
		}
	}

	if ( m_Missions.size() == 1 )
	{
		m_Missions.insert(m_Missions.begin() + 1, Mission(MISSION_TYPE_MOVE, Terrain::GetInstance()->GetTile(m_Position.x, m_Position.y), ENTITY_INVALID));
	}
	
	ProgressMission();

	///Notify that an upgrade has been completed
	g_SSMail.PushToNextFrame(UpgradeCompleteMessage(m_Team, m_UpgradeTo));
}

void Squad::MissionAccomplished()
{
	m_Missions.erase(m_Missions.begin());
	m_MissionProgressPosition = m_Position;
}

rString Squad::GetFormationName() const
{
	if (m_Formation == FORMATION::FORMATION_COLUMN)
		return "Column";
	if (m_Formation == FORMATION::FORMATION_HEDGEHOG)
		return "Hedgehog";
	if (m_Formation == FORMATION::FORMATION_LINE)
		return "Line";
	if (m_Formation == FORMATION::FORMATION_LINE_OFFENSIVE)
		return "Pocket";

	return "None";
}

Squad::MissionType Squad::GetCurrentMission()
{
	if (m_Missions.size() > 0)
	{
		return m_Missions[0].Type;
	}
	else
		return MISSION_TYPE_NONE;
}

void Squad::FocusFireMissionStart()
{
	AllAgentsAddBehaviour(Agent::BehaviourType::ATTACK);
	AllAgentsAddBehaviour(Agent::BehaviourType::FIND_GOALS);
	AllAgentsPopBehaviour(Agent::BehaviourType::AUTO_ATTACK);

	SetTargetSquad(m_Missions[0].Target);

	ProgressMission();
}
