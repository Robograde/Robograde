/**************************************************
2015 Viktor Kelkkanen & Ola Enberg
***************************************************/

#pragma once
#include "stdafx.h"
#include <glm/glm.hpp>
#include <memory/Alloc.h>
#include "../datadriven/ComponentTypes.h"
#include "../component/WeaponComponent.h"

#define AGENT_SIZE_IN_TILES	2
#define AGENT_MINING_DISTANCE 2.0f
class Agent
{
public:
	enum class BehaviourType
	{
		STANDARD = 0,
		MINING,
		FIND_GOALS,
		HOLD_POSITION,
		ATTACK,
		AUTO_ATTACK,
		NO_CLIP,
	};

	struct Behaviour
	{
		BehaviourType Type;
		void(Agent::*Behave)(float dt);
		Behaviour(BehaviourType _type)
		{
			Type = _type;
			SetBehaviour();
		};
		Behaviour()
		{ 
			Type = BehaviourType::STANDARD;
			SetBehaviour(); 
		};

		void SetBehaviour()
		{
			if (Type == BehaviourType::MINING)
			{
				Behave = &Agent::BehaviourMining;
			}
			else if (Type == BehaviourType::FIND_GOALS)
			{
				Behave = &Agent::BehaviourFindGoals;
			}
			else if (Type == BehaviourType::ATTACK)
			{
				Behave = &Agent::BehaviourAttack;
			}
			else if (Type == BehaviourType::AUTO_ATTACK)
			{
				Behave = &Agent::BehaviourAutoAttack;
			}
			else if (Type == BehaviourType::NO_CLIP)
			{
				Behave = &Agent::BehaviourNoClip;
			}
			else
				Behave = &Agent::BehaviourStandard;
		}
	};

	//Functions
	Agent(int _x, int _y, int _squad, Entity _id, int _team);
	~Agent();

	void				Update(float dt);

	//Helpers
	void				NotifySensor();
	bool				HasBuddy() const;

	//Behaviours		
	void				PopBehaviour(Agent::BehaviourType val);
	void				AddBehaviour(BehaviourType _val);
	bool				HasBehaviour(Agent::BehaviourType val) const;
	void				FindEnemy();

	//Goals
	void				NextGoal();
	void				SetGoal(Tile* _tile);
	void				AddGoal(const Goal& goal);
	void				OverrideGoal(const Goal& goal);
	void				CancelOrder();
	void				KeepDistance( float distance, int tag );

	//Path finding
	void				AddPath(rVector<Goal>& _path);
	void				SetPath(rVector<Goal>& _path);
	void				ClearPath();
	void				RunAStar();


	//Get Sets
	glm::vec2			GetPos();
	int					GetX()								{ return (int)m_Pos.x; }
	int					GetY()								{ return (int)m_Pos.y; }

	glm::vec3			GetForward() const					{ return m_Forward; }
	void				SetForward(glm::vec3 val)			{ m_Forward = val; }

	Agent*				GetBuddy() const					{ return m_Buddy; }
	void				SetBuddy(Agent* val)				{ m_Buddy = val; }

	float				GetSpeed( ) const					{ return m_Speed; }
	void				SetSpeed( float val )				{ m_Speed = val; }

	float				GetTurnRate( ) const				{ return m_TurnRate; }
	void				SetTurnRate( float val )			{ m_TurnRate = val; }

	Tile*				GetTile() const						{ return m_Tile; }
	void				SetTile(Tile* val)					{ m_Tile = val; }

	int					GetSquadID() const					{ return m_SquadID; }
	void				SetSquadID(int val)					{ m_SquadID = val; }

	int					GetEvasion() const					{ return m_Evasion; }
	void				SetEvasion(int val)					{ m_Evasion = val; }

	Tile*				GetGoal() const						{ return m_Goal; }

	int					GetSize() const						{ return m_Size; }
	void				SetSize(int val)					{ m_Size = val; }

	Entity				GetTarget() const					{ return m_Target; }
	void				SetTarget(Entity val)				{ m_Target = val; }

	unsigned int		GetMinedResources() const			{ return m_MinedResources; }
	void				SetMinedResources(int val)			{ m_MinedResources = val; }

