/**************************************************
Viktor Kelkkanen & Ola Enberg
***************************************************/

#pragma once
#include "stdafx.h"
#include "Agent.h"
#include "../subsystem/gfx/SSParticle.h"

#define MINING_RANGE_SQUAD_SQRD			900.0f
#define MINING_RANGE_AGENT_SQRD			900.0f
#define ATTACK_RANGE_AGENT_SQRD			400.0f
#define SQUAD_ATTACK_REPATH_TIME		4.0f
#define SQUAD_MINERS_PER_RESOURCE		4
#define SQUAD_SPACING_NR_OF_STEPS		3
#define SQUAD_SPACING_DYNAMIC_MINI		2
#define SQUAD_LINEWIDTH_DYNAMIC_MINI	8
#define SQUAD_HEDGEHOG_SPACING_1		2
#define SQUAD_HEDGEHOG_SPACING_2		12
#define SQUAD_HEDGEHOG_SPACING_3		24
#define SQUAD_POCKET_SPACING_1			8
#define SQUAD_POCKET_SPACING_2			16
#define SQUAD_POCKET_SPACING_3			24
#define SQUAD_UPGRADE_TIME				7.0f
#define SQUAD_UPGRADE_SMOKE_TIME_OUT	0.5f
#define SQUAD_MINUMUM_UNIT_COUNT		4
#define SQUAD_MAXIMUM_UNIT_COUNT		24
#define SQUAD_MAXIMUM_SQUAD_COUNT		10

class Squad
{
public:
	enum FORMATION	// TODOOE: Move NONE to after MAX and see if it simplifies things.
	{
		FORMATION_NONE=0,
		
		FORMATION_COLUMN,
		FORMATION_LINE,
		FORMATION_HEDGEHOG,
		FORMATION_LINE_OFFENSIVE,

		FORMATION_MAX,
	};

	enum MissionType
	{
		MISSION_TYPE_NONE,
		MISSION_TYPE_MOVE,
		MISSION_TYPE_MINE,
		MISSION_TYPE_HOLD,
		MISSION_TYPE_ATTACK,
		MISSION_TYPE_ATTACK_MOVE,
		MISSION_TYPE_UPGRADE,
		MISSION_TYPE_FOCUS_FIRE,
	};

	enum class MissionState
	{
		START = 0,
		RUN,
		END,
		REMOVE,
	};

	struct Mission
	{
		MissionType		Type;
		Tile*			Goal;
		MissionState	Progress;
		Entity			Target;
		void(Squad::*Start)(void);
		void(Squad::*Run)(float dt);
		void(Squad::*End)(void);
		Mission(MissionType _mt, Tile* _t, Entity _target) : Type(_mt), Goal(_t), Target(_target), Progress(MissionState::START)
		{
			if (Type == MissionType::MISSION_TYPE_MOVE)
			{
				Start	= &Squad::MoveMissionStart;
				Run		= &Squad::MoveMissionRun;
				End		= &Squad::StandardMissionEnd;
			}
			if (Type == MissionType::MISSION_TYPE_MINE)
			{
				Start	= &Squad::MoveMissionStart;
				Run		= &Squad::MiningMissionRun;
				End		= &Squad::MiningMissionEnd;
			}
			if (Type == MissionType::MISSION_TYPE_HOLD)
			{
				Start	= &Squad::HoldMissionStart;
				Run		= &Squad::EmptyMission;
				End		= &Squad::HoldMissionEnd;
			}
 			if (Type == MissionType::MISSION_TYPE_ATTACK)
 			{
 				Start	= &Squad::AttackMissionStart;
 				Run		= &Squad::AttackMissionRun;
 				End		= &Squad::StandardMissionEnd;
 			}
			if (Type == MissionType::MISSION_TYPE_UPGRADE)
			{
				Start	= &Squad::UpgradeMissionStart;
				Run		= &Squad::UpgradeMissionRun;
				End		= &Squad::UpgradeMissionEnd;
			}
			if (Type == MissionType::MISSION_TYPE_FOCUS_FIRE)
			{
				Start = &Squad::FocusFireMissionStart;
				Run = &Squad::EmptyMission;
				End = &Squad::EmptyMission;
			}
			if (Type == MissionType::MISSION_TYPE_ATTACK_MOVE)
			{
				Start = &Squad::MoveMissionStart;
				Run = &Squad::MoveMissionRun;
				End = &Squad::StandardMissionEnd;
			}
		}
	};

	Squad(int _team, int _id);
	~Squad();

	void					Update(float dt);

	void					FormationGoals(float dt);
	void					FindBuddies();
	void					AddAgent(Agent* _a);
	void					DeleteAgent(Agent* _a);
	void					SpawnUnit(glm::vec3 _host);
	void					SetFormation(FORMATION _form);
	void					AddSpacingStep(int _step);
	void					CycleFormation();
	void					RepathFormation();
	

	rVector<Agent*>			SplitSquad();

	//Use behaviour
	void					UseAutoAttack		(bool _attack, bool _toggle);
	void					UseHoldPosition		(bool _hold);

	//Get Set
	bool					CanMine( )						const { return m_CanMine; }
	bool					HasMoved( )						const { return m_HasMoved; }
	const bool				HasMission( )					const;
	Agent*					IsUnderAttack()					const;

