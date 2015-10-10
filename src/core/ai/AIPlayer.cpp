/**************************************************
Viktor Kelkkanen
***************************************************/

#include "AIPlayer.h"
#include <utility/Randomizer.h>
#include <messaging/GameMessages.h>
#include "Terrain.h"
#include "../component/ResourceComponent.h"
#include "../component/OwnerComponent.h"
#include "../component/PlacementComponent.h"
#include "../utility/GameData.h"
#include "../subsystem/utility/SSMail.h"
#include "../subsystem/gamelogic/SSControlPoint.h"
#include "../subsystem/input/SSInput.h"
#include "../ai/AICommands.h"
#include "../subsystem/gamelogic/SSUpgrades.h"
#include "../subsystem/gamelogic/SSResearch.h"
#include "../ai/AIMessages.h"
#include "../utility/Alliances.h"

AIPlayer::AIPlayer(int team,bool useNeuralNet): Subscriber("AIPlayer" + rToString( team ) )
{
	m_IsNeuralNetAI = useNeuralNet;
	m_Team = team;

	rVector<ResourceComponent*> knownStaticResources = Terrain::GetInstance()->GetResourceList();

	for (int i = 0; i < knownStaticResources.size(); i++)
	{
		FoodLocation food;
		food.TilePointer = knownStaticResources[i]->TilePointer;
		food.ClaimedBySquad = -1;
		m_Brain.FoodLocations.push_back(food);
#if AI_RENDER_DEBUG == 1
		knownStaticResources[i]->TilePointer->DebugInfo = 5;
#endif
	}


	rVector<Entity> controlPoints = g_SSControlPoint.GetControlPoints();
	float shortest = 5000;
	glm::vec3 squadPos;

	squadPos = g_GameData.GetSpawnPoints()[m_Team];

	glm::vec3 positionToHold;
	int numberOfControlPoints = static_cast<int>(controlPoints.size());
	for (int i = 0; i < numberOfControlPoints; i++)
	{
		OwnerComponent* cpc = GetDenseComponent<OwnerComponent>(controlPoints[i]);

		if (cpc->OwnerID == m_Team)
			continue;

		glm::vec3 controlPointPos = GetDenseComponent<PlacementComponent>(controlPoints[i])->Position;

		int dist = static_cast<int>(glm::distance(controlPointPos, squadPos));

		Tile* t = Terrain::GetInstance()->GetTile(controlPointPos.x + 5, controlPointPos.z + 5);

		ValuableArea valArea;
		valArea.Radius = 30;
		valArea.Reason = VALUE_REASON::VALUE_REASON_CONTROL_POINT;
		valArea.TilePointer = t;
		valArea.Value = dist*(2 - (cpc->OwnerID != m_Team));
		valArea.ClaimedBySquad = -1;
		valArea.Target = controlPoints[i];

		m_ValuableAreas.push_back(valArea);
	}

	m_Brain.DelayUpdateWorldView  += m_Team*0.1f;

	m_NInPuts = PERCEPTION_NUMBER_OF_PERCEPTIONS;
	m_NOutPuts = DECISION_NUMBER_OF_DECISIONS;


	m_Genes.PreferredUpgrade.resize(5);
	for (int i = 0; i < m_Genes.PreferredUpgrade.size();i++)
	{
		m_Genes.PreferredUpgrade[i] = i;
	}

	LoadGenes();

// 	rVector<UpgradeData> upgrades = g_SSUpgrades.GetUpgradeArray();
// 	
// 	for (int i = 0; i < upgrades.size(); i++)
// 	{
// 
// 		m_Genes.PreferredUpgrade.push_back(1);
// 	}

	m_Decisions.resize(m_NOutPuts);
	m_Perceptions.resize(m_NInPuts);
	m_WantedOutputs.resize(m_NOutPuts);
	m_Outputs.resize(m_NOutPuts);
	m_TrainingVector.resize(m_NOutPuts);

	if (m_IsNeuralNetAI)
	{
		m_Net = pNew(NeuralNet, m_NInPuts, m_NOutPuts, 2, m_NInPuts, m_Team);
		m_Net->ReadWeights();
	}

	m_SimInterests = MessageTypes::AI_MSG;
	g_SSMail.RegisterSubscriber(this);
}

AIPlayer::~AIPlayer()
{
	//SaveGenes();
	//SaveDebugData();

	if (m_IsNeuralNetAI)
	{
		if (m_IsWinner && AI_TRAIN_NEURAL_NETS)
			m_Net->WriteWeights();

		pDelete(m_Net);
	}

	g_SSMail.UnregisterSubscriber(this);
}

