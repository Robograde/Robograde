/**************************************************
Copyright 2015 Viktor Kelkkanen
***************************************************/

#include "SSAI.h"
#include "../../CompileFlags.h"
#include <input/Input.h>
#include <utility/Logger.h>
#include <utility/Randomizer.h>
#include <messaging/GameMessages.h>
#include "../../ai/Terrain.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/AgentComponent.h"
#include "../../component/PlacementComponent.h"
#include "../../component/ModelComponent.h"
#include "../../datadriven/EntityManager.h"
#include "../../EntityFactory.h"
#include "../../datadriven/ComponentHelper.h"
#include "../../component/UpgradeComponent.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../utility/GameModeSelector.h"
#include "../../utility/Alliances.h"
#include "../../ai/PathFinder.h"
#include "../utility/SSMail.h"
#include "../gamelogic/SSKill.h"
#include "../gfx/SSDecaling.h"


#if AI_DEBUG == 1
#include <profiler/Profiler.h>
#include <profiler/AutoProfiler.h>
#endif

#include "../../subsystem/gamelogic/SSUpgrades.h"
#include "../gui/SSGUIInfo.h"


SSAI& SSAI::GetInstance( )
{
	static SSAI instance;
	return instance;
}

SSAI::SSAI( )
	: Subsystem( "AI" ), Subscriber("AI") { }

void SSAI::Startup( )
{
	m_MaxSquadID = 0;

	m_SimInterests = MessageTypes::ORDER_UNITS | MessageTypes::ORDER_INVOKE | MessageTypes::UPGRADE | MessageTypes::WRITE_FILE;
	g_SSMail.RegisterSubscriber( this );

	DEV(
		g_Script.Register( "GE_IncreaseSquadSize", std::bind( &SSAI::ScriptIncreaseSquadSize, this, std::placeholders::_1 ) );
		g_Script.Register( "GE_DecreaseSquadSize", std::bind( &SSAI::ScriptDecreaseSquadSize, this, std::placeholders::_1 ) );
		g_Script.Register( "GE_ChangeSquadSize", std::bind( &SSAI::ScriptChangeSquadSize, this, std::placeholders::_1 ) );
		g_Script.Register( "GE_KillPlayer", std::bind( &SSAI::ScriptKillPlayer, this, std::placeholders::_1 ) );
	);
	Pathfinder::GetInstance( )->Init( );

	bool setAI = g_GameModeSelector.GetCurrentGameMode( ).Type == GameModeType::AIOnly ? true : false;

	rVector<glm::vec3> spawnPoints = g_GameData.GetSpawnPoints( );

	int playerToSpawn = 0;

	m_Teams.resize( spawnPoints.size( ) );

#if AI_BALANCE_TEST == 0
	if ( 		g_GameModeSelector.GetCurrentGameMode( ).Type != GameModeType::Multiplayer
			&& 	g_GameModeSelector.GetCurrentGameMode( ).Type != GameModeType::SingleplayerAI // TODOJM: Remove this temp hax
			&&  g_GameModeSelector.GetCurrentGameMode( ).Type != GameModeType::Replay )
	{
		for ( unsigned int i = 0; i < spawnPoints.size( ); i++ )
		{
			bool isAI = setAI;
			if ( g_GameModeSelector.GetCurrentGameMode( ).Type == GameModeType::SingleplayerAI ) // Make sure the players team doesn't get set to an AIPlayer
				isAI = playerToSpawn != g_PlayerData.GetPlayerID( );
			SpawnPlayer( spawnPoints[g_GameData.GetPlayerSpawnPoint( playerToSpawn )], playerToSpawn, isAI );
			playerToSpawn++;
			if ( playerToSpawn >= MAX_PLAYERS )
				playerToSpawn = 0;
		}
	}
	else
	{
		int maxSpawnPoints = static_cast<int>(spawnPoints.size( ));
		for ( auto& player : g_GameData.GetPlayers( ) )
		{
			int spawnPointIndex = g_GameData.GetPlayerSpawnPoint( player.PlayerID );
			if ( spawnPointIndex < maxSpawnPoints && spawnPointIndex != -1 )
			{
				SpawnPlayer( spawnPoints[spawnPointIndex], player.PlayerID, player.IsAI );
			}
		}
	}
#else
		BalanceTest(m_CurrentTestUpg1, m_CurrentTestUpg2);
#endif

}

