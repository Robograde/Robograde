/**************************************************
2015 Viktor Kelkkanen & Ola Enberg
***************************************************/

#include "Agent.h"
#include "../input/GameMessages.h"
#include <gfx/Animation.h>
#include "Terrain.h"
#include "../datadriven/ComponentHelper.h"
#include "../subsystem/audio/SSSFXEmitter.h"
#include "../subsystem/utility/SSMail.h"
#include "PathFinder.h"
/*#include <gfx/AnimationTypes.h>*/
#include "../component/AnimationComponent.h"
#include "../ai/AIMessages.h"
#include "../utility/Alliances.h"

#if AI_DEBUG == 1
#include <profiler/Profiler.h>
#include <profiler/AutoProfiler.h>
#endif
#include "../subsystem/gfx/SSParticle.h"
#include "../subsystem/gfx/SSFogOfWar.h"
#include "../EntityFactory.h"
#include "../component/VisionComponent.h"

#define AGENT_ALLOWED_TILE_COST	1

Agent::Agent(int _x, int _y, int _squad, Entity _id, int _team) //TODOVK Make position parameters float and use z as z instead of y
{
	m_EntityID				= _id;
	m_Pos.x					= (float)_x;
	m_Pos.y					= (float)_y;
	m_Pos.z					= 0.0f;
	m_Forward.x				= 0.0f;
	m_Forward.y				= 1.0f;
	m_Forward.z				= 0.0f;

	m_Team					= _team;
	m_SquadID				= _squad;
	m_Evasion				= 0;
	m_Size					= 0;
	m_MinedResources		= 0;
	m_UpdateID				= 0;
	m_ScanRadius			= 30;
	m_TargetGoalTag			= -1;
	m_CurrentWeapon			= -1;
	m_FavouredWeapon		= -1;

	m_Speed					= 0.1f;
	m_SpeedModifier			= 1.0f;
	m_AttackedSince			= 0.0f;
	m_TurnRate				= 6.0f;
	m_IdleTimer				= 0.0f;
	m_TargetGoalDistance	= 0.0f;
	m_MiningCoolDown		= 0.7f;
	m_MiningCoolDownTimer	= 0.0f;

	m_IsBlocked				= false;
	m_IsAttacking			= false;
	m_IsUnderAttack			= false;
	m_IsEvasiveUnit			= true;
	m_IsIdle				= true;
	m_IsFacingTarget		= true;

	m_Buddy					= nullptr;
	m_EvasionGoal			= nullptr;

	m_Target				= ENTITY_INVALID;

	m_Tile					= Terrain::GetInstance()->GetTile(m_Pos.x, m_Pos.y);

	m_PrevTile				= m_Tile;
	m_Goal					= m_Tile;

	if (GetDenseComponentFlag<PlacementComponent>() & EntityManager::GetInstance().GetEntityMask(m_EntityID))
	{
		PlacementComponent* pc = GetDenseComponent<PlacementComponent>(m_EntityID);

		pc->Position.x = m_Pos.x;
		pc->Position.z = m_Pos.y;
	}
	if (GetDenseComponentFlag<HealthComponent>() & EntityManager::GetInstance().GetEntityMask(m_EntityID))
	{
		HealthComponent* hc = GetDenseComponent<HealthComponent>(m_EntityID);

		hc->Health = 100.0f;
	}

	NotifySensor();

}

Agent::~Agent()
{
// 	if (m_PathFinder!= nullptr)
// 		rDelete( m_PathFinder );
	Terrain::GetInstance()->UpdateSensor(m_EntityID, m_Tile, m_PrevTile,m_Size,true);
}