void AIPlayer::Update(float dt)
{
	if (m_Squads.size() == 0)
		return;

	//Return fire
	CheckBeingAttacked();

	m_Brain.LastUpdateWorldView += dt;

	//Update view of life
	if (m_Brain.LastUpdateWorldView > m_Brain.DelayUpdateWorldView)
	{
		UpdateWorldView();
		m_Brain.LastUpdateWorldView = 0;
	}

	m_Perceptions[PERCEPTION_KNOWN_FARM] = static_cast<float>(m_Brain.FoodLocations.size());

	m_Perceptions[PERCEPTION_OWNED_CONTROL_POINTS] = static_cast<float>(g_SSControlPoint.GetOwnedControlPointsCount(m_Team));

	rVector<bool> currentDecisions = m_Decisions;

	if (m_Perceptions[PERCEPTION_UNDER_ATTACK])
	{
		m_Brain.TimeSinceAttacked += dt;
		if (m_Brain.TimeSinceAttacked > m_Genes.IsUnderAttackReset)
		{
			m_Brain.TimeSinceAttacked = 0;
			m_Perceptions[PERCEPTION_UNDER_ATTACK] = 0;
			m_Brain.SquadIDUnderAttack = -1;
		}
	}

	m_Brain.TimeSinceDecision += dt;

	Terrain::EnemyZone zone = Terrain::GetInstance()->GetClosestEnemy(Terrain::GetInstance()->GetTile(0,0), 1000, m_Team, 0);

	for (int squad = 0; squad < m_Squads.size();squad++)
	{
		if (!m_Squads[squad]->HasMission() || m_Brain.TimeSinceDecision > m_Brain.DecisionCoolDown || zone.Amount != m_Brain.LastKnownEnemySize || m_Brain.TimeSinceAttacked == dt)
		{
			m_Brain.TimeSinceDecision = 0;

			m_Perceptions[PERCEPTION_SIZE_SELF] = static_cast<float>(m_Squads[squad]->GetSize());

			m_Perceptions[PERCEPTION_UPGRADE_VALUE] = static_cast<float>(g_SSUpgrades.GetUnitValue(m_Squads[squad]->GetLeader()->GetEntityID())) - UNIT_DEFAULT_VALUE;

			m_Perceptions[PERCEPTION_AVAILABLE_UPGRADES] = 0;

			UPGRADE_NAME chosenUpg = UPGRADE_NAME::EMPTY;

			int indexStart = m_Brain.CurrentUpgradeLoopIndex - 1;

			if (indexStart < 0)
				indexStart = 0;

			bool tryTwoLoops = false;

			if (indexStart != 0)
				tryTwoLoops = true;

			for (int i = indexStart; i < m_Genes.PreferredUpgrade.size(); i++)
			{
				if (g_SSResearch.GetPlayerResearchDone(m_Team, static_cast<UPGRADE_NAME>(m_Genes.PreferredUpgrade[i])))
				{
					if (!g_SSUpgrades.SquadHasUpgrade(m_Team, m_Squads[squad]->GetID(), m_Genes.PreferredUpgrade[i]))
					{
						UpgradeData data = g_SSUpgrades.GetUpgrade(m_Genes.PreferredUpgrade[i]);

						//no overwrite cheating
						if (!g_SSUpgrades.SquadSlotFree(m_Team, m_Squads[squad]->GetID(), data.Slot))
						{
							continue;
						}

						m_Perceptions[PERCEPTION_AVAILABLE_UPGRADES]++;
					}
				}

				if (i == m_Genes.PreferredUpgrade.size() - 1 && tryTwoLoops)
				{
					i = 0;
					tryTwoLoops = false;
				}
			}

// 			int slotsTaken = 0;
// 			ParentComponent* parentComponent = GetDenseComponent<ParentComponent>(m_Squads[squad]->GetLeader()->GetEntityID());
// 			for (Entity slot : parentComponent->Children)
// 			{
// 				if (slot != ENTITY_INVALID)
// 				{
// 					slotsTaken++;
// 				}
// 			}
// 
// 			if (slotsTaken == 4)
// 				m_Perceptions[PERCEPTION_AVAILABLE_UPGRADES] = 0;
// 			else
// 			{
// 				if (m_AllResearchFinished)
// 				{
// 					m_Perceptions[PERCEPTION_AVAILABLE_UPGRADES] = static_cast<float>(m_Genes.PreferredUpgrade.size());
// 				}
// 				else
// 				{
// 					for (int i = 0; i < m_Genes.PreferredUpgrade.size(); i++)
// 					{
// 						if (g_SSResearch.GetPlayerResearchDone(m_Team, static_cast<UPGRADE_NAME>(m_Genes.PreferredUpgrade[i])) && g_SSUpgrades.SquadSlotFree(m_Team, m_Squads[squad]->GetID(), g_SSUpgrades.GetUpgradeSlot(static_cast<UPGRADE_NAME>(m_Genes.PreferredUpgrade[i]))))
// 							m_Perceptions[PERCEPTION_AVAILABLE_UPGRADES]++;
// 					}
// 				}
// 			}

			FindEnemy(m_Squads[squad]->GetLeader()->GetTile());

			//Train
			if (m_IsNeuralNetAI)
			{
				GetNetOutput();

				if (AI_TRAIN_NEURAL_NETS)
				{
					SoftTraining(squad);
				}
			}
			else
				HardTrainingData();

// 				if (PRINT_AI_PLAYER)
// 					printf("team: %d squad %d | At: %d, F: %d, Sc: %d, Rt: %d, Sp: %d, Up: %d\n", m_Team,squad->GetID(), m_Decisions[DECISION_ATTACK] ? 1 : -1, m_Decisions[DECISION_FARM] ? 1 : -1, m_Decisions[DECISION_SCOUT] ? 1 : -1, m_Decisions[DECISION_RETREAT] ? 1 : -1, m_Decisions[DECISION_SPLIT] ? 1 : -1, m_Decisions[DECISION_UPGRADE] ? 1 : -1);


			if (!m_IsNeuralNetAI)
			{
				for (int i = 0; i < m_Decisions.size(); i++)
				{
					m_Decisions[i] = (m_WantedOutputs[i] == 1);
				}
			}

			if (m_Perceptions[PERCEPTION_SIZE_SELF] >= SQUAD_MAXIMUM_UNIT_COUNT && m_Squads.size() < SQUAD_MAXIMUM_SQUAD_COUNT)
				DecideToSplit(m_Squads[squad]);

			if (m_Decisions[DECISION_UPGRADE])
				DecideToUpgrade(m_Squads[squad]);
			else if (m_Decisions[DECISION_RETREAT])
				DecideToRetreat(m_Squads[squad]);
			else if (m_Decisions[DECISION_ATTACK])
				DecideToAttack(m_Squads[squad]);
			else if (m_Decisions[DECISION_FARM])
				DecideToFarm(m_Squads[squad]);
			else if (m_Decisions[DECISION_SCOUT])
				DecideToScout(m_Squads[squad]);

			m_SavedPerceptions[squad].push_back(m_Perceptions);
			m_SavedDecisions[squad].push_back(m_Decisions);

			for (int i = 0; i < m_TrainingVector.size(); i++)
			{
				m_TrainingVector[i] = 0.0f;
			}
		}
	}
	
	m_Brain.LastKnownEnemySize = zone.Amount;

#if AI_DRAW_NEURAL_NETS == 1
	if (m_IsNeuralNetAI)
	{
		rString decision = "None";
		if (m_Decisions[DECISION_ATTACK])
			decision = "Attack";
		else if (m_Decisions[DECISION_FARM])
			decision = "Farm";
		else if (m_Decisions[DECISION_SCOUT])
			decision = "Scout";
		else if (m_Decisions[DECISION_RETREAT])
			decision = "Retreat";
// 		else if (m_Decisions[DECISION_SPLIT])
// 			decision = "Split";
		else if (m_Decisions[DECISION_UPGRADE])
			decision = "Upgrade";

		rString text = 
			"Player: " + rToString(m_Team) + 
			"\nDecision: " + decision + 
			"\nFitness: " + rToString(m_Genes.Fitness) + 
			"\nError: " + rToString(m_Net->GetError());
		m_Net->SetText(text);
	}
#endif
}