void SSAI::UpdateSimLayer(const float timeStep)
{
	for (int i = 0; i < m_SimMailbox.size(); ++i)
	{
		const Message* message = m_SimMailbox[i];
		switch (message->Type)
		{

		case MessageTypes::ORDER_UNITS:
		{
			const OrderUnitsMessage* order = static_cast<const OrderUnitsMessage*>(message);
			OrderUnits(order->Squads, order->TargetLocation, order->TargetEntity, static_cast<Squad::MissionType>(order->MissionType), order->TeamID, order->OverridingCommand);
		} break;

		case MessageTypes::ORDER_INVOKE:
		{
		   const OrderInvokeMessage* order = static_cast<const OrderInvokeMessage*>(message);
		   InvokeAICommand( order->Squads, static_cast<AICommands>(order->EnumCommand), order->TeamID, order->GenericValue );
		} break;

		case MessageTypes::UPGRADE:
		{
			const UpgradeMessage* upgradeMessage = static_cast<const UpgradeMessage*>(message);
			InvokeAICommand( upgradeMessage->Squads, AICommands::INVOKE_UPGRADE, upgradeMessage->PlayerID, upgradeMessage->UpgradeType );
		} break;

// 		case MessageTypes::WRITE_FILE:
// 		{
// 			const WriteFileMessage* msg = static_cast<const WriteFileMessage*>(message);
// 			WriteToFile(msg->FileTextStr,msg->FilePathStr);
// 		} break;

		default:
			Logger::Log("Received unrecognized message", "SSAI", LogSeverity::WARNING_MSG );
		}
	}

	UpdateAIControl( timeStep );

	for ( int i = 0; i < m_Teams.size( ); i++ )
	{
		for ( int j = 0; j < static_cast<int>(m_Teams[i].Squads.size( )); j++ )
		{
			rVector<glm::vec3> newUnits = m_Teams[i].Squads[j]->GetUnitsToSpawn();
			if ( newUnits.size() > 0 )
			{
				Tile* t = Terrain::GetInstance()->GetSurroundingFreeTile(newUnits.back().x, newUnits.back().z, ENTITY_INVALID);
				Entity newEntityID = EntityFactory::CreateSquadUnit(static_cast<float>(t->X), static_cast<float>(t->Y), m_Teams[i].Squads[j]->GetID(), m_Teams[i].TeamID);

				ParentComponent* parent = GetDenseComponent<ParentComponent>(m_Teams[i].Squads[j]->GetLeader()->GetEntityID());

				for (int slot = 0; slot < 4; slot++)
				{
					if (parent->Children[slot] != ENTITY_INVALID)
					{
						UpgradeComponent* upgrade = GetDenseComponent<UpgradeComponent>( parent->Children[slot] );
						EntityFactory::CreateSquadUnitModule( newEntityID, m_Teams[i].TeamID, upgrade->UpgradeDataID );
					}
				}

				m_Teams[i].Squads[j]->PopHostUnit();
			}
		}
	}

#if AI_BALANCE_TEST == 1

	if (m_Teams[0].Squads.size() == 0 || m_Teams[1].Squads.size() == 0)
	{
		KillPlayer(0);
		KillPlayer(1);

		m_CurrentTestUpg1++;
		if (m_CurrentTestUpg1 > 6)
		{
			m_CurrentTestUpg1 = 0;
			m_CurrentTestUpg2++;
		}

		if (m_CurrentTestUpg2 > 6)
		{
			printf("balance test finished\n");
			m_CurrentTestUpg2 = 0;
		}

		BalanceTest(m_CurrentTestUpg1, m_CurrentTestUpg2);
	}

	m_Teams[0].Squads[0]->UseAutoAttack(true, false);
	m_Teams[1].Squads[0]->UseAutoAttack(true, false);
#endif
}



rVector<int> SSAI::GetSquadsFromSelection( const rVector<Entity>& _units, int* outTeamID, rVector<Entity>& outEntitiesInSquads ) const
{
	rVector<int> squadIDs;

	for (int i = 0; i < _units.size(); i++)
	{
		Agent* a = GetAgentPointer(_units[i]);
		if (a != nullptr)
		{
			int squad = a->GetSquadID();

			if (squad > -1)
			{
				bool squadIDAlreadyAdded = false;
				for (int j = 0; j < squadIDs.size(); j++)
				{
					if (squadIDs[j] == squad)
					{
						squadIDAlreadyAdded = true;
						break;
					}
				}
				if (!squadIDAlreadyAdded)
				{
					squadIDs.push_back(squad);
					*outTeamID = a->GetTeam();

					//GetSquadWithID may return nullptr, check this, noobs
					if (GetSquadWithID(*outTeamID, squad) != nullptr)
					{
						for (auto& agent : GetSquadWithID(*outTeamID, squad)->GetAgents())
						{
							outEntitiesInSquads.push_back(agent->GetEntityID());
						}
					}
				}
			}
		}
	}
	return squadIDs;
}