void Agent::Update(float dt)
{
	IsEvasiveUnit(true);
	IsAttacking(false);

	// Drop targets that are in the Fog of War.
	if (HasComponent<PlacementComponent>(m_Target))
	{
		PlacementComponent* targetPC = GetDenseComponent<PlacementComponent>(m_Target);
		if ( !g_SSFogOfWar.CalcPlayerVisibilityForPosition( m_Team, targetPC->Position ) )
			m_Target = ENTITY_INVALID;
	}

	for (int i = 0; i < m_Behaviours.size(); i++)
	{
		(this->*m_Behaviours[i].Behave)(dt);
	}

	//rotates agent toward target when its not done by OnMove
	if (HasComponent<PlacementComponent>(m_Target))
	{
		if (m_Path.size() == 0 || !HasBehaviour(BehaviourType::FIND_GOALS))
		{
			PlacementComponent* targetPC = GetDenseComponent<PlacementComponent>(m_Target);

			if (targetPC != nullptr)
			{
				PlacementComponent* agentPC = GetDenseComponent<PlacementComponent>(m_EntityID);

				glm::vec3 rotateVector = targetPC->Position - agentPC->Position;

				rotateVector.y = rotateVector.z;
				rotateVector.z = 0;

				RotateAgent(rotateVector, dt);
			}
		}
	}

	if (!HasBehaviour(BehaviourType::FIND_GOALS) || m_Path.empty())
	{
		Entity movementModule = GetDenseComponent<ParentComponent>(m_EntityID)->Children[MODULE_SLOT_SIDES];
		if (movementModule != ENTITY_INVALID)
		{
			if (HasComponent<AnimationComponent>(movementModule))
			{
				AnimationComponent* ac = GetDenseComponent<AnimationComponent>(movementModule);
				ac->StartAnimation(gfx::AnimationType::Idle);
			}
		}
	}

	if (m_CurrentWeapon >= 0)
	{
		if (m_Weapons[m_CurrentWeapon] != nullptr && m_Weapons[m_CurrentWeapon]->CoolDownTimer > 0)
		{
			for (int i = 0; i < m_Weapons.size(); i++)
				m_Weapons[i]->CoolDownTimer -= dt;
		}
	}
	
	if (m_IsUnderAttack)
		m_AttackedSince += dt;

	if (m_AttackedSince > RESET_ATTACKED_TIME)
	{
		m_AttackedSince = 0;
		IsUnderAttack(false);
	}

	//sets to idle if stuck for IDLE_TIMER_MAX time
	if (!m_IsIdle)
		m_IdleTimer += dt;
	else
	{
		m_Evasion = 0;
	}

	if (m_IdleTimer > IDLE_TIMER_MAX)
	{
		m_IsIdle = true;
		m_IdleTimer = 0;
#if AI_DEBUG == 1
		printf("agent was non idle without moving for too long, set to idle.\n");
#endif
	}
}

glm::vec2 Agent::GetPos()
{
	PlacementComponent* pc = GetDenseComponent<PlacementComponent>(m_EntityID);
	return glm::vec2(pc->Position.x,pc->Position.z);
}

const glm::vec3& Agent::GetWorldPos() const
{
	PlacementComponent* pc = GetDenseComponent<PlacementComponent>(m_EntityID);
	return pc->Position;
}

void Agent::AttemptMove(glm::vec3 _move, float _dt)
{
	if (_move.x == 0 && _move.y == 0)
		return;

	Terrain* terrain = Terrain::GetInstance();

	float lngth = sqrt(_move.x*_move.x + _move.y*_move.y);

	float newX = _move.x / lngth;
	float newY = _move.y / lngth;

	float		amountBehind	= 0.0f;
	glm::vec2	prevPos			= glm::vec2( m_Pos.x, m_Pos.y );
	glm::vec2	nextPos;
	if ( m_TargetGoalTag != -1 )
	{
		for ( int i = static_cast<int>(m_Path.size()) - 1; i >=0; --i )
		{
			if ( m_Path[i].Tag > m_TargetGoalTag )
			{
				m_SpeedModifier = glm::min( 1.0f, m_SpeedModifier );
				m_SpeedModifier = glm::max( 0.5f, m_SpeedModifier - 10.0f * _dt );
				break;
			}
			else if ( m_Path[i].Tag == m_TargetGoalTag )
			{
				amountBehind += glm::length( glm::vec2( m_Path[i].Tile->X - prevPos.x, m_Path[i].Tile->Y - prevPos.y ) ) - m_TargetGoalDistance;
				if ( amountBehind > 0.0f )
				{
					m_SpeedModifier = glm::max( 1.0f, m_SpeedModifier );
					m_SpeedModifier = glm::min( 2.0f, m_SpeedModifier + amountBehind * _dt );
				}
				else if ( amountBehind < 0.0f )
				{
					m_SpeedModifier = glm::min( 1.0f, m_SpeedModifier );
					m_SpeedModifier = glm::max( 0.5f, m_SpeedModifier + amountBehind * _dt );
				}
				break;
			}
			else
			{
				nextPos			= glm::vec2( m_Path[i].Tile->X, m_Path[i].Tile->Y );
				amountBehind	+= glm::length( nextPos - prevPos );
				prevPos			= nextPos;
			}
		}
	}
	m_TargetGoalTag			= -1;
	m_TargetGoalDistance	= 0.0f;

	const float distanceToTravel = glm::min( lngth, m_Speed*_dt * m_SpeedModifier );

	_move.x = (newX)*distanceToTravel;
	_move.y = (newY)*distanceToTravel;

	const glm::vec2 pos = this->GetPos();
	Tile* t = terrain->GetTile(pos.x + _move.x, pos.y + _move.y);

	//forward blocked by terrain
	if (terrain->IsUnpathableTerrain(t))
	{
		m_IsBlocked = true;
		return;
	}

	//forward blocked by objects
// 	if (!terrain->IsOccupiedByUnitExclude(t,m_EntityID))
// 	{
// 		m_IsBlocked = false;
// 		OnMove(_move, _dt);
// 		return;
// 	}

	if (terrain->IsOccupiedByNonEvasiveUnitExclude(t,m_EntityID))
	//if (terrain->IsUnpathableTerrain(t))
	{
		m_IsBlocked = true;
	}
	else
	{
		Entity e = terrain->WhoIsOnTile(t);
		Agent* a = GetAgentPointer(e);

		if (a != nullptr)
		{
			if (a->GetSquadID() != m_SquadID)
			{
				HandleBeingBlockedByAgent(a);
				return;
			}
		}

		m_IsBlocked = false;
		OnMove(_move, _dt,false);
	}
}