	rString					GetFormationName()				const;
	FORMATION				GetFormation()					const	{ return m_Formation; }
	const glm::vec2&		GetPosition()					const	{ return m_Position; }
	const rVector<Goal>&	GetPath()						const	{ return m_Path; }
	const rVector<Agent*>&	GetAgents()						const	{ return m_Agents; }
	const rVector<glm::vec3>&	GetUnitsToSpawn()				const	{ return m_UnitsToSpawnFrom; }
	int						GetSize()						const	{ return (int)m_Agents.size(); }
	int						GetTeam()						const	{ return m_Team; }
	int						GetID()							const	{ return m_ID; }
	int						GetSpacingStep()				const	{ return m_SpacingStep; }
	int						GetResources()					const	{ return m_Resources; }
	bool					GetUseAutoAttack()				const	{ return m_UseAutoAttack; }
	bool					GetUseHoldPosition()			const	{ return m_UseHoldPosition; }
	float					GetUpgradeProgress()			const	{ return glm::clamp( 1.0f - ( m_UpgradeTimer / SQUAD_UPGRADE_TIME ), 0.0f, 1.0f ); }
	const rVector<Mission>&	GetMissions()					const	{ return m_Missions; }
	MissionType				GetCurrentMission();

	void					SetResources		(int val)			{ m_Resources = val; }
	void					SetSpacingStep		(int val);
	void					SetTarget			(Entity _target);
	void					SetTargetSquad		(Entity _target);
	void					SetUpgradeTo		(UPGRADE_NAME val)	{ m_UpgradeTo = val; }

	Agent*					GetLeader();

	void					PopHostUnit();

	//Missions
	void					AddMission(Tile* _t, MissionType _m, bool _override);
	bool					AllIdle() const;
	void					PopAllMissions();
	void					PopQueuedMissions();

	void					AllAgentsAddBehaviour(Agent::BehaviourType _behaviour);
	void					AllAgentsPopBehaviour(Agent::BehaviourType _behaviour);
	bool					CheckCanMine();
private:

	rVector<Mission>		m_Missions;
	rVector<Agent*>			m_Agents;
	rVector<glm::vec3>		m_UnitsToSpawnFrom;
	rVector<glm::vec2>		m_FormationPositions;
	rVector<int>			m_FormationAssignments;

	FORMATION				m_Formation;

	Tile*					m_Goal;

	glm::vec2				m_Forward;
	glm::vec2				m_FormationForward;
	glm::vec2				m_Position;
	glm::vec2				m_MissionProgressPosition;
	glm::vec3				m_Target;
	glm::vec3				m_MinPos;
	glm::vec3				m_MaxPos;

	bool					m_AttackMode;
	bool					m_UseAutoAttack;
	bool					m_UseHoldPosition;
	bool					m_IsUnderAttack;
	bool					m_HasMoved;
	bool					m_CanMine;

	float					m_AttackRepathTimer;
	float					m_UpgradeTimer;
	gfx::ParticleSystem*	m_UpgradeParticleSystem;

	int						m_SpacingStep;
	int						m_Resources;
	int						m_Team;
	int						m_TargetSquad;
	int						m_TargetTeam;
	int						m_Radius;
	int						m_ID;
	int						m_NextGoalTag;
	int						m_FormationHedgehogSpacing	[ SQUAD_SPACING_NR_OF_STEPS ];
	int						m_FormationPocketSpacing	[ SQUAD_SPACING_NR_OF_STEPS ];
	UPGRADE_NAME			m_UpgradeTo;

	rVector<Goal>			m_Path;


	//Formations
	void					CalculateFormationPositions		( rVector<glm::vec2>& outRelativePosition, const glm::vec2& forward, int allowedCost );	// TODOOE: Move formations to it's own class maybe?
	void					ColumnFormation					( rVector<glm::vec2>& outRelativePosition, int allowedCost );
	void					ColumnFormationSpecified		( rVector<glm::vec2>& outRelativePosition, int unitsPerRow, const glm::ivec2& spacing );
	void					HedgeHogFormation				( rVector<glm::vec2>& outRelativePosition, int allowedCost );
	void					LineFormation					( rVector<glm::vec2>& outRelativePosition, int allowedCost );
	void					LineFormationOffensive			( rVector<glm::vec2>& outRelativePosition, int allowedCost );

	int						CalculateFormationWidthInTiles	( FORMATION formation, int spacingStep );
	int						CalcColumnWidthInTiles			( int spacingStep );
	int						CalcHedgeHogWidthInTiles		( int spacingStep );
	int						CalcLineWidthInTiles			( int spacingStep );
	int						CalcLineOffensiveWidthInTiles	( int spacingStep );
	void					CalculateFormationAssignments	( const rVector<glm::vec2>& agentPositions, const rVector<glm::vec2>& formationPositions, const glm::vec2& offset, rVector<int>& outAssignments );
	

	void					BuildSquadPath		();
	void					BuildFormationPath	();


	//Missions
	void		ProgressMission();

	//Start
	void		MoveMissionStart();
	void		HoldMissionStart();
	void		AttackMissionStart();
	void		UpgradeMissionStart();
	void		FocusFireMissionStart();

	//Run
	void		MiningMissionRun(float dt);
	void		AttackMissionRun(float dt);
	void		MoveMissionRun(float dt);
	void		UpgradeMissionRun(float dt);

	//End
	void		StandardMissionEnd();
	void		MiningMissionEnd();
	void		AttackMissionEnd();
	void		HoldMissionEnd();
	void		UpgradeMissionEnd();

	void		EmptyMission(float dt);
	void		EmptyMission();
	void		MissionAccomplished();


	///SFX
	void		PlaySFX(const rString &path, glm::vec3 pos, const float distMin, const float distMax);

};