void SSAI::OrderUnits( const rVector<int>& _squads, const glm::vec3& _targetPosition, Entity _targetEntity, Squad::MissionType _mission, int _team, bool _override )
{
	glm::vec3	position	= _targetEntity != ENTITY_INVALID ? GetDenseComponent<PlacementComponent>( _targetEntity )->Position : _targetPosition;
	Tile*		tile		= Terrain::GetInstance()->GetTile(position.x, position.z);

	float markerMaxScale = 2.0f;
	float markerMinScale = 1.5f;
	float markerTTL = 2.0f;

	if ( _mission == Squad::MissionType::MISSION_TYPE_ATTACK )
	{
		if (_team == g_PlayerData.GetPlayerID())
		{
			g_SSDecaling.AddTimedDecal(position, "markerAttack.png", markerTTL, m_MarkerAttackColour, markerMaxScale, markerMinScale, false);
		}

		for ( auto& squadID : _squads )
		{
			Squad* squadPointer = GetSquadWithID(_team, squadID);
			if ( squadPointer == nullptr )
				continue;

			squadPointer->AddMission(tile, Squad::MissionType::MISSION_TYPE_FOCUS_FIRE, _override);
		}
	}
	else if (_mission == Squad::MissionType::MISSION_TYPE_MOVE)
	{
		if (_team == g_PlayerData.GetPlayerID())
		{
			g_SSDecaling.AddTimedDecal( position, "markerMove.png", markerTTL, m_MarkerMoveColour, markerMaxScale, markerMinScale, false );
		}

		for (auto& squadID : _squads)
		{
			Squad* squadPointer = GetSquadWithID(_team, squadID);
			if ( squadPointer == nullptr )
				continue;

			squadPointer->AddMission(tile, Squad::MissionType::MISSION_TYPE_MOVE, _override);
		}
	}
	else if (_mission == Squad::MissionType::MISSION_TYPE_ATTACK_MOVE)
	{
		if (_team == g_PlayerData.GetPlayerID())
		{
			g_SSDecaling.AddTimedDecal( position, "markerAttackMove.png", markerTTL, m_MarkerAttackMoveColour, markerMaxScale, markerMinScale, false );
		}

		for (auto& squadID : _squads)
		{
			Squad* squadPointer = GetSquadWithID(_team, squadID);
			if (squadPointer == nullptr)
				continue;

			squadPointer->AddMission(tile, Squad::MissionType::MISSION_TYPE_ATTACK_MOVE, _override);
		}
	}
	else if ( _mission == Squad::MissionType::MISSION_TYPE_MINE )
	{
		if (_team == g_PlayerData.GetPlayerID())
		{
			float miningMarkerMaxScale = _targetEntity == ENTITY_INVALID ? 10.0f : markerMaxScale;
			float miningMarkerMinScale = _targetEntity == ENTITY_INVALID ? 7.0f : markerMinScale;
			g_SSDecaling.AddTimedDecal( position, "markerMining.png", markerTTL, m_MarkerMiningColour, miningMarkerMaxScale, miningMarkerMinScale, false );
		}

		for ( auto& squadID : _squads )
		{
			Squad* squadPointer = GetSquadWithID( _team, squadID );
			if ( squadPointer == nullptr )
				continue;

			if ( !squadPointer->CheckCanMine( ) )
			{
				if ( squadPointer->GetTeam( ) == g_PlayerData.GetPlayerID( ) )
					g_SSGUIInfo.DisplayMessage( "Squad too big and units too healthy. Split squad to keep multiplying." );
			}
			else
				squadPointer->AddMission( tile, Squad::MissionType::MISSION_TYPE_MINE, _override );
		}
	}
	else
	{
		for ( auto& squadID : _squads )
		{
			Squad* squadPointer = GetSquadWithID(_team, squadID);
			if ( squadPointer == nullptr )
				continue;

			squadPointer->AddMission(tile, _mission, _override);
		}
	}
}

void SSAI::AddSquad( Squad* squad )
{
	if (squad->GetID() > m_MaxSquadID)
		m_MaxSquadID = squad->GetID();

	m_Teams[squad->GetTeam()].Squads.push_back(squad);

	if (m_AIPlayers.size() > 0)
	{
		for (int i = 0; i < m_AIPlayers.size();i++)
		{
			if (m_AIPlayers[i]->GetTeamID() == squad->GetTeam())
				m_AIPlayers[i]->AddSquad(squad);
		}
	}
}

void SSAI::KillAgent( Agent* agent )
{
	int squadID = agent->GetSquadID();
	int teamID = agent->GetTeam();

	if ( squadID >= 0 )
	{
		Squad* squad = GetSquadWithID( teamID, squadID );
		squad->DeleteAgent( agent );

		if ( squad->GetAgents().empty() )
		{
			KillSquad( squad );
		}
	}
}

void SSAI::IncreaseSquadSize( int sizeIncrease, int squadID, int teamID )
{
	Squad* squad = GetSquadWithID( teamID, squadID );
	if ( squad )
	{
		Entity squadMemberID = squad->GetAgents()[0]->GetEntityID();
		for ( int i = 0; i < sizeIncrease; ++i )
		{
			EntityFactory::CreateSquadUnitCopy( squadMemberID );
		}
	}
}

void SSAI::DecreaseSquadSize( int sizeDecrease, int squadID, int teamID )
{
	Squad* squad = GetSquadWithID( teamID, squadID );

	if ( squad )
	{
		int targetSquadSize = glm::max( 0, squad->GetSize() - sizeDecrease );
		while ( squad->GetSize() > targetSquadSize )
		{
			 g_SSKill.KillEntity( squad->GetAgents().back()->GetEntityID() );
		}
	}
}

void SSAI::ChangeSquadSize( int newSize, int squadID, int teamID )
{
	Squad* squad = GetSquadWithID( teamID, squadID );

	if ( squad )
	{
		int unitsToAdd = newSize - squad->GetSize();
		if ( unitsToAdd > 0 )
		{
			IncreaseSquadSize( unitsToAdd, squadID, teamID );
		}
		else if ( unitsToAdd < 0 )
		{
			DecreaseSquadSize( -unitsToAdd, squadID, teamID );
		}
	}
}