void Agent::HandleBeingBlockedByAgent(Agent* _agent)
{
	m_Evasion++;

	//break formation if too many evasions going on
	if (m_Evasion > MAX_EVASIONS_TO_STUCK)
	{
		//m_Formation = Formation::BREAK;
		//m_Spacing = 3;
		m_Evasion = 0;
	}

	//if (/*(_agent->GetID() >= m_ID) || */_agent->IsIdle())
			_agent->Evade(this);
}

void Agent::Evade(Agent* _agent)
{
	if (m_Evasion > 0)
		return;

	//just in case agent doesn't have it
	AddBehaviour(BehaviourType::FIND_GOALS);

	//pop recent evasion if bad
	if (m_Path.size() > 0)
	{
		if (m_Path[m_Path.size() - 1].Tile == m_EvasionGoal)
		{
			if (Terrain::GetInstance()->IsOccupiedByUnitExclude(m_Path[m_Path.size() - 1].Tile, m_EntityID))
			m_Path.pop_back();
			else
				return;
		}
	}
 		
	//push evasion goal
	SetEvasionGoal();
}

void Agent::AddPath(rVector<Goal>& _path)
{
	if (_path.empty())
	{
		return;
	}

#if AI_DEBUG == 1
	for (int i = 0; i < _path.size(); i++)
	{
		_path[i].Tile->DebugInfo = 16;
	}
#endif

	m_Path.insert(m_Path.end(), _path.begin(), _path.end());
	SetGoal(m_Path[m_Path.size() - 1].Tile);
}

void Agent::SetPath(rVector<Goal>& _path)
{
	if (_path.empty())
	{
		return;
	}

	for (int i = (int)(m_Path.size() - 1); i > -1; i--)
	{
		m_Path.erase(m_Path.begin() + i);
	}
	//m_IsIdle = false;
	m_Path = _path;
	SetGoal(m_Path[m_Path.size()-1].Tile);
}

void Agent::ClearPath()
{
	m_Path.clear();
}

void Agent::AddGoal(const Goal& goal)
{
	m_Path.insert( m_Path.begin(), goal );

	if ( m_Path.size() <= 1 )
	{
		SetGoal( goal.Tile );
	}
}

void Agent::OverrideGoal(const Goal& goal)
{
	m_Path.clear();

	m_Path.push_back(goal);
	SetGoal(goal.Tile);
}

bool Agent::HasBuddy() const
{
	if (m_Buddy == nullptr)
		return false;
	else
		return true;
}

void Agent::NextGoal()
{
	if (m_Path[m_Path.size() - 1].Tile == m_EvasionGoal)
	{
		m_EvasionGoal = nullptr;
		m_Evasion = 0;
		//m_IsIdle = false;
	}
// #if AI_RENDER_DEBUG == 1
// 	Terrain::GetInstance()->SetTerrain(m_Path[m_Path.size() - 1]->X, m_Path[m_Path.size() - 1]->Y, 0);
// #endif
	m_Path.pop_back();
	if (m_Path.size() > 0)
		SetGoal(m_Path.back().Tile);
	else
		SetGoal(m_Tile);
}

void Agent::SetGoal(Tile* _tile)
{
	m_Goal = Terrain::GetInstance()->GetMovableTile((float)(_tile->X), (float)(_tile->Y), m_EntityID,m_Tile);
}

glm::vec3 Agent::SimplePathFind()
{
	PlacementComponent* pc = GetDenseComponent<PlacementComponent>(m_EntityID);
	glm::vec3 pos = pc->Position;

	float x = 0, y = 0;
	if (m_Goal->X != m_Tile->X)
		x = m_Goal->X - pos.x;
	if (m_Goal->Y != m_Tile->Y)
		y = m_Goal->Y - pos.z;

	return glm::vec3(x, y, 0);

}

