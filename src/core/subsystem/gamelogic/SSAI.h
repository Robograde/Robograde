/**************************************************
Copyright 2015 Viktor Kelkkanen
***************************************************/

#pragma once

#include "../Subsystem.h"
#include "../../ai/Squad.h"
#include <memory/Alloc.h>
#include <script/ScriptEngine.h>
#include <messaging/Subscriber.h>
#include "../../ai/AIPlayer.h"
#include "../../ai/AICommands.h"

#define g_SSAI SSAI::GetInstance()

class SSAI : public Subsystem, public Subscriber
{
public:

	struct TeamAI
	{
		int					TeamID;
		rVector<Squad*>		Squads;

		TeamAI(int _id) :TeamID(_id){}
		TeamAI()
		{
			TeamID = -1;
		}
	};

	rVector<TeamAI>		m_Teams;

	static SSAI& GetInstance( );

	void Startup			( ) override;

	void SpawnPlayer(glm::vec3 spawnPoint, int player, bool isAI);
	void UpdateSimLayer(const float timeStep) override;
	void Shutdown			( ) override;

	int  SaveGenes();

	void AddSquad 			( Squad* squad);
	void AddAgent			( Agent* agent);
	void KillAgent			( Agent* agent);
	void IncreaseSquadSize	( int sizeIncrease, int squadID, int teamID );
	void DecreaseSquadSize	( int sizeDecrease, int squadID, int teamID );
	void ChangeSquadSize	( int newSize, int squadID, int teamID );
	void KillPlayer			( int teamID );

	void OrderUnits( const rVector<int>& _squads, const glm::vec3& _targetPosition, Entity _targetEntity, Squad::MissionType _mission, int _team, bool _override );
	void AddToSensor(Entity _entity, int _x, int _y, unsigned int _size);

	void InvokeAICommand(const rVector<int>& _squads, const AICommands _command, const int _team, int _value);

	rVector<int> GetSquadsFromSelection( const rVector<Entity>& _units, int* outTeamID, rVector<Entity>& outEntitiesInSquads ) const;
	Squad* GetSquadWithID(int _team, int _id) const;
	int GetFreeSquadID( ) const { return m_MaxSquadID + 1; }

	void PlayerLost(int id);
	void SetWinner(int id);

	//control spawn vars from lobby
	void SetSpawnCount(int val) { m_SpawnCount = val; }
	int GetSpawnCount() const {return m_SpawnCount; }

// 	void SetSpawnUpgradeTop(unsigned int val)	{ m_SpawnUpgradeTop = val; }
// 	void SetSpawnUpgradeSides(unsigned int val)	{ m_SpawnUpgradeSides = val; }
// 	void SetSpawnUpgradeFront(unsigned int val)	{ m_SpawnUpgradeFront = val; }
// 	void SetSpawnUpgradeBack(unsigned int val)	{ m_SpawnUpgradeBack = val; }
private:
	SSAI( );

	void KillSquad(const Squad* squad);

	void UpdateAIControl(float dt);

	int ScriptIncreaseSquadSize	( IScriptEngine* scriptEngine );
	int ScriptDecreaseSquadSize	( IScriptEngine* scriptEngine );
	int ScriptChangeSquadSize	( IScriptEngine* scriptEngine );
	int ScriptKillPlayer		( IScriptEngine* scriptEngine );

	int m_DeadAIplayers = 0;
	int m_MaxSquadID;

	const double MAX_TIME_AI = 3.0;

	const glm::vec4 m_MarkerAttackColour 		= glm::vec4( 0.7f, 0.0f, 0.0f, 1.0f );
	const glm::vec4 m_MarkerAttackMoveColour 	= glm::vec4( 0.7f, 0.0f, 0.0f, 1.0f );
	const glm::vec4 m_MarkerMiningColour 		= glm::vec4( 0.8f, 0.8f, 0.0f, 1.0f );
	const glm::vec4 m_MarkerMoveColour 			= glm::vec4( 0.8f, 0.8f, 0.0f, 1.0f );

	//spawn vars
	int m_SpawnCount = 4;
	unsigned int m_SpawnUpgradeTop = UPGRADE_NAME::EMPTY;
	unsigned int m_SpawnUpgradeSides = UPGRADE_NAME::MOVEMENT_LEGS;
	unsigned int m_SpawnUpgradeFront = UPGRADE_NAME::WEAPON_PINCERS;
	unsigned int m_SpawnUpgradeBack = UPGRADE_NAME::EMPTY;
#if AI_BALANCE_TEST == 1
	void BalanceTest(int upg1, int upg2);

	int m_CurrentTestUpg1 = 0;
	int m_CurrentTestUpg2 = 3;
#endif
	rVector<AIPlayer*> m_AIPlayers;
};