void AIPlayer::GetNetOutput()
{
	m_Net->Use(m_Perceptions, m_Outputs);

	int bestDecision = 0;
	float highestVoltage = -1;

	for (int i = 0; i < m_NOutPuts; i++)
	{
		float decision = m_Outputs[i];

		if (decision > highestVoltage)
		{
			highestVoltage = decision;
			bestDecision = i;
		}

		m_Decisions[i] = 0;
	}

// 	m_Decisions[DECISION_SPLIT]		= (m_Outputs[DECISION_SPLIT] > 0.9f);
// 	m_Decisions[DECISION_UPGRADE]	= (m_Outputs[DECISION_UPGRADE] > 0.9f);
	m_Decisions[bestDecision] = 1;

//   	if (PRINT_AI_PLAYER)
//   		printf("team: %d | At: %.2f, F: %.2f, Sc: %.2f, Rt: %.2f, Sp: %.2f, Up: %.2f\n", m_Team, m_Outputs[DECISION_ATTACK], m_Outputs[DECISION_FARM], m_Outputs[DECISION_SCOUT], m_Outputs[DECISION_RETREAT], m_Outputs[DECISION_SPLIT], m_Outputs[DECISION_UPGRADE]);
}

void AIPlayer::AddSquad(Squad* squad)
{
	m_Squads.push_back(squad);
	squad->UseAutoAttack(true, false);
	UpdateWorldView();

	m_SavedDecisions.resize(m_Squads.size());
	m_SavedPerceptions.resize(m_Squads.size());
}

void AIPlayer::DeleteSquad(int id)
{
	int index = 0;
	for (auto it = m_Squads.begin(); it < m_Squads.end(); it++)
	{
		if ((*it)->GetID() == id)
		{
			m_SavedDecisions.erase(m_SavedDecisions.begin()+index);
			m_SavedPerceptions.erase(m_SavedPerceptions.begin() + index);
			m_Squads.erase(it);
			break;
		}
		index++;
	}
}

bool AIPlayer::FindEnemy(Tile* tile)
{
	if (m_Perceptions[PERCEPTION_UNDER_ATTACK] > 0)
	{
		Agent* agentPointer = GetAgentPointer(m_Brain.AgentUnderAttack);
		if (agentPointer != nullptr)
		{
			tile = agentPointer->GetTile();
		}
	}

	Terrain::EnemyZone zone = Terrain::GetInstance()->GetClosestEnemy(tile, m_Brain.GeneralSearchRange, m_Team,0);

	//m_UpdateID = zone.UpdateID;

	if (zone.Closest != ENTITY_INVALID)
	{
		Tile* t = GetAgentPointer(zone.Closest)->GetTile();

		int dist = Terrain::GetInstance()->GetTileDistance(t, tile);

		m_Perceptions[PERCEPTION_SIZE_ENEMY] = static_cast<float>(zone.Amount);

		//m_Perceptions[PERCEPTION_CLOSEST_ENEMY] = static_cast<float>(dist);

		m_Brain.EnemyLocations.push_back(tile);

		//if (PRINT_AI_PLAYER)
		//	printf("team: %d | enemy location:(%d,%d)\n", m_Team, m_Brain.EnemyLocations.back()->X, m_Brain.EnemyLocations.back()->Y);
		return true;
	}
	m_Perceptions[PERCEPTION_SIZE_ENEMY] = 0;
	return false;
}

bool AIPlayer::FindFood(Tile* tile)
{
	Entity e = Terrain::GetInstance()->GetClosestResource(tile, m_Brain.GeneralSearchRange);
	if (HasComponent<ResourceComponent>(e))
	{
		ResourceComponent* rc = GetDenseComponent<ResourceComponent>(e);
		FoodLocation food;
		food.TilePointer = rc->TilePointer;
		food.ClaimedBySquad = -1;

		m_Brain.FoodLocations.push_back(food);
		if (PRINT_AI_PLAYER)
			printf("team: %d | food location:(%d,%d)\n", m_Team, m_Brain.FoodLocations.back().TilePointer->X, m_Brain.FoodLocations.back().TilePointer->Y);
		return true;
	}
	return false;
}

void AIPlayer::DecideToScout(Squad* squad)
{
// 	if (PRINT_AI_PLAYER)
// 		printf("team: %d | scout\n", m_Team);

	Tile* t;

	if (m_Perceptions[PERCEPTION_SIZE_ENEMY] > 0)
	{
		m_TrainingVector[DECISION_SCOUT] = -2;
	}

	int valuedAreas = static_cast<int>(m_ValuableAreas.size());

	if (valuedAreas > 0)
	{
		int maxValue = -1000;
		int bestIndex = 0;
		for (int i = 0; i < valuedAreas; i++)
		{
			int valuedTo = m_ValuableAreas[i].Value;

			int dist = Terrain::GetInstance()->GetTileDistance(m_ValuableAreas[i].TilePointer, squad->GetLeader()->GetTile());

			valuedTo = valuedTo * 300 - dist - (m_ValuableAreas[i].ClaimedBySquad != squad->GetID())*m_Genes.SquadCoherence;

			if (valuedTo > maxValue)
			{
				maxValue = valuedTo;
				bestIndex = i;
			}
		}
		t = m_ValuableAreas[bestIndex].TilePointer;
		m_ValuableAreas[bestIndex].ClaimedBySquad = squad->GetID();
	}

	squad->AddMission(t, Squad::MissionType::MISSION_TYPE_ATTACK_MOVE, true);

}