void Agent::AdvancedPathFind()
{
	Terrain* terrain = Terrain::GetInstance();



	if (terrain->IsUnpathableTerrain(m_Goal) || terrain->IsOccupiedByNonEvasiveUnitExclude(m_Goal, m_EntityID))
	{
		m_Path.back().Tile = terrain->GetMovableTile(static_cast<float>(m_Goal->X), static_cast<float>(m_Goal->Y), m_EntityID,m_Tile);
		m_Goal = m_Path.back().Tile;
	}

	//if (terrain->IsOccupiedByNonEvasiveUnitExclude(m_Goal, m_EntityID) && !m_IsLeader)
	//{
	//	m_Goal = terrain->GetMovableTile(static_cast<float>(m_Goal->X), static_cast<float>(m_Goal->Y), m_EntityID, m_Tile);
	//	//CancelOrder();
	//	return;
	//}

// 	RunAStar();
// 	m_IsBlocked = false;
// 	return;

	rVector<Tile*> pathList;
	
	int px = m_Tile->X, py = m_Tile->Y;

	//added for breadcrumb, will not be added to final path
	pathList.push_back(m_Tile);
	while (true)
	{
		int x = 0, y = 0;
		int tx = 0, ty = 0;
		if (m_Goal->X != px)
			x = m_Goal->X - px > 0 ? 1 : -1;
		if (m_Goal->Y != py)
			y = m_Goal->Y - py > 0 ? 1 : -1;

		if (x == 0 && y == 0)
			break;

		bool bread = false;

		//breadcrumbs, so agent doesnt get stuck going back
		Tile* t = terrain->GetTile(px + x, py + y);
		for (int k = 0; k < pathList.size(); k++)
		{
			if (t == pathList[k])
			{
				bread = true;
				break;
			}
		}

		if (bread || terrain->IsUnpathableTerrain(t) || terrain->IsOccupiedByNonEvasiveUnitExclude(t, m_EntityID))
		{
			bool canMove = false;
			int low = 1000;
			for (int i = -1; i < 2; i++)
			{
				for (int j = -1; j < 2; j++)
				{
					if (i == 0 && j == 0)
						continue;

					t = terrain->GetTile(px + j, py + i);

					if (!terrain->IsUnpathableTerrain(t) && !terrain->IsOccupiedByNonEvasiveUnitExclude(t, m_EntityID))
					{
						int temp = abs(px + j - m_Goal->X) + abs(py + i - m_Goal->Y);

						if (temp > low)
							continue;

						//breadcrumbs
						bread = false;
						
						for (int k = 0; k < pathList.size(); k++)
						{
							if (t == pathList[k])
							{
								bread = true;
								break;
							}
						}

						if (bread)
							continue;

						canMove = true;
						low = temp;
						x = j;
						y = i;
					}
				}
			}
			if (!canMove)
			{
				//not gonna be solved like this. do A*
				RunAStar();
				m_IsBlocked = false;
				return;
			}
		}
		
		px += x;
		py += y;

		pathList.push_back(terrain->GetTile(px, py));

		if (pathList.back() == m_Goal)
			break;

		if (pathList.size() > 30)
		{
			//not gonna be solved like this. do A*
			RunAStar();
			m_IsBlocked = false;
			return;
		}
	}

	//have to change order of list to keep same format as A* paths
	rVector<Goal> pathListHeap;
	for (int i = 0; i < pathList.size() -1; i++)
	{
		pathListHeap.push_back( Goal(pathList[pathList.size() - 1 - i]) );
	}

	//pop the original goal added for crumb
	if (pathListHeap.size() > 1)
		pathListHeap.erase(pathListHeap.begin());

 		AddPath(pathListHeap);

 	m_IsBlocked = false;
}

void Agent::CancelOrder()
{
	OverrideGoal(m_Tile);
}

void Agent::KeepDistance( float distance, int tag )
{
	m_TargetGoalDistance	= distance;
	m_TargetGoalTag			= tag;
}

void Agent::RunAStar()
{
#if AI_DEBUG == 1
	PROFILE(AutoProfiler AIAStar("AStar"));
#endif

	Pathfinder* pathfinder = Pathfinder::GetInstance();

	pathfinder->SetStart(m_Tile);

	int waypointTag = -1;

	if ( m_Path.empty() )
	{
		pathfinder->SetGoal(m_Goal);
	}
	else
	{
		auto importantWaypoint = m_Path.end() - 1;
		for ( ; importantWaypoint >= m_Path.begin(); --importantWaypoint )
		{
			if ( importantWaypoint->Tag >= 0 )
				break;

			if ( importantWaypoint == m_Path.begin() )	// Iterators are not allowed to go bellow .begin()
				break;
		}
		pathfinder->SetGoal( importantWaypoint->Tile );
		waypointTag = importantWaypoint->Tag;
		m_Path.erase( importantWaypoint, m_Path.end() );
	}

	pathfinder->SetStartFloat2( this->GetPos() );
	rVector<Goal> path = pathfinder->AStar(m_Tile,true,AGENT_ALLOWED_TILE_COST,false);

	if ( !path.empty() )
		path[0].Tag = waypointTag;

	AddPath( path );
#if AI_DEBUG == 1
	PROFILE(AIAStar.Stop());
#endif
}

