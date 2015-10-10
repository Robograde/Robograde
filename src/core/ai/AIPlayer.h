/**************************************************
2015 Viktor Kelkkanen
***************************************************/

#pragma once
#include "Squad.h"
#include "NNetwork.h"
#include <messaging/Subscriber.h>

class AIPlayer : public Subscriber
{
public:
	enum PERCEPTION
	{
		PERCEPTION_UNDER_ATTACK,
		PERCEPTION_KNOWN_FARM,
		PERCEPTION_SIZE_SELF,
		PERCEPTION_SIZE_ENEMY,
		PERCEPTION_AVAILABLE_UPGRADES,
		PERCEPTION_UPGRADE_VALUE,
		PERCEPTION_OWNED_CONTROL_POINTS,
		//PERCEPTION_TIME,
		PERCEPTION_NUMBER_OF_PERCEPTIONS,
		
	};

	enum DECISION
	{
		DECISION_ATTACK,
		DECISION_FARM,
		DECISION_SCOUT,
		DECISION_RETREAT,
		DECISION_UPGRADE,
		DECISION_NUMBER_OF_DECISIONS,
	};

	AIPlayer(int team, bool useNeuralNet);
	~AIPlayer();
	void Update(float dt);

	void CheckBeingAttacked();

	void AddSquad(Squad* squad);
	void DeleteSquad(int id);

	int GetTeamID() const { return m_Team; }
	int GetSize();
	int GetGeneFitness() const { return m_Genes.Fitness; }
	bool GetIsNeuralNetAI() const { return m_IsNeuralNetAI; }
	void SetGeneFitness(int val);
	void SetScaledFitness(int val);
	void SetIsWinner(bool val) { m_IsWinner = val; }
	bool IsWinner() const { return m_IsWinner; }

private:
	bool m_IsWinner = false;
	bool m_IsNeuralNetAI = false;
	bool m_AllResearchFinished = false;
	bool m_IsUnderAttack = false;

	int m_NInPuts;
	int m_NOutPuts;
	double m_TotalErrorSum=0;

	rVector<float> m_Perceptions;
	rVector<float> m_Outputs;
	rVector<float> m_WantedOutputs;
	rVector<float> m_TrainingVector;

	rVector<rVector<rVector<float>>> m_SavedPerceptions;
	rVector<rVector<rVector<bool>>>	m_SavedDecisions;

	rVector<bool> m_Decisions;

	NeuralNet*    m_Net;

	static const int SCOUT_AREA_SIZE = 16;

	struct FoodLocation
	{
		Tile*	TilePointer;
		int		ClaimedBySquad;
	};

	struct Knowledge
	{
		rVector<FoodLocation>	FoodLocations;
		rVector<Tile*>	EnemyLocations;
		int				GeneralSearchRange = 30;
		float			LastUpdateWorldView = 0.0f;
		float			DelayUpdateWorldView = 4.7f;

		float			TimeSinceDecision = 75.0f;
		float			DecisionCoolDown = 75.0f;

		float			TimeSinceAttacked = 0;
		float			TimeSinceRandomDecision = 0;
		int				SquadIDUnderAttack = -1;
		Entity			AgentUnderAttack = -1;

		unsigned int	CurrentUpgradeLoopIndex = 0;
		int				LastKnownEnemySize = 0;
	};

	struct Genes
	{
		int						SplitOnSquadSize = 8;
		int						Fitness = 10;
		int						SquadCoherence = 300;
		int						MiningSlotsPerSquad = 7;
		int						DistanceToClaimableFood = 20;
		int						IsUnderAttackReset = 5;

		int						ScaledFitness = 0;
		int						SquadValueLowerLimit = 100;
		
		rVector<unsigned int>	PreferredUpgrade;
	};



	enum class VALUE_REASON
	{
		VALUE_REASON_CONTROL_POINT,
		VALUE_REASON_CHOKE,
		VALUE_REASON_LEARNED,
		VALUE_REASON_OTHER,
	};

	struct ValuableArea
	{
		Tile*			TilePointer = nullptr;
		int				Radius = 0;
		VALUE_REASON	Reason = VALUE_REASON::VALUE_REASON_OTHER;
		int				Value = 0;
		int				ClaimedBySquad;
		Entity			Target;
	};

	Knowledge	m_Brain;
	Genes		m_Genes;

	rVector<ValuableArea> m_ValuableAreas;
	rVector<Squad*> m_Squads;
	int m_Team;

	UPGRADE_NAME m_CurrentUpgrade = UPGRADE_NAME::MOVEMENT_TRACKS;

	void UpdateWorldView();

 	void SaveGenes();
 	void LoadGenes();
	void SaveDebugData();

	void SoftTraining(int id);

	void ForbiddenDecisions(bool& useTraining, int id);

	void GetNetOutput();

	void HardTrainingData();

	bool FindFood(Tile* tile);
	bool FindEnemy(Tile* tile);

	bool DecideToRetreat(Squad* squad);
	void DecideToScout(Squad* squad);
	void DecideToSplit(Squad* squad);
	bool DecideToAttack(Squad* squad);
	bool DecideToFarm(Squad* squad);
	void DecideToUpgrade(Squad* squad);

	void FindResearch();

	void GenericMoveMission(Squad* squad, Tile* tile);
};