bool AIPlayer::DecideToRetreat(Squad* squad)
{
	Terrain::EnemyZone ez = Terrain::GetInstance()->GetClosestEnemy(squad->GetLeader()->GetTile(), m_Brain.GeneralSearchRange, m_Team, 0);

	if (ez.Closest != ENTITY_INVALID)
	{
		Agent* closeEnemy = GetAgentPointer(ez.Closest);
		glm::vec3 retreatTo = closeEnemy->GetForward();
		retreatTo *= 100;
		Tile* t = squad->GetLeader()->GetTile();
		t = Terrain::GetInstance()->GetMovableTile(retreatTo.x + t->X, retreatTo.y + t->Y, ENTITY_INVALID, t);
		GenericMoveMission(squad, t);
		if (PRINT_AI_PLAYER)
			printf("team: %d | retreating\n", m_Team);
		return true;
	}
// 	if (PRINT_AI_PLAYER)
// 		printf("team: %d | bad retreat call\n", m_Team);

	//m_IsMakingBadDecision = true;
	m_TrainingVector[DECISION_RETREAT] = -2;
	
	return false;
}

bool AIPlayer::DecideToAttack(Squad* squad)
{
	if (m_Brain.EnemyLocations.size() == 0)
	{
// 		if (PRINT_AI_PLAYER)
// 			printf("team: %d | bad attack call\n", m_Team);
		m_TrainingVector[DECISION_ATTACK] = -2;
		//m_IsMakingBadDecision = true;
		return false;
	}

	Tile* leaderTile = squad->GetLeader()->GetTile();
	Terrain::EnemyZone ez = Terrain::GetInstance()->GetClosestEnemy(m_Brain.EnemyLocations.back(), m_Brain.GeneralSearchRange, m_Team, 0);

	if (ez.Closest != ENTITY_INVALID)
	{
		Tile* tile = GetAgentPointer(ez.Closest)->GetTile();
		squad->AddMission(tile, Squad::MissionType::MISSION_TYPE_ATTACK_MOVE, true);

		squad->SetSpacingStep(1);
		if (PRINT_AI_PLAYER)
			printf("team: %d | successful attack\n", m_Team);

		return true;
	}
	else if (Terrain::GetInstance()->GetTileDistance(leaderTile, m_Brain.EnemyLocations.back()) < m_Brain.GeneralSearchRange)
	{
		m_Brain.EnemyLocations.pop_back();
	}
	else
	{
		squad->UseAutoAttack(true, false);
		squad->AddMission(m_Brain.EnemyLocations.back(), Squad::MissionType::MISSION_TYPE_ATTACK_MOVE, true);
	}

 	if (PRINT_AI_PLAYER)
		printf("team: %d | no known enemy areas\n", m_Team);
	
	return false;
}

bool AIPlayer::DecideToFarm(Squad* squad)
{
	if (PRINT_AI_PLAYER)
		printf("team: %d | farm\n", m_Team);

	if (m_Brain.FoodLocations.size() == 0)
	{
//  		if (PRINT_AI_PLAYER)
//  			printf("team: %d | bad farm call\n", m_Team);
		m_TrainingVector[DECISION_FARM] = -2;

		//m_IsMakingBadDecision = true;
		return false;
	}

	int shortest = 5000;
	int indexBestFood = 0;
	for (int i = 0; i < m_Brain.FoodLocations.size(); i++)
	{
		int claimedBy = m_Brain.FoodLocations[i].ClaimedBySquad;

		if (claimedBy == -1 || claimedBy == squad->GetID())
		{
			int dist = Terrain::GetInstance()->GetTileDistance(m_Brain.FoodLocations[i].TilePointer, squad->GetLeader()->GetTile());
			if (dist < shortest)
			{
				shortest = dist;
				indexBestFood = i;
				if (dist < 10)
				{
					break;
				}
			}
		}
	}
	int amountClaimed = 0;
	for (int i = 0; i < m_Brain.FoodLocations.size(); i++)
	{
		int dist = Terrain::GetInstance()->GetTileDistance(m_Brain.FoodLocations[indexBestFood].TilePointer, m_Brain.FoodLocations[i].TilePointer);

		//already claimed on previous run, break
		if (m_Brain.FoodLocations[i].ClaimedBySquad == squad->GetID())
			break;

		if (dist < m_Genes.DistanceToClaimableFood && m_Brain.FoodLocations[i].ClaimedBySquad == -1)
		{
			m_Brain.FoodLocations[i].ClaimedBySquad = squad->GetID();
			amountClaimed++;
		}

		if (amountClaimed > m_Genes.MiningSlotsPerSquad)
			break;
	}

	m_Brain.FoodLocations[indexBestFood].ClaimedBySquad = squad->GetID();

	Entity e = Terrain::GetInstance()->GetClosestResource(m_Brain.FoodLocations[indexBestFood].TilePointer, m_Brain.GeneralSearchRange);
	if (HasComponent<ResourceComponent>(e))
	{
		ResourceComponent* rc = GetDenseComponent<ResourceComponent>(e);
		squad->AddMission(rc->TilePointer, Squad::MissionType::MISSION_TYPE_MINE, true);
		return true;
	}
	if (PRINT_AI_PLAYER)
	printf("failed to find farm\n");
	m_TrainingVector[DECISION_FARM] = -2;
	return false;
}

void AIPlayer::GenericMoveMission(Squad* squad,Tile* tile)
{
	squad->UseAutoAttack(false, false);
	squad->AddMission(tile, Squad::MissionType::MISSION_TYPE_MOVE, true);
}