void Agent::OnMove(glm::vec3 _move, float _dt, bool _noClip)
{

	m_Forward.x = _move.x;
	m_Forward.y = _move.y;

	//float lngth = sqrt(_move.x*_move.x + _move.y*_move.y);

// 	_move.x = (_move.x / lngth)*m_Speed*_dt;
// 	_move.y = (_move.y / lngth)*m_Speed*_dt;

	RotateAgent(_move, _dt);

	{
		Entity movementModule = GetDenseComponent<ParentComponent>(m_EntityID)->Children[MODULE_SLOT_SIDES];
		if (movementModule != ENTITY_INVALID)
		{
			if ( HasComponent<AnimationComponent>( movementModule ) )
			{
				AnimationComponent* ac = GetDenseComponent<AnimationComponent>( movementModule );
				ac->StartAnimation( gfx::AnimationType::Walk );
			}
		}

		float speedToUse = 1.0f;

		if (!m_IsFacingTarget)
			speedToUse = SPEED_FACTOR_ON_ROTATE;

		PlacementComponent* pc = GetDenseComponent<PlacementComponent>(m_EntityID);

		float newX = pc->Position.x + speedToUse*_move.x;
		float newZ = pc->Position.z + speedToUse*_move.y;

		Tile* t = Terrain::GetInstance()->GetTile(newX, newZ);

		if ((!Terrain::GetInstance()->IsUnpathableTerrain(t) && !Terrain::GetInstance()->IsOccupiedByNonEvasiveUnitExclude(t, m_EntityID)) || _noClip)
		{
			pc->Position.x = newX;
			pc->Position.z = newZ;
			m_Pos.x = newX;
			m_Pos.y = newZ;

			if (t != m_Tile)
			{
				m_PrevTile = m_Tile;
				m_Tile = t;
				m_IdleTimer = 0;
				NotifySensor();
			}
		}
		else
			m_IsBlocked = true;
// 
// 		float d = glm::distance(glm::vec3(m_Tile->X, m_Tile->Y, 0), m_Pos);
// 
// 		if (d >= 1.0f)
// 		{
// 			if (!Terrain::GetInstance()->IsUnpathableTerrain(t) && !Terrain::GetInstance()->IsOccupiedByNonEvasiveUnitExclude(t,m_EntityID))
// 			{
// 				m_PrevTile = m_Tile;
// 				m_Tile = t;
// 				m_IdleTimer = 0;
// 				NotifySensor();
// 			}
// 		}
	}


}

void Agent::SetEvasionGoal()
{
	Tile* t;
	int x = m_Tile->X;
	int y = m_Tile->Y;

	t = Terrain::GetInstance()->GetSurroundingFreeTile(static_cast<float>(x), static_cast<float>(y), m_EntityID);

	m_Path.push_back(t);
	m_Goal = t;

	m_EvasionGoal = t;
}

void Agent::NotifySensor()
{
	//footprints, need to not be entities to work
// 	Entity movementModule = GetDenseComponent<ParentComponent>(m_EntityID)->Children[MODULE_SLOT_SIDES];
// 	if (movementModule != ENTITY_INVALID)
// 	{
// 		if (HasComponent<UpgradeComponent>(movementModule))
// 		{
// 			if(GetDenseComponent<UpgradeComponent>(movementModule)->UpgradeDataID == UPGRADE_NAME::MOVEMENT_LEGS)
// 				EntityFactory::CreateEnvironmentDecal(GetWorldPos(), "footPrint.png", 2.0f, glm::vec4(1.0f), GetDenseComponent<PlacementComponent>(m_EntityID)->Scale.z, GetDenseComponent<PlacementComponent>(m_EntityID)->Orientation);
// 			if (GetDenseComponent<UpgradeComponent>(movementModule)->UpgradeDataID == UPGRADE_NAME::MOVEMENT_TRACKS)
// 				EntityFactory::CreateEnvironmentDecal(GetWorldPos(), "tracksPrint.png", 2.0f, glm::vec4(1.0f), GetDenseComponent<PlacementComponent>(m_EntityID)->Scale.z, GetDenseComponent<PlacementComponent>(m_EntityID)->Orientation);
// 		}
// 	}

	

	Terrain::GetInstance()->UpdateSensor(m_EntityID, m_Tile, m_PrevTile, m_Size, false);
}

void Agent::BehaviourMining(float _dt)
{
	if (m_Target == ENTITY_INVALID || !HasComponent<ResourceComponent>(m_Target))
	{
		m_Target = ENTITY_INVALID;
		return;
	}

	m_IsIdle = false;
	ResourceComponent* rc = GetDenseComponent<ResourceComponent>(m_Target);
	if (Terrain::GetInstance()->GetTileDistance(rc->TilePointer, m_Tile) <= AGENT_MINING_DISTANCE)
	{
		IsEvasiveUnit(false);
		m_MiningCoolDownTimer += _dt;
		if (m_MiningCoolDownTimer > m_MiningCoolDown)
		{
			OnMining();
			m_MiningCoolDownTimer = 0;
		}
		return;
	}
	else if (m_Path.empty())
	{
		m_Goal = Terrain::GetInstance()->GetMovableTile(static_cast<float>(rc->TilePointer->X), static_cast<float>(rc->TilePointer->Y), m_EntityID, m_Tile);
		RunAStar();

		if ( !m_Path.empty() && Terrain::GetInstance()->GetTileDistance(m_Path[0].Tile,m_Tile) > 30)
		{
			PopBehaviour(BehaviourType::MINING);
			m_Target = ENTITY_INVALID;
			ClearPath();
		}

		return;
	}
}