	int					GetTeam() const						{ return m_Team; }
	void				SetTeam(int val)					{ m_Team = val; }

	WeaponComponent*	GetWeapon();

	void				CycleWeapon();

	int					GetScanRadius() const				{ return m_ScanRadius; }
	void				SetScanRadius(int val)				{ m_ScanRadius = val; }

	int					GetPathSize() const;
	const rVector<Goal>& GetPath() const					{ return m_Path; }

	Entity				GetEntityID() const					{ return m_EntityID; }

	const glm::vec3&	GetWorldPos() const;
	void				AddWeapon(WeaponComponent* val);

	//Get Set booleans
	bool				IsIdle() const						{ return m_IsIdle; }
	void				IsIdle(bool val)					{ m_IsIdle = val; }

	bool				IsBlocked() const					{ return m_IsBlocked; }
	void				IsBlocked(bool val)					{ m_IsBlocked = val; }

	bool				IsEvasiveUnit() const				{ return m_IsEvasiveUnit; }
	void				IsEvasiveUnit(bool val)				{ m_IsEvasiveUnit = val; }

	bool				IsAttacking() const					{ return m_IsAttacking; }
	void				IsAttacking(bool val)				{ m_IsAttacking = val; }

	bool				IsUnderAttack() const				{ return m_IsUnderAttack; }
	void				IsUnderAttack(bool val);

	bool				IsFacingTarget() const				{ return m_IsFacingTarget; }


private:
	glm::vec3				m_Pos;
	glm::vec3				m_Forward;

	rVector<Goal>			m_Path;

	Tile*					m_Goal;
	Tile*					m_Tile;
	Tile*					m_PrevTile;
	Tile*					m_EvasionGoal;

	Entity					m_EntityID;
	Entity					m_Target;
	Agent*					m_Buddy;

	rVector<Behaviour>		m_Behaviours;

	rVector<WeaponComponent*>	m_Weapons;

	bool					m_IsIdle;
	bool					m_IsBlocked;
	bool					m_IsEvasiveUnit;
	bool					m_IsAttacking;
	bool					m_IsUnderAttack;
	bool					m_IsFacingTarget;

	int						m_SquadID;
	int						m_Evasion;
	int						m_Size;
	int						m_Team;
	int						m_ScanRadius;
	int						m_TargetGoalTag;
	int						m_CurrentWeapon;
	int						m_FavouredWeapon;

	unsigned int			m_MinedResources;
	unsigned int			m_UpdateID;

	float					m_Speed;
	float					m_SpeedModifier;
	float					m_AttackedSince;
	float					m_TurnRate;
	float					m_IdleTimer;
	float					m_TargetGoalDistance;
	float					m_MiningCoolDownTimer;
	float					m_MiningCoolDown;

	static const int		MAX_EVASIONS_TO_STUCK = 30;
	static const int		RESET_ATTACKED_TIME = 3;
		   const float		SPEED_FACTOR_ON_ROTATE = 0.3f;
	static const int		IDLE_TIMER_MAX = 5;
	static const int		MINING_DAMAGE_TO_RESOURCE = 10;
	static const int		RANDOM_COOLDOWN_PERCENT_SPREAD = 10;
		   

	//Attempts and externally relevant On-methods
	void		AttemptMove					(glm::vec3 _move, float dt);
	void		AttemptAttack				(Entity _target);

	void		OnAttacking					(Agent* _target);
	void		OnMining					();
	void		OnMove						(glm::vec3 _move, float dt, bool _noClip);

	void RotateAgent(glm::vec3 &_move, float _dt);


	//Behaviours
	void		BehaviourAttack				(float dt);
	void		BehaviourAutoAttack			(float dt);
	void		BehaviourFindGoals			(float dt);
	void		BehaviourStandard			(float dt);
	void		BehaviourMining				(float dt);
	void		BehaviourNoClip				(float dt);
	
	//Movement
	glm::vec3	SimplePathFind				();
	void		AdvancedPathFind			();
	void		Evade						(Agent* _agent);
	void		HandleBeingBlockedByAgent	(Agent* _agent);
	void		SetEvasionGoal				();

	void		StopAttackAnimation			();
};