void AIPlayer::UpdateWorldView()
{
	m_Brain.LastUpdateWorldView = 0;

	for (int i = 0; i < m_ValuableAreas.size(); i++)
	{
		OwnerComponent* cpc = GetDenseComponent<OwnerComponent>(m_ValuableAreas[i].Target);

		m_ValuableAreas[i].Value = (g_Alliances.IsAllied(cpc->OwnerID, m_Team) == false) - 1 + (cpc->OwnerID == NO_OWNER_ID);
		m_ValuableAreas[i].ClaimedBySquad = -1;
	}

	if (m_Brain.FoodLocations.size() > 0)
	{
		for (int i = static_cast<int>(m_Brain.FoodLocations.size()) - 1; i > -1; i--)
		{
			if (Terrain::GetInstance()->WhoIsOnTile(m_Brain.FoodLocations[i].TilePointer) == ENTITY_INVALID)
			{
//#if AI_RENDER_DEBUG == 1
//				m_Brain.FoodLocations[i]->DebugInfo = 0;
//#endif
				m_Brain.FoodLocations.erase(m_Brain.FoodLocations.begin()+i);
			}
		}
	}

	if (m_Brain.EnemyLocations.size() > 0)
	{
		for (int i = static_cast<int>(m_Brain.EnemyLocations.size()) - 1; i > -1; i--)
		{
			if (Terrain::GetInstance()->WhoIsOnTile(m_Brain.EnemyLocations[i]) == ENTITY_INVALID)
			{
#if AI_RENDER_DEBUG == 1
				m_Brain.EnemyLocations[i]->DebugInfo = 0;
#endif
				m_Brain.EnemyLocations.erase(m_Brain.EnemyLocations.begin() + i);
			}
		}
	}

	if (m_Brain.FoodLocations.size() < 10)
	{
		m_Brain.FoodLocations.clear();

		rVector<ResourceComponent*> knownStaticResources = Terrain::GetInstance()->GetResourceList();

		for (int i = 0; i < knownStaticResources.size(); i++)
		{
			FoodLocation food;
			food.TilePointer = knownStaticResources[i]->TilePointer;
			food.ClaimedBySquad = -1;
			m_Brain.FoodLocations.push_back(food);
#if AI_RENDER_DEBUG == 1
			knownStaticResources[i]->TilePointer->DebugInfo = 5;
#endif
		}
	}

	if (!m_AllResearchFinished && m_Perceptions[PERCEPTION_OWNED_CONTROL_POINTS] > 0)
		FindResearch();
}

void AIPlayer::DecideToSplit(Squad* squad)
{
// 	if (PRINT_AI_PLAYER)
// 		printf("team %d split\n", m_Team);

// 	if (m_Perceptions[PERCEPTION_SIZE_SELF] < m_Genes.SplitOnSquadSize)
// 		m_TrainingVector[DECISION_SPLIT] = -2;

	rVector<int> squadVector;
	squadVector.push_back(squad->GetID());
	OrderInvokeMessage message = OrderInvokeMessage(g_GameData.GetFrameCount(), squadVector, AICommands::SQUAD_SPLIT, m_Team, 0);
	g_SSMail.PushToNextFrame(message);
}

void AIPlayer::DecideToUpgrade(Squad* squad)
{
	if (PRINT_AI_PLAYER)
		printf("team %d upgrade\n", m_Team);

	UPGRADE_NAME chosenUpg = UPGRADE_NAME::EMPTY;

	int indexStart = m_Brain.CurrentUpgradeLoopIndex - 1;

	if (indexStart < 0)
		indexStart = 0;

	bool tryTwoLoops = false;

	if (indexStart != 0)
		tryTwoLoops = true;

	for (int i = indexStart; i < m_Genes.PreferredUpgrade.size(); i++)
	{
		if (g_SSResearch.GetPlayerResearchDone(m_Team, static_cast<UPGRADE_NAME>(m_Genes.PreferredUpgrade[i])))
		{
			if (!g_SSUpgrades.SquadHasUpgrade(m_Team, squad->GetID(), m_Genes.PreferredUpgrade[i]))
			{
				UpgradeData data = g_SSUpgrades.GetUpgrade(m_Genes.PreferredUpgrade[i]);
				
				//no overwrite cheating
				if (!g_SSUpgrades.SquadSlotFree(m_Team, squad->GetID(), data.Slot))
				{
					continue;
				}

				chosenUpg = static_cast<UPGRADE_NAME>(m_Genes.PreferredUpgrade[i]);
				break;
			}
		}

		if (i == m_Genes.PreferredUpgrade.size() - 1 && tryTwoLoops)
		{
			i = 0;
			tryTwoLoops = false;
		}
	}
	if (chosenUpg != UPGRADE_NAME::EMPTY)
	{
		rVector<int> squadVector;
		squadVector.push_back(squad->GetID());
		UpgradeMessage message = UpgradeMessage(g_GameData.GetFrameCount(), m_Team, squadVector, chosenUpg);
		g_SSMail.PushToNextFrame(message);
		if (PRINT_AI_PLAYER)
			printf("team %d: Upgrade mission message, upgrade: %d, index: %d\n", m_Team, chosenUpg, m_Brain.CurrentUpgradeLoopIndex);

		UpdateWorldView();
	}
	else
	{
		 if (PRINT_AI_PLAYER)
		 	printf("team %d: no upgrade available\n", m_Team);
		 m_TrainingVector[DECISION_UPGRADE] = -2;
	}
}

void AIPlayer::SaveDebugData()
{
	FILE* pFile;
	rString filePath = "../../../asset/ai/debuginfo" + rToString(m_Team) + ".txt";
	if ((pFile = fopen(filePath.c_str(), "a")) == NULL)
		return;

	fprintf(pFile, "\n%f", m_TotalErrorSum);

	fclose(pFile);
}

void AIPlayer::SaveGenes()
{
	FILE* pFile;
	rString filePath = "../../../asset/ai/genes" + rToString(m_Team) + ".txt";
	if ((pFile = fopen(filePath.c_str(), "w")) == NULL)
		return;

//	fprintf(pFile, "%d ", m_Genes.ScaledFitness);

	for (int i = 0; i < m_Genes.PreferredUpgrade.size(); i++)
	{
		fprintf(pFile, "%d ", m_Genes.PreferredUpgrade[i]);
	}

	fclose(pFile);
}