void Agent::BehaviourStandard(float _dt)
{

}

void Agent::BehaviourFindGoals(float _dt)
{
	for (int i = (int)(m_Path.size() - 1); i > -1; i--)
	{
		if (m_Tile != m_Goal)
		{
			AttemptMove(SimplePathFind(),_dt);

			if (m_IsBlocked)
			{
				if (m_IsBlocked)
				{
					AdvancedPathFind();
				}

				if (m_Path.size() > 2)
				{
					if (m_Path[m_Path.size() - 1].Tile == m_Path[m_Path.size() - 2].Tile)
					{
						BehaviourNoClip(_dt);
					}
				}
			}
		}
		else
		{
			NextGoal();
			continue;
		}
		m_IsIdle = false;
		return;
	}
	m_IsIdle = true;
}

void Agent::BehaviourAttack(float _dt)
{
	if (m_CurrentWeapon < 0)
	{
		PopBehaviour(BehaviourType::ATTACK);
		return;
	}

	if (m_Target != ENTITY_INVALID)
	{
		AttemptAttack(m_Target);
	}
	else
	{
		StopAttackAnimation();
		PopBehaviour(BehaviourType::ATTACK);
		AddBehaviour(BehaviourType::AUTO_ATTACK);
	}
}

void Agent::BehaviourAutoAttack(float _dt)
{
	if (m_CurrentWeapon < 0)
	{
		PopBehaviour(BehaviourType::AUTO_ATTACK);
		return;
	}

	if (m_Target != ENTITY_INVALID)
	{
		AttemptAttack(m_Target);
	}
	else
	{
		StopAttackAnimation();
		FindEnemy();
	}
}

void Agent::AttemptAttack(Entity _target)
{
	IsEvasiveUnit(false);

	Terrain* terrain = Terrain::GetInstance();
	Agent* targetAgent = GetAgentPointer(m_Target);
	if (targetAgent != nullptr)
	{
		if (!g_SSFogOfWar.CalcPlayerVisibilityForPosition(m_Team, targetAgent->GetWorldPos()))
		{
			m_Target = ENTITY_INVALID;
			return;
		}

		if (targetAgent->GetTeam() != m_Team && !g_Alliances.IsAllied(m_Team, targetAgent->GetTeam()))
		{
			if (m_Weapons[m_CurrentWeapon]->CoolDownTimer > 0)
			{
				CycleWeapon();
			}

			int dist = terrain->GetTileDistance(m_Tile, targetAgent->GetTile());

			//stop
			if (dist <= m_Weapons[m_FavouredWeapon]->Range)
			{
				if (m_Goal != m_Tile || m_Path.size() > 0)
					OverrideGoal(m_Tile);

				PopBehaviour(BehaviourType::FIND_GOALS);
				m_IdleTimer = 0;
			}

			
			int minRange = 0;
			if (m_Weapons[m_CurrentWeapon]->Name == UPGRADE_NAME::WEAPON_ARTILLERY_CANNON)
				minRange = 10;

			if (dist <= m_Weapons[m_CurrentWeapon]->Range && dist > minRange)
			{
				if (m_Weapons[m_CurrentWeapon]->IsTurret || (m_IsFacingTarget && !m_Weapons[m_CurrentWeapon]->IsTurret))
				{
					if (m_Weapons[m_CurrentWeapon]->CoolDownTimer <= 0)
					{
						Entity frontUpg = GetDenseComponent<ParentComponent>(m_EntityID)->Children[MODULE_SLOT_FRONT];
						if (frontUpg != ENTITY_INVALID)
						{
							if (HasComponent<AnimationComponent>(frontUpg))
							{
								if (HasComponent<WeaponComponent>(frontUpg))
								{
									if (GetDenseComponent<WeaponComponent>(frontUpg) == m_Weapons[m_CurrentWeapon])
									{
										AnimationComponent* ac = GetDenseComponent<AnimationComponent>(frontUpg);
										ac->StartAnimation(gfx::AnimationType::Attack);
									}
								}
							}
						}
						IsAttacking(true);
					}
				}
			}
			else if ((static_cast<int>(m_Path.size()) == 0 || (!m_IsFacingTarget && !m_Weapons[m_CurrentWeapon]->IsTurret)))
			{
				if (m_CurrentWeapon == m_FavouredWeapon)
				{
					StopAttackAnimation();

					Tile* targetTile = targetAgent->GetTile();

// 					if (terrain->FreePathBool(m_Tile, targetTile, false, m_EntityID, AGENT_ALLOWED_TILE_COST, this->GetPos()))
// 					{
						glm::vec2 v2 = glm::vec2(targetAgent->GetPos().x - m_Pos.x, targetAgent->GetPos().y - m_Pos.y);

						v2 = v2 / glm::length(v2);

						v2 *= -(/*m_Weapons[m_CurrentWeapon]->Range*/minRange+1);

						v2 += glm::vec2(targetTile->X, targetTile->Y);

						if (static_cast<int>(m_Path.size()) == 0)
						{
							m_Goal = terrain->GetTile(v2.x, v2.y);
							RunAStar();
						}

//						OverrideGoal(terrain->GetTile(v2.x, v2.y));

						AddBehaviour(BehaviourType::FIND_GOALS);
					//}
// 					/*else */if (static_cast<int>(m_Path.size()) == 0)
// 					{
// 						//AddBehaviour(BehaviourType::FIND_GOALS);
// 
// 						
// 					}
				}
				else
					CycleWeapon();
			}
 			else if (!HasBehaviour(BehaviourType::FIND_GOALS) && !HasBehaviour(BehaviourType::HOLD_POSITION))
 				AddBehaviour(BehaviourType::FIND_GOALS);
		}
	}
	else if (HasComponent<ResourceComponent>(m_Target))
	{
		//resource target snuck into attack behavior
		PopBehaviour(BehaviourType::ATTACK);
		PopBehaviour(BehaviourType::AUTO_ATTACK);
		
	}
	else
	{
		m_Target = ENTITY_INVALID;
		m_IsIdle = true;
	}
}