void SSAI::KillPlayer( int teamID )
{
	if ( teamID < 0 || teamID >= m_Teams.size() )
	{
		return;	// TODOOE: Print warning here.
	}

	auto& squads = m_Teams[teamID].Squads;

	while ( !squads.empty() )
	{
		this->ChangeSquadSize( 0, squads.back()->GetID(), teamID );
	}
}

void SSAI::AddAgent( Agent* agent )
{
	int squad = agent->GetSquadID();
	int team = agent->GetTeam();
	if (squad >= 0)
	{
		GetSquadWithID(team, squad)->AddAgent(agent);
    }
// 	else
// 		agent->SetID(-1);
}
void SSAI::UpdateAIControl( float dt )
{
#if AI_DEBUG == 1
	PROFILE(AutoProfiler AIPlayers("AI_Bots"));
#endif
	for (int i = 0; i < m_AIPlayers.size(); i++)
	{
		m_AIPlayers[i]->Update(dt);
	}
#if AI_DEBUG == 1
	PROFILE(AIPlayers.Stop());
#endif
#if AI_DEBUG == 1
	PROFILE(AutoProfiler AISquads("AI_Squads"));
#endif
	for (int i = 0; i < m_Teams.size(); i++)
	{
		for (int j = 0; j < m_Teams[i].Squads.size(); j++)
		{
			m_Teams[i].Squads[j]->Update(dt);
		}
	}
#if AI_DEBUG == 1
	PROFILE(AISquads.Stop());
#endif
	EntityMask agentFlag = DenseComponentCollection<AgentComponent>::GetInstance().GetComponentTypeFlag();
	EntityMask placementFlag = DenseComponentCollection<PlacementComponent>::GetInstance().GetComponentTypeFlag();
	int entityID = 0;
#if AI_DEBUG == 1
	PROFILE(AutoProfiler AIAgents("AI_Agents"));
#endif
	for ( auto& entityMask : g_EntityManager.GetEntityMasks() )
	{
		if ( ( entityMask & agentFlag ) )
		{
			AgentComponent* agent = GetDenseComponent<AgentComponent>( entityID );
			agent->Agent->Update(dt);
		}
		entityID++;
	}
#if AI_DEBUG == 1
	PROFILE(AIAgents.Stop());
#endif
}

void SSAI::AddToSensor( Entity _entity, int _x, int _y, unsigned int _size )
{
	Tile* t = Terrain::GetInstance()->GetTile(_x,_y);
	Terrain::GetInstance()->UpdateSensor(_entity, t, t, _size, false);
}