//---------------------------------------------------------
void AIPlayer::LoadGenes()
{
	FILE* pFile;
	rString filePath = "../../../asset/ai/genes" + rToString(m_Team) + ".txt";
	if ((pFile = fopen(filePath.c_str(), "r")) == NULL)
	{
		printf("Failed to read genes!\n");
		return;
	}

//	int ScaledFitness = 0;

// 	fscanf(pFile, "%d ", &m_Genes.Fitness);
// 
// 	fscanf(pFile, "%d ", &m_Genes.SplitOnSquadSize);
// 	fscanf(pFile, "%d ", &m_Genes.SquadCoherence);
// 	fscanf(pFile, "%d ", &m_Genes.MiningSlotsPerSquad);
// 	fscanf(pFile, "%d ", &m_Genes.DistanceToClaimableFood);
// 	fscanf(pFile, "%d ", &m_Genes.IsUnderAttackReset);
// 	fscanf(pFile, "%d ", &m_Genes.OverWriteUpgrades);
// 	fscanf(pFile, "%d ", &m_Genes.UpgradePriority);
// 	fscanf(pFile, "%d ", &m_Genes.SquadValueLowerLimit);

	for (int i = 0; i < m_Genes.PreferredUpgrade.size(); i++)
	{
		fscanf(pFile, "%d ", &m_Genes.PreferredUpgrade[i]);
	}

// 	if (m_Genes.Fitness < 50)
// 	{
// 		int maxUpgrades = static_cast<int>(m_Genes.PreferredUpgrade.size());
// 		int samples = g_Randomizer.SimRand(2, maxUpgrades);
// 
// 		for (int i = 0; i < samples;i++)
// 		{
// 			int index = g_Randomizer.SimRand(0, maxUpgrades);
// 			m_Genes.PreferredUpgrade[index] = g_Randomizer.SimRand(0, maxUpgrades);
// 		}
// 	}

	//float useOldGene = 1;
	//float useRndGene = std::abs(useOldGene - 1.0f);

// 	m_Genes.SplitOnSquadSize = (int)(m_Genes.SplitOnSquadSize*useOldGene)/* + (int)(g_Randomizer.UserRand(2, 5)*(useRndGene))+2*/;
// 	m_Genes.SquadCoherence = (int)(m_Genes.SplitOnSquadSize*useOldGene) /*+ (int)(g_Randomizer.UserRand(350, 1000)*(useRndGene))*/;
// 	m_Genes.MiningSlotsPerSquad = (int)(m_Genes.SplitOnSquadSize*useOldGene) /*+ (int)(g_Randomizer.UserRand(0, 15)*(useRndGene))*/;
// 	m_Genes.DistanceToClaimableFood = (int)(m_Genes.SplitOnSquadSize*useOldGene) /*+ (int)(g_Randomizer.UserRand(3, 50)*(useRndGene))*/;
// 	m_Genes.IsUnderAttackReset = (int)(m_Genes.SplitOnSquadSize*useOldGene) /*+ (int)(g_Randomizer.UserRand(3, 10)*(useRndGene))*/;
// 	m_Genes.SquadValueLowerLimit = /*(int)(m_Genes.SquadValueLowerLimit*useOldGene) + (int)(g_Randomizer.UserRand(100, 500)*(useRndGene))*/50;

	//only random if score was really bad
// 	if (ScaledFitness == 0)
// 	{
// 		m_Genes.OverWriteUpgrades		= !m_Genes.OverWriteUpgrades;
// 		m_Genes.UpgradePriority			= static_cast<UPGRADE_PRIORITY>(g_Randomizer.UserRand(0, UPGRADE_PRIORITY_MAX));
// 
// 		for (int i = 0; i < m_Genes.PreferredUpgrade.size(); i++)
// 		{
// 			m_Genes.PreferredUpgrade[i] = g_Randomizer.UserRand(0, UPGRADE_NAME::EMPTY);
// 		}
// 	}

//	printf("Team %d Genes: %d,%d,%d,%d,%d\n", m_Team, m_Genes.SplitOnSquadSize, m_Genes.SquadCoherence, m_Genes.MiningSlotsPerSquad, m_Genes.DistanceToClaimableFood, m_Genes.IsUnderAttackReset);

	fclose(pFile);
}

void AIPlayer::SetScaledFitness(int val)
{
	m_Genes.ScaledFitness = val; 
}

void AIPlayer::SetGeneFitness(int val)
{
	m_Genes.Fitness = val;
}

void AIPlayer::HardTrainingData()
{
	if (m_Perceptions[PERCEPTION_SIZE_ENEMY] > m_Perceptions[PERCEPTION_SIZE_SELF] && m_Perceptions[PERCEPTION_UPGRADE_VALUE] < m_Genes.SquadValueLowerLimit /*&& (m_Perceptions[PERCEPTION_CLOSEST_ENEMY] < 25)*/)
		m_WantedOutputs[DECISION_RETREAT] = 1;
	else
		m_WantedOutputs[DECISION_RETREAT] = -1;

	if ((m_Perceptions[PERCEPTION_SIZE_ENEMY] <= m_Perceptions[PERCEPTION_SIZE_SELF] && m_Perceptions[PERCEPTION_SIZE_ENEMY] > 0) || m_Perceptions[PERCEPTION_UNDER_ATTACK] > 0)
		m_WantedOutputs[DECISION_ATTACK] = 1;
	else
		m_WantedOutputs[DECISION_ATTACK] = -1;

	if (m_Perceptions[PERCEPTION_KNOWN_FARM] > 0 && m_Perceptions[PERCEPTION_UNDER_ATTACK] == 0 && m_Perceptions[PERCEPTION_OWNED_CONTROL_POINTS] > 0)
		m_WantedOutputs[DECISION_FARM] = 1;
	else
		m_WantedOutputs[DECISION_FARM] = -1;

	if ((m_Perceptions[PERCEPTION_SIZE_ENEMY] == 0 && m_Perceptions[PERCEPTION_KNOWN_FARM] == 0) || m_Perceptions[PERCEPTION_OWNED_CONTROL_POINTS] == 0)
		m_WantedOutputs[DECISION_SCOUT] = 1;
	else
		m_WantedOutputs[DECISION_SCOUT] = -1;

	//check for upgrades
	if (m_Perceptions[PERCEPTION_SIZE_SELF] > 1 && m_Perceptions[PERCEPTION_AVAILABLE_UPGRADES] > 0)
	{
		int limitUpg = 0;

		for (int i = m_Brain.CurrentUpgradeLoopIndex; i < m_Genes.PreferredUpgrade.size(); i++)
		{
			if (g_SSResearch.GetPlayerResearchDone(m_Team, static_cast<UPGRADE_NAME>(m_Genes.PreferredUpgrade[i])))
				limitUpg += g_SSUpgrades.GetUpgrade(m_Genes.PreferredUpgrade[i]).Value;
		}

		if (m_Perceptions[PERCEPTION_UPGRADE_VALUE] < limitUpg)
		{
			m_WantedOutputs[DECISION_UPGRADE] = 1;
			m_WantedOutputs[DECISION_ATTACK] = -1;
			m_WantedOutputs[DECISION_FARM] = -1;
			m_WantedOutputs[DECISION_SCOUT] = -1;
			m_WantedOutputs[DECISION_RETREAT] = -1;
			if (PRINT_AI_PLAYER)
				printf("team %d do upgrade\n", m_Team);
		}
		else if (m_Decisions[DECISION_UPGRADE])
		{
			m_WantedOutputs[DECISION_UPGRADE] = -1;
		}
	}
	else
	{
		m_WantedOutputs[DECISION_UPGRADE] = -1;
	}
}