void Agent::IsUnderAttack(bool val)
{
	if (val)
	{
		if(!m_IsUnderAttack)
		{
			AIMessage message = AIMessage(static_cast<unsigned int>(AIStateMessages::UNDER_ATTACK), m_Team, m_SquadID,static_cast<unsigned int>(m_EntityID));
			message.CreatedFromPacket = true; // This is a hack since this is a simulation message that doesn't need to be synched over network (Works deterministically)
			g_SSMail.PushToNextFrame(message);
		}
	}

	m_IsUnderAttack = val;
}

void Agent::AddBehaviour(BehaviourType _val)
{
	for (int i = 0; i < m_Behaviours.size(); i++)
	{
		if (m_Behaviours[i].Type == _val)
		{
			return;
		}
	}

	m_Behaviours.push_back(Behaviour(_val));
}

void Agent::PopBehaviour(BehaviourType _val)
{
	if (_val == BehaviourType::AUTO_ATTACK || _val == BehaviourType::ATTACK)
	{
		m_Target = ENTITY_INVALID;
		StopAttackAnimation();
	}

	if (_val == BehaviourType::MINING)
	{
		m_IsIdle = true;
		m_Target = ENTITY_INVALID;
	}

	for (int i = 0; i < m_Behaviours.size(); i++)
	{
		if (m_Behaviours[i].Type == _val)
		{
			m_Behaviours.erase(m_Behaviours.begin()+i);
			return;
		}
	}
}

bool Agent::HasBehaviour(BehaviourType _val) const
{
	for (int i = 0; i < m_Behaviours.size(); i++)
	{
		if (m_Behaviours[i].Type == _val)
		{
			return true;
		}
	}
	return false;
}

void Agent::FindEnemy()
{
	m_CurrentWeapon = m_FavouredWeapon;

	if (HasBuddy())
	{
		Entity buddyTarget = m_Buddy->GetTarget();

		if (buddyTarget != ENTITY_INVALID)
		{
			Agent* a = GetAgentPointer(buddyTarget);

			if (a != nullptr)
			{
				if (a->GetTeam() != m_Team && !g_Alliances.IsAllied(m_Team, a->GetTeam()))
				{
					m_Target = buddyTarget;
					AttemptAttack(m_Target);
					return;
				}
			}
		}
	}

	Terrain::EnemyZone ez = Terrain::GetInstance()->GetClosestEnemy(m_Tile, std::max(static_cast<int>(m_Weapons[m_FavouredWeapon]->Range), static_cast<int>(GetDenseComponent<VisionComponent>(m_EntityID)->VisionRange)), m_Team, m_UpdateID);

	m_UpdateID = ez.UpdateID;

	if (ez.Closest != ENTITY_INVALID)
	{
		m_Target = ez.Closest;
		AttemptAttack(m_Target);
	}
}

void Agent::OnMining()
{
	m_IdleTimer = 0;
	m_MinedResources += MINING_DAMAGE_TO_RESOURCE;
	HealthComponent* hc = GetDenseComponent<HealthComponent>(m_Target);
	//g_SSParticle.DamageEffect(m_Target);

	//laser effect cutting resources
	gfx::ParticleSystem effect = g_SSParticle.GetDefaultParticleSystem(PARTICLE_TYPE_LASER);
	PlacementComponent* agentPC = GetDenseComponent<PlacementComponent>(m_EntityID);
	PlacementComponent* resourcePC = GetDenseComponent<PlacementComponent>(m_Target);
	effect.ParticlesSpeed = 50;
	effect.Colour = glm::vec4(0.4f, 1.0f, 0.4f, 0.4f);
	effect.TimeToLive = m_MiningCoolDown;
	effect.ParticlesTimeToLive = effect.TimeToLive;
	effect.EmitterPosition = agentPC->Position;
	effect.Direction = glm::normalize(resourcePC->Position - agentPC->Position) * glm::distance(resourcePC->Position,agentPC->Position);
	g_SSParticle.SpawnParticleSystem(PARTICLE_TYPE_LASER, effect);

	hc->OnAttacked(static_cast<float>(MINING_DAMAGE_TO_RESOURCE));
	if (hc->IsDead)
		m_Target = ENTITY_INVALID;

	///Request to play SFX while eating
	SFXEmitterComponent* sfxEmitter = GetDenseComponent<SFXEmitterComponent>(m_EntityID);

	if(sfxEmitter)
	{
		g_SSSFXEmitter.AddEvent(sfxEmitter, SFXTriggerType::WHILE_EATING);
	}
}