void SSAI::InvokeAICommand( const rVector<int>& _squads, const AICommands _command, const int _team, int _value )
{
	for ( auto& squadID : _squads )
	{
		Squad* squadPointer = GetSquadWithID( _team, squadID );
		if ( squadPointer == nullptr )
			continue;

		if ( _command == INVOKE_AUTO_ATTACK )
		{
			if ( squadPointer->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
			{
				if (g_PlayerData.GetPlayerID() == _team)
					g_SSGUIInfo.DisplayMessage( "Unable to toggle auto attack on an upgrading squad." );
				continue;
			}

			bool toggle = false;
			if ( _value == 2 )
				toggle = true;

			squadPointer->UseAutoAttack( ( _value == 1 ), toggle );
		}
		else if ( _command == INVOKE_UPGRADE )
		{
			if ( squadPointer->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
			{
				if (g_PlayerData.GetPlayerID() == _team)
					g_SSGUIInfo.DisplayMessage( "Unable to upgrade multiple upgrades on the same squad at the same time." );
				continue;
			}
			squadPointer->SetUpgradeTo(static_cast<UPGRADE_NAME>(_value));
			squadPointer->AddMission(nullptr, Squad::MissionType::MISSION_TYPE_UPGRADE, true);
		}
		else if ( _command == INVOKE_STOP )
		{
			if ( squadPointer->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
			{
				if (g_PlayerData.GetPlayerID() == _team)
					g_SSGUIInfo.DisplayMessage( "Unable to stop an ongoing upgrade." );
				squadPointer->PopQueuedMissions();
				continue;
			}
			squadPointer->PopAllMissions();
		}
		else if ( _command == FORMATION_CYCLE )
		{
			if ( squadPointer->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
			{
				if (g_PlayerData.GetPlayerID() == _team)
					g_SSGUIInfo.DisplayMessage( "Unable to change formation on an upgrading squad." );
				continue;
			}
			WriteFileMessage message = WriteFileMessage("omfg", "../../../asset/ai/huehue.txt");
			g_SSMail.PushToNextFrame(message);
			squadPointer->CycleFormation();
		}
		else if ( _command == FORMATION_SET )
		{
			if ( squadPointer->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
			{
				if (g_PlayerData.GetPlayerID() == _team)
					g_SSGUIInfo.DisplayMessage( "Unable to change formation on an upgrading squad." );
				continue;
			}
			squadPointer->SetFormation( Squad::FORMATION( _value ) );
		}
		else if ( _command == FORMATION_ADD_SPACE )
		{
			if ( squadPointer->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
			{
				if (g_PlayerData.GetPlayerID() == _team)
					g_SSGUIInfo.DisplayMessage( "Unable to change formation spacing on an upgrading squad." );
				continue;
			}
			squadPointer->AddSpacingStep( _value );
		}
		else if ( _command == FORMATION_SET_SPACE )
		{
			if ( squadPointer->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
			{
				if (g_PlayerData.GetPlayerID() == _team)
					g_SSGUIInfo.DisplayMessage( "Unable to change formation spacing on an upgrading squad." );
				continue;
			}
			squadPointer->SetSpacingStep( _value );
		}
		else if ( _command == FORMATION_RESET )
		{
			if ( squadPointer->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
			{
				if (g_PlayerData.GetPlayerID() == _team)
					g_SSGUIInfo.DisplayMessage( "Unable to change formation on an upgrading squad." );
				continue;
			}
			squadPointer->FindBuddies();
			squadPointer->SetFormation(squadPointer->GetFormation());
		}
		else if ( _command == SQUAD_MERGE )
		{
			if ( _squads.size() < 2 )
				continue;

			//check for mixed slots, no merge if so
			rVector<std::array<UPGRADE_NAME,4>> slots;
			rVector<rVector<int>> buckets;
			slots.resize(_squads.size());
			unsigned int bucketIndex = 0;
			bool mixedSlots = false;
			bool upgrading = false;
			bool tooBig = false;

			for (int i = 0; i < _squads.size(); i++)
			{
				slots[i][0] = EMPTY; slots[i][1] = EMPTY; slots[i][2] = EMPTY; slots[i][3] = EMPTY;

				Squad* tempSquad = GetSquadWithID(_team, _squads[i]);

				if ( tempSquad == nullptr )
					break;

				if ( tempSquad->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
				{
					upgrading = true;
					continue;
				}

				ParentComponent* parent = GetDenseComponent<ParentComponent>(tempSquad->GetLeader()->GetEntityID());

				for (int slot = 0; slot < 4; slot++)
				{
					if (parent->Children[slot] != ENTITY_INVALID)
					{
						slots[i][slot] = static_cast<UPGRADE_NAME>(GetDenseComponent<UpgradeComponent>(parent->Children[slot])->UpgradeDataID);
					}
				}
			}

			bool mergeHappened = false;
			rVector<int> removed;
			for (int g = 0; g < _squads.size();g++)
			{
				Squad* squadToMerge = GetSquadWithID(_team, _squads[g]);

				if (squadToMerge == nullptr)
					continue;

				for (int i = static_cast<int>(slots.size()) - 1; i > 0; i--)
				{
					if (i == g)
						continue;

					bool wasRemoved = false;
					for (int r = 0; r < removed.size(); r++)
					{
						if (removed[r] == i)
						{
							wasRemoved = true;
							break;
						}
					}

					if (wasRemoved)
						continue;

					bool mixed = false;
					for (int k = 0; k < 4; k++)
					{
						if (slots[g][k] != slots[i][k])
						{
							mixedSlots = true;
							mixed = true;
							break;
						}
					}

					if (mixed)
						continue;

					Squad* squadToRemove = GetSquadWithID(_team, _squads[i]);
					if (squadToRemove == nullptr)
						break;
					rVector<Agent*> agents = squadToRemove->GetAgents();

					if (agents.size() + squadToMerge->GetSize() > SQUAD_MAXIMUM_UNIT_COUNT)
					{
						tooBig = true;
						continue;
					}

					mergeHappened = true;

					assert(agents.size() > 0);

					for (int j = 0; j < agents.size(); j++)
					{
						squadToMerge->AddAgent(agents[j]);
					}

					for (int j = 0; j < m_Teams[_team].Squads.size(); j++)
					{
						if (m_Teams[_team].Squads[j]->GetID() == squadToRemove->GetID())
						{
							removed.push_back(g);
							KillSquad(m_Teams[_team].Squads[j]);
							break;
						}
					}
				}
				squadToMerge->RepathFormation();

			}

			if (mergeHappened)
			{
				if (_team == g_PlayerData.GetPlayerID())
				{
					g_PlayerData.GetEditableSelectedSquads().clear();
					g_PlayerData.GetEditableSelectedSquads().push_back(_squads[0]);
				}
			}
			else
			{
				if (mixedSlots)
				{
					if (g_PlayerData.GetPlayerID() == _team)
						g_SSGUIInfo.DisplayMessage("Unable to merge squads with different upgrades.");
					break;
				}

				if (upgrading)
				{
					if (g_PlayerData.GetPlayerID() == _team)
						g_SSGUIInfo.DisplayMessage("Unable to merge squads since one of them is currently upgrading.");
					break;
				}

				if (tooBig)
				{
					if (squadPointer->GetTeam() == g_PlayerData.GetPlayerID())
						g_SSGUIInfo.DisplayMessage("Resulting squad would become too big. No squads were merged.");
					continue;
				}
			}

			//only run once
			break;
		}
		else if ( _command == SQUAD_SPLIT )
		{
			if ( squadPointer->GetCurrentMission() == Squad::MISSION_TYPE_UPGRADE )
			{
				if (g_PlayerData.GetPlayerID() == _team)
					g_SSGUIInfo.DisplayMessage( "Unable to split an upgrading squad." );
				continue;
			}

			Squad* squadToSplit = squadPointer;

			if ( squadToSplit == nullptr )
				continue;

			if ( m_Teams.at( _team ).Squads.size( ) >= SQUAD_MAXIMUM_SQUAD_COUNT )
			{
				if ( squadToSplit->GetTeam( ) == g_PlayerData.GetPlayerID( ) )
					g_SSGUIInfo.DisplayMessage( "You have too many squads. Merge a squad to be able to create a new one." );
				break;
			}

			if ( squadToSplit->GetSize( ) < 2 * SQUAD_MINUMUM_UNIT_COUNT )
			{
				if ( squadToSplit->GetTeam() == g_PlayerData.GetPlayerID() )
					g_SSGUIInfo.DisplayMessage( "Squad is too small to be split." );
				continue;
			}

			rVector<Agent*> agents = squadToSplit->SplitSquad();

			squadToSplit->RepathFormation();

			int newSquadID = m_MaxSquadID + 1;

			Squad* newSquad = tNew( Squad, _team, newSquadID );

			AddSquad( newSquad );

			newSquad->SetFormation( squadToSplit->GetFormation() );
			newSquad->SetSpacingStep( squadToSplit->GetSpacingStep() );

			glm::vec2 position( 0.0f );
			for ( int i = 0; i < agents.size(); i++ )
			{
				newSquad->AddAgent( agents[i] );
				position += agents[i]->GetPos();
			}
			position /= agents.size();

			newSquad->AddMission( Terrain::GetInstance()->GetTile( position.x, position.y ), Squad::MissionType::MISSION_TYPE_MOVE, true );

			if ( _team == g_PlayerData.GetPlayerID() )
			{
				for ( int i = 0; i < g_PlayerData.GetEditableSelectedSquads().size(); ++i )
				{
					if ( squadToSplit->GetID() == g_PlayerData.GetEditableSelectedSquads()[i] )
					{
						g_PlayerData.GetEditableSelectedSquads().erase( g_PlayerData.GetEditableSelectedSquads().begin() + i );
						break;
					}
				}
				g_PlayerData.GetEditableSelectedSquads().push_back( newSquad->GetID() );
			}
		}
	}
}

Squad* SSAI::GetSquadWithID(int _team, int _id) const
{
	if ( _team >= 0 && _team < m_Teams.size() )
	{
		for (int i = 0; i < m_Teams[_team].Squads.size(); i++)
		{
			if (m_Teams[_team].Squads[i]->GetID() == _id)
				return m_Teams[_team].Squads[i];
		}
	}

	//Logger::Log("GetSquadWithID returned nullptr (no such ID)", "SSAI", LogSeverity::WARNING_MSG);
	return nullptr;
}

void SSAI::KillSquad(const Squad* squad)
{
	int teamID	= squad->GetTeam();
	int squadID = squad->GetID();

	if ( teamID == g_PlayerData.GetPlayerID() )
	{
		// Remove squad from selection
		for ( int i = 0; i < g_PlayerData.GetEditableSelectedSquads().size(); ++i )
		{
			if ( g_PlayerData.GetSelectedSquads()[i] == squadID )
			{
				g_PlayerData.GetEditableSelectedSquads().erase( g_PlayerData.GetSelectedSquads().begin() + i );
				break;
			}
		}

		// Remove squad from all control groups
		for ( int i = 0; i < MAX_CONTROL_GROUPS; ++i )
		{
			rVector<int> controlGroup = g_PlayerData.GetControlGroup( i );
			for ( int j = 0; j < controlGroup.size(); ++j )
			{
				if ( controlGroup[j] == squadID )
				{
					g_PlayerData.RemoveSquadFromControlGroup( i, j );
					break;
				}
			}
		}
	}

	// Remove squad from AI player
	for (int j = 0; j < m_AIPlayers.size(); j++)
	{
		if (m_AIPlayers[j]->GetTeamID() == teamID)
			m_AIPlayers[j]->DeleteSquad(squadID);
	}

	// Remove squad from AI and lastly delete it
	for ( int i = 0; i < m_Teams[teamID].Squads.size(); ++i )
	{
		if ( m_Teams[teamID].Squads[i]->GetID() == squadID )
		{
			m_Teams[teamID].Squads.erase( m_Teams[teamID].Squads.begin() + i );
			tDelete( squad );
			break;
		}
	}
}

void SSAI::SpawnPlayer(glm::vec3 spawnPoint,int player,bool isAI)
{
	if (isAI)
	{
		bool neuralNet = true;
// 		if (player == 0)
// 			neuralNet = false;

		m_AIPlayers.push_back(pNew(AIPlayer, player, neuralNet));
	}

	m_Teams[player].TeamID = player;
	
	AddSquad(tNew(Squad, player, 0));
	//AddSquad(rNew(Squad, player, 1));		// TODOOE: For playtesting, only want to start with one squad.

	int row = static_cast<int>(floor(sqrt(m_SpawnCount)+1));
	int spacing = 3;

	int x = 0;
	int y = 0;

	for (int i = 0; i < m_SpawnCount; i++)
	{
		Tile* t = Terrain::GetInstance()->GetMovableTile(spawnPoint.x + x * spacing, spawnPoint.z + y* spacing, ENTITY_INVALID, Terrain::GetInstance()->GetTile(spawnPoint.x + x, spawnPoint.z + y));
		Entity entity = EntityFactory::CreateSquadUnit(static_cast<float>(t->X), static_cast<float>(t->Y), 0, player);

		if (m_SpawnUpgradeBack != UPGRADE_NAME::EMPTY)
			EntityFactory::CreateSquadUnitModule(entity, player, m_SpawnUpgradeBack);
		if (m_SpawnUpgradeTop != UPGRADE_NAME::EMPTY)
			EntityFactory::CreateSquadUnitModule(entity, player, m_SpawnUpgradeTop);
		if (m_SpawnUpgradeSides != UPGRADE_NAME::EMPTY)
			EntityFactory::CreateSquadUnitModule(entity, player, m_SpawnUpgradeSides);
		if (m_SpawnUpgradeFront != UPGRADE_NAME::EMPTY)
			EntityFactory::CreateSquadUnitModule(entity, player, m_SpawnUpgradeFront);

		x++;
		if (x > row)
		{
			y++;
			x = 0;
		}
	}

	// TODOOE: For playtesting, only want to start with one squad.
	//for ( int i = 0; i < AGENTS; i++ )
	//{
	//	Entity entity = EntityFactory::CreateSquadUnit( spawnPoint.x + i * 4.0f, spawnPoint.z + 5.0f, 1, player );
	//	EntityFactory::CreateSquadUnitModule( entity, player, WEAPON_PINCERS );
	//	EntityFactory::CreateSquadUnitModule( entity, player, MOVEMENT_LEGS );
	//}
}

#if AI_BALANCE_TEST == 1
void SSAI::BalanceTest(int upg1, int upg2)
{
	int agents = 25;
	int player = 0;
	int dist = 80;

	AddSquad(tNew(Squad, player, 0));

	float x = g_GameData.GetFieldHeight() / 2.0f;
	float y = g_GameData.GetFieldHeight() / 3.0f;

	for (int i = 0; i < agents; ++i)
	{
		Entity entity = EntityFactory::CreateSquadUnit(x - dist, y, 0, player);
		//EntityFactory::CreateSquadUnitModule(entity, player, upg1);
		EntityFactory::CreateSquadUnitModule(entity, player, MOVEMENT_LEGS);
	}

	m_Teams[player].Squads[0]->SetUpgradeTo(static_cast<UPGRADE_NAME>(upg1));
	m_Teams[player].Squads[0]->AddMission(nullptr, Squad::MISSION_TYPE_UPGRADE, true);
	m_Teams[player].Squads[0]->AddMission(Terrain::GetInstance()->GetTile(x + dist, y), Squad::MISSION_TYPE_MOVE, false);

	player = 1;

	AddSquad(tNew(Squad, player, 0));

	for (int i = 0; i < agents; ++i)
	{
		Entity entity = EntityFactory::CreateSquadUnit(x + dist, y, 0, player);
		//EntityFactory::CreateSquadUnitModule(entity, player, WEAPON_TANK_CANNON);
		EntityFactory::CreateSquadUnitModule(entity, player, MOVEMENT_LEGS);
		//EntityFactory::CreateSquadUnitModule(entity, player, UTILITY_PASSIVE_IMPROVED_SENSORS);
	}

	m_Teams[player].Squads[0]->SetUpgradeTo(static_cast<UPGRADE_NAME>(upg2));
	m_Teams[player].Squads[0]->AddMission(nullptr, Squad::MISSION_TYPE_UPGRADE, true);
	m_Teams[player].Squads[0]->AddMission(Terrain::GetInstance()->GetTile(x - dist, y), Squad::MISSION_TYPE_MOVE, false);
}
#endif

int SSAI::ScriptIncreaseSquadSize( IScriptEngine* scriptEngine )
{
	int playerID		= scriptEngine->PopInt();
	int squadID			= scriptEngine->PopInt();
	int sizeIncrease	= scriptEngine->PopInt();

	this->IncreaseSquadSize( sizeIncrease, squadID, playerID );

	return 0; // Number of return values
}

int SSAI::ScriptDecreaseSquadSize( IScriptEngine* scriptEngine )
{
	int playerID		= scriptEngine->PopInt();
	int squadID			= scriptEngine->PopInt();
	int sizeDecrease	= scriptEngine->PopInt();

	this->DecreaseSquadSize( sizeDecrease, squadID, playerID );

	return 0; // Number of return values
}

int SSAI::ScriptChangeSquadSize( IScriptEngine* scriptEngine )
{
	int playerID		= scriptEngine->PopInt();
	int squadID			= scriptEngine->PopInt();
	int newSize			= scriptEngine->PopInt();

	this->ChangeSquadSize( newSize, squadID, playerID );

	return 0; // Number of return values
}

int SSAI::ScriptKillPlayer( IScriptEngine* scriptEngine )
{
	int playerID		= scriptEngine->PopInt();

	this->KillPlayer( playerID );

	return 0; // Number of return values
}

void SSAI::Shutdown()
{
	bool winnerDeclared = false;
	for (int i = static_cast<int>(m_AIPlayers.size()) - 1; i > -1; i--)
	{
		if (m_AIPlayers[i]->IsWinner())
			winnerDeclared = true;
	}

	if (!winnerDeclared)
	{
		for (int i = static_cast<int>(m_AIPlayers.size()) - 1; i > -1; i--)
		{
			if (m_AIPlayers[i]->GetSize() > 0)
				m_AIPlayers[i]->SetIsWinner(true);
		}
	}

	EntityMask agentFlag = DenseComponentCollection<AgentComponent>::GetInstance().GetComponentTypeFlag();
	int entityID = 0;
	for (auto& entityMask : EntityManager::GetInstance().GetEntityMasks())
	{
		// Check if entity has a placement component
		if ((entityMask & agentFlag))
		{
			AgentComponent* agent = GetDenseComponent<AgentComponent>(entityID);
			tDelete(agent->Agent);
		}
		entityID++;
	}

	for (int i = static_cast<int>(m_Teams.size()) - 1; i > -1; i--)
	{
		for (int j = static_cast<int>(m_Teams[i].Squads.size()) - 1; j > -1; j--)
		{
			tDelete(m_Teams[i].Squads[j]);
		}
		m_Teams[i].Squads.clear();
	}

	if (!m_AIPlayers.empty())
		SaveGenes();

	for (int i = static_cast<int>(m_AIPlayers.size()) - 1; i > -1; i--)
	{
		pDelete(m_AIPlayers[i]);
	}
	m_AIPlayers.clear();

	//Terrain::GetInstance()->InitField();

	g_SSMail.UnregisterSubscriber(this);
	Subsystem::Shutdown();
}

int SSAI::SaveGenes()
{
	rVector<int> AIFitness;
	AIFitness.resize(m_AIPlayers.size());
	int total = 0;
	int bots = static_cast<int>(AIFitness.size());
	for (int i = 0; i < bots; i++)
	{
		AIFitness[i] = m_AIPlayers[i]->GetGeneFitness();
		for (int j = 0; j < m_Teams[i].Squads.size(); j++)
		{
			AIFitness[i] += m_Teams[i].Squads[j]->GetSize()*30;
		}
		
		total += AIFitness[i];
	}

	int average = total / bots;

	for (int i = 0; i < bots; i++)
	{
		AIFitness[i] = static_cast<int>(pow(AIFitness[i] - average, 2));
	}

	int totalDiff = 0;
	for (int i = 0; i < bots; i++)
	{
		totalDiff += AIFitness[i];
	}

	int standardDeviation = static_cast<int>(sqrt(totalDiff / bots));

	for (int i = 0; i < bots; i++)
	{
		int scaledFit=m_AIPlayers[i]->GetGeneFitness() - (average - standardDeviation);

		if (scaledFit < 0)
			scaledFit = 0;
		if (scaledFit > 100)
			scaledFit = 100;

		m_AIPlayers[i]->SetScaledFitness(scaledFit);
	}
	

	return standardDeviation;
}

void SSAI::PlayerLost(int id)
{
	if (m_AIPlayers.size() > 0)
	{
		for (int i = 0; i < m_AIPlayers.size(); i++)
		{
			if (m_AIPlayers[i]->GetTeamID() == id)
			{
				if (m_DeadAIplayers == 0)
					m_AIPlayers[i]->SetGeneFitness(0);
				else
					m_AIPlayers[i]->SetGeneFitness(m_DeadAIplayers * 25 + m_AIPlayers[i]->GetGeneFitness());
				break;
			}
		}
		m_DeadAIplayers++;
	}
}

void SSAI::SetWinner(int id)
{
	if (m_AIPlayers.size() == 0)
		return;

	for (int i = 0; i < m_AIPlayers.size(); i++)
	{
		if (m_AIPlayers[i]->GetTeamID() == id && m_AIPlayers[i]->GetIsNeuralNetAI())
		{
			m_AIPlayers[i]->SetGeneFitness(100 + m_AIPlayers[i]->GetGeneFitness());
			m_AIPlayers[i]->SetIsWinner(true);
			return;
		}
	}

	//winner did not use neural nets, just set any neural net to winner so weights are saved

	int bestFitness=-1000;
	int bestIndex = 0;

	for (int i = 0; i < m_AIPlayers.size(); i++)
	{
		if (m_AIPlayers[i]->GetIsNeuralNetAI())
		{
			int fit = m_AIPlayers[i]->GetGeneFitness();
			if (fit > bestFitness)
			{
				bestIndex = i;
				bestFitness = fit;
			}
		}
	}

	m_AIPlayers[bestIndex]->SetGeneFitness(100 + m_AIPlayers[bestIndex]->GetGeneFitness());
	m_AIPlayers[bestIndex]->SetIsWinner(true);
	return;
}