void AIPlayer::SoftTraining(int id)
{
	if (m_SavedPerceptions[id].size() > 3)
	{
		for (int i = 0; i < m_WantedOutputs.size(); i++)
		{
			m_WantedOutputs[i] = 0.0f;
		}

		bool useTraining = false;

		float farmOnMap = m_SavedPerceptions[id][0][PERCEPTION_KNOWN_FARM] - m_Perceptions[PERCEPTION_KNOWN_FARM];
		float lostCP = m_SavedPerceptions[id][0][PERCEPTION_OWNED_CONTROL_POINTS] - m_Perceptions[PERCEPTION_OWNED_CONTROL_POINTS];
		float lostEnemies = m_SavedPerceptions[id][0][PERCEPTION_SIZE_ENEMY] - m_Perceptions[PERCEPTION_SIZE_ENEMY];
		float lostUnits = m_SavedPerceptions[id][0][PERCEPTION_SIZE_SELF] - m_Perceptions[PERCEPTION_SIZE_SELF];
		float valueUnits = m_SavedPerceptions[id][0][PERCEPTION_UPGRADE_VALUE] - m_Perceptions[PERCEPTION_UPGRADE_VALUE];

		//reward good decision
		if (lostUnits < 0 || valueUnits < 0 || (lostEnemies > 0 && m_SavedDecisions[id][0][DECISION_ATTACK]) || (lostEnemies > 0 && m_SavedDecisions[id][0][DECISION_RETREAT]) || lostCP < 0)
		{
			m_WantedOutputs[DECISION_ATTACK] = (m_SavedDecisions[id][0][DECISION_ATTACK] ? 1.0f : -1.0f);
			m_WantedOutputs[DECISION_FARM] = (m_SavedDecisions[id][0][DECISION_FARM] ? 1.0f : -1.0f);
			m_WantedOutputs[DECISION_RETREAT] = (m_SavedDecisions[id][0][DECISION_RETREAT] ? 1.0f : -1.0f);
			m_WantedOutputs[DECISION_SCOUT] = (m_SavedDecisions[id][0][DECISION_SCOUT] ? 1.0f : -1.0f);
/*			m_WantedOutputs[DECISION_SPLIT] = (m_SavedDecisions[id][0][DECISION_SPLIT] ? 1.0f : -1.0f);*/
			m_WantedOutputs[DECISION_UPGRADE] = (m_SavedDecisions[id][0][DECISION_UPGRADE] ? 1.0f : -1.0f);
			useTraining = true;
			if (PRINT_AI_PLAYER)
				printf("team %d | took good decision\n lostUnits: %.1f, valueUnits: %.1f, lostEnemies: %.1f, lostCP %.1f\n", m_Team,lostUnits ,valueUnits , lostEnemies , lostCP);

			m_Genes.Fitness ++;
		}
		else if (m_Genes.Fitness < 0)
		{
			int index = g_Randomizer.SimRand(0, 5);

			m_Decisions[DECISION_ATTACK] = (index == 0 ? 1 : 0);
			m_Decisions[DECISION_FARM] = (index == 1 ? 1 : 0);
			m_Decisions[DECISION_RETREAT] = (index == 2 ? 1 : 0);
			m_Decisions[DECISION_SCOUT] = (index == 3 ? 1 : 0);
			m_Decisions[DECISION_UPGRADE] = (index == 4 ? 1 : 0);
			m_Genes.Fitness++;
		}
		else
			m_Genes.Fitness--;

		ForbiddenDecisions(useTraining, id);

		if (useTraining)
		{
			//normalize
			for (int i = 0; i < m_WantedOutputs.size(); i++)
			{
				if (m_WantedOutputs[i] < -1.0f)
					m_WantedOutputs[i] = -1.0f;
				else if (m_WantedOutputs[i] > 1.0f)
					m_WantedOutputs[i] = 1.0f;
			}

			if (PRINT_AI_PLAYER)
				printf("team: %d learning | At: %.2f, F: %.2f, Sc: %.2f, Rt: %.2f, Up: %.2f\n", m_Team, m_WantedOutputs[DECISION_ATTACK], m_WantedOutputs[DECISION_FARM], m_WantedOutputs[DECISION_SCOUT], m_WantedOutputs[DECISION_RETREAT], m_WantedOutputs[DECISION_UPGRADE]);

			for (int i = 0; i < 5; i++)
				m_Net->Train(m_SavedPerceptions[id][0], m_WantedOutputs);

			GetNetOutput();

			if (PRINT_AI_PLAYER)
				printf("team %d | training\n",m_Team);
		}

		m_SavedPerceptions[id].erase(m_SavedPerceptions[id].begin());
		m_SavedDecisions[id].erase(m_SavedDecisions[id].begin());
	}
}

void AIPlayer::FindResearch()
{
	rVector<UpgradeData> candidates = g_SSUpgrades.GetUpgradeArray();

	if (g_SSResearch.GetPlayerResearchDone(m_Team, static_cast<UPGRADE_NAME>(m_Genes.PreferredUpgrade[m_Brain.CurrentUpgradeLoopIndex])))
	{
		m_Brain.CurrentUpgradeLoopIndex++;
	}

	if (m_Brain.CurrentUpgradeLoopIndex >= m_Genes.PreferredUpgrade.size())
	{
		m_AllResearchFinished = true;
		if (PRINT_AI_PLAYER)
		printf("team %d Finished all research\n",m_Team);
		return;
	}

	m_CurrentUpgrade = static_cast<UPGRADE_NAME>(m_Genes.PreferredUpgrade[m_Brain.CurrentUpgradeLoopIndex]);

// 	if (PRINT_AI_PLAYER)
// 		printf("team %d: Research %d, Progress %d\n", m_Team, m_CurrentUpgrade,g_SSResearch.GetPlayerResearchProgress(m_Team,m_CurrentUpgrade));

	g_SSResearch.SwitchPlayerActiveResearch(m_Team, m_CurrentUpgrade);
}