void Agent::BehaviourNoClip(float _dt)
{
	if (m_Tile != m_Goal)
	{
		glm::vec3 dir = SimplePathFind();

		float lngth = sqrt(dir.x*dir.x + dir.y*dir.y);

		float newX = dir.x / lngth;
		float newY = dir.y / lngth;

		dir.x = (newX)*m_Speed*_dt;
		dir.y = (newY)*m_Speed*_dt;

		if (newX > 0) newX = 1;
		else if (newX < 0) newX = -1;
		if (newY > 0) newY = 1;
		else if (newY < 0) newY = -1;

		Tile* t = Terrain::GetInstance()->GetTile(m_Tile->X + newX, m_Tile->Y + newY);

		m_IsBlocked = false;
		OnMove(dir, _dt,true);
	}
	else
		PopBehaviour(BehaviourType::NO_CLIP);
}

int Agent::GetPathSize() const
{
	return static_cast<int>(m_Path.size()); 
}

void Agent::CycleWeapon()
{
	m_CurrentWeapon++;
	if (m_CurrentWeapon >= m_Weapons.size())
		m_CurrentWeapon = 0;
}

void Agent::AddWeapon(WeaponComponent* val)
{
	m_CurrentWeapon++;

	m_Weapons.push_back(val);
	m_Weapons[m_CurrentWeapon] = val;
	m_Weapons[m_CurrentWeapon]->CoolDownTimer = 0;

	//random cooldown
	float randomCoolDown = m_Weapons[m_CurrentWeapon]->CoolDown / 100.0f;
	int percent = g_Randomizer.SimRand(0, RANDOM_COOLDOWN_PERCENT_SPREAD+1);

	randomCoolDown = percent*randomCoolDown;

	if(g_Randomizer.SimRand(0, 2))
		randomCoolDown = -randomCoolDown;

	m_Weapons[m_CurrentWeapon]->CoolDown += randomCoolDown;

	//best range gets favoured
	if (m_CurrentWeapon == 0)
		m_FavouredWeapon = m_CurrentWeapon;
	else
	{
		float longestRange = 0;
		for (int i = 0; i < m_Weapons.size(); i++)
		{
			if (m_Weapons[i]->Range > longestRange)
			{
				m_FavouredWeapon = i;
				longestRange = m_Weapons[i]->Range;
			}
		}
	}
}

void Agent::RotateAgent(glm::vec3 &_move, float _dt)
{
	PlacementComponent* pc = GetDenseComponent<PlacementComponent>(m_EntityID);

	const glm::vec3	startRot	= pc->Orientation * glm::vec3(0.0f, 0.0f, -1.0f);
	const float		angleStart	= glm::half_pi<float>() + glm::atan(startRot.z, -startRot.x);
	const float		angleGoal	= (glm::half_pi<float>() + glm::atan(_move.y, -_move.x));
	float			angleDist	= angleGoal - angleStart;

	if ( angleDist < 0.0f )
		angleDist += 2.0f * glm::pi<float>();

	const float angleChange = _dt * m_TurnRate;

	if ( angleDist < angleChange || angleDist > 2.0f * glm::pi<float>() - angleChange )
	{
		m_IsFacingTarget	= true;
		pc->Orientation		= glm::quat( glm::cos(0.5f * angleGoal), 0.0f, glm::sin(0.5f * angleGoal), 0.0f );
	}
	else
	{
		m_IsFacingTarget	= false;
		float angleEnd		= angleStart + ( angleDist < glm::pi<float>() ? angleChange : -angleChange );
		pc->Orientation		= glm::quat( glm::cos(0.5f * angleEnd), 0.0f, glm::sin(0.5f * angleEnd), 0.0f );
	}
}

void Agent::StopAttackAnimation()
{
	Entity movementModule = GetDenseComponent<ParentComponent>(m_EntityID)->Children[MODULE_SLOT_FRONT];
	if ( movementModule != ENTITY_INVALID )
	{
		if ( HasComponent<AnimationComponent>( movementModule ) )
		{
			AnimationComponent* ac = GetDenseComponent<AnimationComponent>( movementModule );
			ac->StartAnimation( gfx::AnimationType::Idle );
		}
	}
}

WeaponComponent* Agent::GetWeapon()
{
	if (m_CurrentWeapon < m_Weapons.size())
	{
		return m_Weapons[m_CurrentWeapon];
	}

	return nullptr;
}