int AIPlayer::GetSize()
{
	int str = 0;

	for (int i = 0; i < m_Squads.size(); i++)
	{
		str += m_Squads[i]->GetSize();
	}

	return str;
}

void AIPlayer::CheckBeingAttacked()
{
	for (int i = 0; i < m_SimMailbox.size(); ++i)
	{
		const Message* message = m_SimMailbox[i];
		switch (message->Type)
		{

		case MessageTypes::AI_MSG:
		{
			const AIMessage* openMsg = static_cast<const AIMessage*>(message);
			if (openMsg->TeamID == m_Team)
			{
				if (static_cast<AIStateMessages>(openMsg->AIMsg) == AIStateMessages::UNDER_ATTACK)
				{
					if (openMsg->SquadID >= m_Squads.size())
						continue;

					Squad* squadUnderAttack = m_Squads[openMsg->SquadID];
					Agent* agentUnderAttack = GetAgentPointer(openMsg->EntityID);

					if (squadUnderAttack->GetSize() == 0)
						continue;

					m_Brain.SquadIDUnderAttack = openMsg->SquadID;

					if (agentUnderAttack != nullptr)
					{
						m_Perceptions[PERCEPTION_UNDER_ATTACK] = 1;
						m_Brain.TimeSinceAttacked = 0;
						m_Brain.AgentUnderAttack = agentUnderAttack->GetEntityID();
					}
				}
			}
		}
		break;

		default:
			Logger::Log("Received unrecognized message", "SSAI", LogSeverity::WARNING_MSG);
			break;
		}
	}
}

void AIPlayer::ForbiddenDecisions(bool& useTraining, int id)
{
	if (PRINT_AI_PLAYER)
		printf("team %d | available upgs %.2f\n",m_Team, m_SavedPerceptions[id][0][PERCEPTION_AVAILABLE_UPGRADES]);

	//check for upgrades
	if (m_SavedPerceptions[id][0][PERCEPTION_SIZE_SELF] > 3 && m_SavedPerceptions[id][0][PERCEPTION_AVAILABLE_UPGRADES] > 0)
	{
		int limitUpg = 0;

		for (int i = 0; i < m_Genes.PreferredUpgrade.size(); i++)
		{
			if (g_SSResearch.GetPlayerResearchDone(m_Team, static_cast<UPGRADE_NAME>(m_Genes.PreferredUpgrade[i])))
				limitUpg += g_SSUpgrades.GetUpgrade(m_Genes.PreferredUpgrade[i]).Value;
		}

		if (m_SavedPerceptions[id][0][PERCEPTION_UPGRADE_VALUE] < limitUpg)
		{
			m_Decisions[DECISION_UPGRADE] = 1;
			m_Decisions[DECISION_ATTACK] = 0;
			m_Decisions[DECISION_FARM] = 0;
			m_Decisions[DECISION_SCOUT] = 0;
			m_Decisions[DECISION_RETREAT] = 0;

			if (PRINT_AI_PLAYER)
				printf("team %d do upgrade\n", m_Team);

			m_WantedOutputs[DECISION_ATTACK] = -1;
			m_WantedOutputs[DECISION_FARM] = -1;
			m_WantedOutputs[DECISION_RETREAT] = -1;
			m_WantedOutputs[DECISION_SCOUT] = -1;
			m_WantedOutputs[DECISION_UPGRADE] = 1;

			useTraining = true;
		}
		else if (m_Decisions[DECISION_UPGRADE])
		{
			m_WantedOutputs[DECISION_UPGRADE] = -1;
			m_Decisions[DECISION_UPGRADE] = 0;
		}
	}
	else if (m_Decisions[DECISION_UPGRADE])
	{
		m_WantedOutputs[DECISION_UPGRADE] = -1;
		m_Decisions[DECISION_UPGRADE] = 0;
	}

	if (m_SavedPerceptions[id][0][PERCEPTION_UNDER_ATTACK] > 0 && !m_SavedDecisions[id][0][DECISION_ATTACK])
	{
		m_Decisions[DECISION_UPGRADE] = 0;
		m_Decisions[DECISION_ATTACK] = 1;
		m_Decisions[DECISION_FARM] = 0;
		m_Decisions[DECISION_SCOUT] = 0;
		m_Decisions[DECISION_RETREAT] = 0;

		if (PRINT_AI_PLAYER)
			printf("team %d return fire\n", m_Team);

		m_WantedOutputs[DECISION_ATTACK] = 1;
		m_WantedOutputs[DECISION_FARM] = -1;
		m_WantedOutputs[DECISION_RETREAT] = -1;
		m_WantedOutputs[DECISION_SCOUT] = -1;
		m_WantedOutputs[DECISION_UPGRADE] = -1;
	}

	//hard fixed nono decisions
	if (m_SavedPerceptions[id][0][PERCEPTION_KNOWN_FARM] == 0 && m_SavedDecisions[id][0][DECISION_FARM])
	{
		m_Decisions[DECISION_FARM] = 0;
		m_WantedOutputs[DECISION_FARM] = -1;
	}
	if (m_SavedPerceptions[id][0][PERCEPTION_SIZE_ENEMY] == 0)
	{
		if (m_SavedDecisions[id][0][DECISION_ATTACK])
		{
			m_Decisions[DECISION_ATTACK] = 0;
			m_WantedOutputs[DECISION_ATTACK] = -1;
		}
		if (m_SavedDecisions[id][0][DECISION_RETREAT])
		{
			m_Decisions[DECISION_RETREAT] = 0;
			m_WantedOutputs[DECISION_RETREAT] = -1;
		}
	}

	if (m_SavedPerceptions[id][0][PERCEPTION_OWNED_CONTROL_POINTS] == 0)
	{
		m_WantedOutputs[DECISION_ATTACK] = -1;
		m_WantedOutputs[DECISION_FARM] = -1;
		m_WantedOutputs[DECISION_RETREAT] = -1;
		m_WantedOutputs[DECISION_SCOUT] = 1;
		m_WantedOutputs[DECISION_UPGRADE] = -1;

		useTraining = true;
	}

	if (m_Perceptions[PERCEPTION_SIZE_SELF] == SQUAD_MAXIMUM_UNIT_COUNT)
	{
		m_WantedOutputs[DECISION_FARM]		= -1;
		m_Decisions[DECISION_FARM] = 0;
	}
}
