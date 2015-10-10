/**************************************************
Copyright 2015 Robograde Development Team
***************************************************/

#include "SSControlPoint.h"
#include "../../CompileFlags.h"
#include <utility/Logger.h>
#include <utility/Colours.h>
#include <messaging/GameMessages.h>
#include <gfx/GraphicsEngine.h>
#include <gfx/Terrain.h>
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/OwnerComponent.h"
#include "../../component/ControlPointComponent.h"
#include "../../component/ColourComponent.h"
#include "../../component/ModelComponent.h"
#include "../../ai/Terrain.h"
#include "../utility/SSMail.h"
#include "../input/SSPicking.h"
#include "../gui/SSMiniMap.h"
#include "../../utility/Alliances.h"
#include "../../utility/GameData.h"
#include "../../utility/GameModeSelector.h"
#include "../../utility/PlayerData.h"
#include "../../EntityFactory.h"
#include "SSAI.h"
#include "../../component/ChildComponent.h"


SSControlPoint& SSControlPoint::GetInstance()
{
	static SSControlPoint instance;
	return instance;
}

void SSControlPoint::Startup()
{
	DEV(
			g_Script.Register( "GE_SetAllNeutral",				std::bind( &SSControlPoint::ScriptSetAllNeutral,				this, std::placeholders::_1 ) );
			g_Script.Register( "GE_SetAllBelongTo",				std::bind( &SSControlPoint::ScriptSetAllBelongTo,				this, std::placeholders::_1 ) );
			g_Script.Register( "GE_SetAllNeutralThatBelongTo",	std::bind( &SSControlPoint::ScriptSetAllNeutralThatBelongTo,	this, std::placeholders::_1 ) );
			g_Script.Register( "GE_SetNeutral",					std::bind( &SSControlPoint::ScriptSetNeutral,					this, std::placeholders::_1 ) );
			g_Script.Register( "GE_SetSetBelongTo",				std::bind( &SSControlPoint::ScriptSetBelongTo,					this, std::placeholders::_1 ) );
	   );

	g_Script.Register( "GE_SelectUnmovedSquads", [] ( IScriptEngine* ) -> int
	{
		if ( g_GameModeSelector.GetCurrentGameMode( ).Type != GameModeType::SingleplayerAI
			&& g_GameModeSelector.GetCurrentGameMode( ).Type != GameModeType::Singleplayer
			&& g_GameModeSelector.GetCurrentGameMode( ).Type != GameModeType::Multiplayer )
			return 0;

		rVector<int>& selection = g_PlayerData.GetEditableSelectedSquads( );
		selection.clear( );

		for ( auto& squad : g_SSAI.m_Teams[g_PlayerData.GetPlayerID( )].Squads )
			if ( !squad->HasMoved( ) )
				selection.push_back( squad->GetID( ) );

		return 0;
	} );

	for ( int i = 0; i < MAX_PLAYERS; ++i )
		m_ControlPointsPerPlayer[i] = 0;

	glm::ivec2 winSize = g_GUI.GetWindowSize( "InGameWindow" );
	g_GUI.AddWindow( "ControlPoints", GUI::Rectangle( 0, 0, winSize.x, winSize.y ), "InGameWindow" );
	g_GUI.OpenWindow( "ControlPoints" );

	Subsystem::Startup();
}

void SSControlPoint::Shutdown()
{
	m_ControlPoints.clear();
	m_ControlPointsContested.clear();
	m_Counters.clear();
	g_GUI.DeleteObject( "ControlPoints" );
}

void SSControlPoint::UpdateSimLayer( const float timestep )
{
	// Reset variables
	for ( int i = 0; i < MAX_PLAYERS; ++i )
		m_ControlPointsPerPlayer[i] = 0;
	m_UnownedControlPoints = 0;

	for ( int i = 0; i < m_ControlPoints.size(); ++i )
	{
		// Calculate how many control points each player has
		Entity controlPoint = m_ControlPoints[i];

		ParentComponent* parentComponent = GetDenseComponent<ParentComponent>(controlPoint);
		ChildComponent* childComponent = GetDenseComponent<ChildComponent>(parentComponent->Children[MODULE_SLOT_TOP]);

		float rotationAmount = 0.05f*timestep;
		glm::quat rotation(glm::cos(rotationAmount), glm::vec3(0,1,0) * glm::sin(rotationAmount));
		childComponent->Orientation = glm::normalize(rotation * childComponent->Orientation);

		OwnerComponent* ownerComponent = GetDenseComponent<OwnerComponent>( controlPoint );
		short ownerID = ownerComponent->OwnerID;
		if (ownerID != NO_OWNER_ID)
		{
			for (int j = 0; j < MAX_PLAYERS; j++)
			{
				if (j == ownerID)
					++m_ControlPointsPerPlayer[ownerID];
				else if (g_Alliances.IsAllied(j, ownerID))
					++m_ControlPointsPerPlayer[j];
			}
		}
		else
			++m_UnownedControlPoints;

		bool isContested = CheckForCapture( ownerID, controlPoint );
		if ( !m_ControlPointsContested[i] && isContested )
			g_SSMail.PushToNextFrame( ControlPointMessage( controlPoint, ownerID, INVALID_OWNER_ID, CONTROL_POINT_STATE_CHANGE_CONTESTED ) );
		m_ControlPointsContested[i] = isContested;

		

		// Spawn units from control point
		ControlPointComponent* controlPointComponent = GetDenseComponent<ControlPointComponent>( m_ControlPoints[i] );
		controlPointComponent->IsContested = isContested;
		if ( ownerComponent->OwnerID != NO_OWNER_ID )
		{
			controlPointComponent->UnitSpawnTimer += timestep;
			PlacementComponent* placementComponent	= GetDenseComponent<PlacementComponent>( m_ControlPoints[i] );
			ColourComponent*	colourComponent		= GetDenseComponent<ColourComponent>( m_ControlPoints[i] );
			colourComponent		->Colour			= g_GameData.GetPlayerColour( ownerID );

			if ( controlPointComponent->UnitSpawnTimer >= CONTROL_POINT_UNIT_SPAWN_TIMER )
			{
				bool spawnedUnits = false;
				for ( auto& ally : g_Alliances.GetAllies( ownerComponent->OwnerID ) )
				{
					if ( g_SSAI.m_Teams.at( ally ).Squads.size( ) < SQUAD_MAXIMUM_SQUAD_COUNT )
					{
						int		squadID;
						Squad*	squad = nullptr;

						if ( controlPointComponent->LastSquadSpawnForPlayer.find( ally ) != controlPointComponent->LastSquadSpawnForPlayer.end( ) )
							squad = g_SSAI.GetSquadWithID( ally, controlPointComponent->LastSquadSpawnForPlayer[ally] );

						if ( squad && !squad->HasMoved( ) && squad->GetSize( ) < SQUAD_MAXIMUM_UNIT_COUNT )
						{
							squadID = squad->GetID( );
						}
						else
						{
							squadID = g_SSAI.GetFreeSquadID( );
							squad	= tNew( Squad, ally, squadID );
							g_SSAI.AddSquad( squad );

							controlPointComponent->LastSquadSpawnForPlayer[ally] = squadID;
						}

						unsigned int unitSpawnCount = CONTROL_POINT_UNIT_SPAWN_COUNT / (unsigned int) g_Alliances.GetAllies( ownerComponent->OwnerID ).size( );
						for ( unsigned int i = 0; i < unitSpawnCount; ++i )
						{
							Tile* tile = Terrain::GetInstance( )->GetSurroundingFreeTile( placementComponent->Position.x, placementComponent->Position.z + 7.0f, ENTITY_INVALID );
							Entity unit = EntityFactory::CreateSquadUnit( (float) tile->X, (float) tile->Y, squadID, ally );
							EntityFactory::CreateSquadUnitModule( unit, ally, WEAPON_PINCERS );
							EntityFactory::CreateSquadUnitModule( unit, ally, MOVEMENT_LEGS );
						}
						g_SSMiniMap.PingLocation( placementComponent->Position, glm::vec3( 0.0f, 1.0f, 1.0f ) );
						g_SSMail.PushToUserLayer( SFXPingMessage(ownerID , 1) ); ///Type == 1 == spawn ping
						spawnedUnits = true;
					}
				}

				if ( spawnedUnits )
				{
					controlPointComponent->UnitSpawnTimer -= CONTROL_POINT_UNIT_SPAWN_TIMER;
				}
				else
				{
					m_Counters[i]->SetVisible( true );
					m_Counters[i]->SetPosition( g_SSPicking.WorldToScreen( placementComponent->Position + m_CounterYOffset ) );
					m_Counters[i]->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );
					m_Counters[i]->SetText( "Waiting to Spawn\nToo many squads" );
					controlPointComponent->UnitSpawnTimer = CONTROL_POINT_UNIT_SPAWN_TIMER;
					colourComponent->Colour *= 0.75f;
				}
			}
			else
			{
				m_Counters[i]->SetVisible( true );
				m_Counters[i]->SetPosition( g_SSPicking.WorldToScreen( placementComponent->Position + m_CounterYOffset ) );
				m_Counters[i]->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );
				m_Counters[i]->SetText( rToString( static_cast<int>( CONTROL_POINT_UNIT_SPAWN_TIMER - controlPointComponent->UnitSpawnTimer ) ) + "s\nuntil next squad spawns" );
			}
		}
		else
		{
			controlPointComponent->UnitSpawnTimer = 0.0f;
			m_Counters[i]->SetVisible( false );
		}
	}
}

bool SSControlPoint::RegisterControlPoint( Entity entityID )
{
	EntityMask entityMask = EntityManager::GetInstance().GetEntityMask( entityID );
	EntityMask controlPointFlag = GetDenseComponentFlag<ControlPointComponent>();
	if ( ( entityMask & controlPointFlag ) != controlPointFlag )
	{
		Logger::Log( "Attempted to register entity without ControlPointComponent", "SSControlPoint", LogSeverity::WARNING_MSG );
	}

	// Check for duplicates
	for ( int i = 0; i < m_ControlPoints.size(); ++i )
	{
		if ( m_ControlPoints[i] == entityID )
		{
			Logger::Log( "Attempted to register duplicate of control point", "SSControlPoint", LogSeverity::WARNING_MSG );
			return false;
		}
	}

	m_ControlPoints.push_back( entityID );
	m_ControlPointsContested.push_back( false );
	g_GUI.UseFont( FONT_ID_LEKTON_14 );
	m_Counters.push_back( g_GUI.AddText( "", GUI::TextDefinition( "", 0, 0, 0, 0 ), "ControlPoints" ) );

	g_GUI.BringWindowToFront( "ControlPoints" );
	return true;
}

unsigned int SSControlPoint::GetTotalControlPointCount() const
{
	return static_cast<unsigned int>( m_ControlPoints.size() );
}

unsigned int SSControlPoint::GetOwnedControlPointsCount(short ownerID) const
{
	if ( ownerID < MAX_PLAYERS && ownerID > -1 )
		return m_ControlPointsPerPlayer[ownerID];
	else
		Logger::Log( "Attempted to get owned control points count from invalid owner", "SSControlPoint", LogSeverity::WARNING_MSG );

	return 0;
}

unsigned int SSControlPoint::GetUnownedControlPointsCount() const
{
	return m_UnownedControlPoints;
}

const rVector<Entity>& SSControlPoint::GetControlPoints() const
{
	return m_ControlPoints;
}

void SSControlPoint::Capture( short newOwnerID, Entity controlPoint )
{
	OwnerComponent* ownerComponent = GetDenseComponent<OwnerComponent>( controlPoint );
	int oldOwnerID = ownerComponent->OwnerID;
	if ( newOwnerID != oldOwnerID )
	{
		ownerComponent->OwnerID = newOwnerID;
		GetDenseComponent<ColourComponent>(controlPoint)->Colour = g_GameData.GetPlayerColour( newOwnerID );

		int statusChange = newOwnerID != NO_OWNER_ID ? CONTROL_POINT_STATE_CHANGE_CAPTURED : CONTROL_POINT_STATE_CHANGE_LOST;
		g_SSMail.PushToNextFrame( ControlPointMessage( controlPoint, oldOwnerID, newOwnerID, statusChange ) );
	}
}

void SSControlPoint::SetAllNeutral()
{
	this->SetAllBelongTo( NO_OWNER_ID );
}

void SSControlPoint::SetAllBelongTo( short ownerID )
{
	for ( auto& controlPoint : m_ControlPoints )
	{
		OwnerComponent* ownerComponent = GetDenseComponent<OwnerComponent>( controlPoint );
		if ( ownerComponent->OwnerID != ownerID )
		{
			this->Capture( ownerID, controlPoint );
		}
	}
}

void SSControlPoint::SetAllNeutralThatBelongTo( short ownerID )
{
	for ( auto& controlPoint : m_ControlPoints )
	{
		OwnerComponent* ownerComponent = GetDenseComponent<OwnerComponent>( controlPoint );
		if ( ownerComponent->OwnerID == ownerID )
		{
			this->Capture( NO_OWNER_ID, controlPoint );
		}
	}
}

float SSControlPoint::GetAverageHeight()
{
	EntityMask placementMask = GetDenseComponentFlag<PlacementComponent>();
	EntityMask modelMask = GetDenseComponentFlag<ModelComponent>();
	EntityMask entityMask;
	float h = 0,c = 0;
	float scale;
	for(int i = 0; i < m_ControlPoints.size(); i++)
	{
		entityMask = g_EntityManager.GetEntityMask(m_ControlPoints[i]);
		if(entityMask & placementMask && entityMask & modelMask)
		{
			glm::vec3 position = GetDenseComponent<PlacementComponent>( m_ControlPoints[i] )->Position;
			scale = GetDenseComponent<ModelComponent>( m_ControlPoints[i] )->Max.y - GetDenseComponent<ModelComponent>( m_ControlPoints[i] )->Min.y;
			h += gfx::g_GFXTerrain.GetHeightAtWorldCoord(position.x, position.z);
			c += 1.0f;
		}
	}
	return (h / c) + scale;
}

bool SSControlPoint::CheckForCapture( short owner, Entity controlPoint )
{
	ControlPointComponent*	capturePointComponent	= GetDenseComponent<ControlPointComponent>( controlPoint );
	Terrain*				terrain					= Terrain::GetInstance( );

	glm::vec3 position = GetDenseComponent<PlacementComponent>( controlPoint )->Position;

	Terrain::EnemyZone zone = terrain->GetClosestEnemy( terrain->GetTile( position.x, position.z ), CONTROL_POINT_RADIUS, owner, 0, false );
	int capturerAmount		= zone.Amount;
	int othersAmount		= 0;
	int captureTeam			= NO_OWNER_ID;

	if ( capturerAmount > 0 )
	{
		captureTeam			= GetAgentPointer( zone.Closest )->GetTeam( );
		zone				= terrain->GetClosestEnemy( terrain->GetTile( position.x, position.z ), CONTROL_POINT_RADIUS, captureTeam, 0, false );
		othersAmount		= zone.Amount;
	}

	if ( capturerAmount > 0 && othersAmount > 0 )
	{
		// do nothing;contested
		return true;
	}
	else if ( capturerAmount > 0 )
	{
		// only one alliance in zone; capture
		if ( g_Alliances.IsAllied( capturePointComponent->OwnerID, captureTeam ) )
		{
			// already owned; do nothing
			return false;
		}
		else
		{
			// capture
			if ( capturePointComponent->CaptureTeam != captureTeam )
			{
				// tick down
				capturePointComponent->OwnerShipClock -= CONTROL_POINT_BASE_CAPTURE_SPEED + glm::min( (float) (capturerAmount * CONTROL_PONT_UNIT_CAPTURE_SPEED), CONTROL_POINT_MAX_UNIT_CAPTURE_RANGE );
				if ( capturePointComponent->OwnerShipClock <= CONTROL_CLOCK_CAPTURE / 2 )
				{
					capturePointComponent->OwnerID			= NO_OWNER_ID;
					capturePointComponent->CaptureTeam		= captureTeam;
					capturePointComponent->OwnerShipClock	= CONTROL_CLOCK_CAPTURE / 2;
					Capture( NO_OWNER_ID, controlPoint );
				}
			}
			else
			{
				// tick up
				capturePointComponent->OwnerShipClock += CONTROL_POINT_BASE_CAPTURE_SPEED + glm::min( (float) (capturerAmount * CONTROL_PONT_UNIT_CAPTURE_SPEED), CONTROL_POINT_MAX_UNIT_CAPTURE_RANGE );
				if ( capturePointComponent->OwnerShipClock >= CONTROL_CLOCK_CAPTURE )
				{
					capturePointComponent->OwnerID			= captureTeam;
					capturePointComponent->OwnerShipClock	= CONTROL_CLOCK_CAPTURE;
					Capture( captureTeam, controlPoint );
				}
			}
		}

		return true;
	}
	else if ( capturePointComponent->OwnerShipClock > 0 && capturePointComponent->CaptureTeam != capturePointComponent->OwnerID )
	{
		// tick back ( down; not owning )
		capturePointComponent->OwnerShipClock -= 1.0f;
		if ( capturePointComponent->OwnerShipClock <= 0 )
		{
			capturePointComponent->CaptureTeam		= capturePointComponent->OwnerID;
			capturePointComponent->OwnerShipClock	= 0;
		}

		return true;
	}
	else if ( capturePointComponent->OwnerShipClock < CONTROL_CLOCK_CAPTURE && capturePointComponent->OwnerID != NO_OWNER_ID )
	{
		// tick back ( up; owning )
		capturePointComponent->OwnerShipClock += 1.0f;
		
		return true;
	}

	// Not contested
	return false;
}

int SSControlPoint::ScriptSetAllNeutral( IScriptEngine* scriptEngine )
{
	this->SetAllNeutral();

	return 0; // Number of return values.
}

int SSControlPoint::ScriptSetAllBelongTo( IScriptEngine* scriptEngine )
{
	short	ownerID			= scriptEngine->PopInt();

	this->SetAllBelongTo( ownerID );

	return 0; // Number of return values.
}

int SSControlPoint::ScriptSetAllNeutralThatBelongTo( IScriptEngine* scriptEngine )
{
	short	ownerID			= scriptEngine->PopInt();

	this->SetAllNeutralThatBelongTo( ownerID );

	return 0; // Number of return values.
}

int SSControlPoint::ScriptSetNeutral( IScriptEngine* scriptEngine )
{
	Entity	controlPoint	= scriptEngine->PopInt();

	this->Capture( NO_OWNER_ID, controlPoint );

	return 0; // Number of return values.
}

int SSControlPoint::ScriptSetBelongTo( IScriptEngine* scriptEngine )
{
	Entity	controlPoint	= scriptEngine->PopInt();
	short	ownerID			= scriptEngine->PopInt();

	this->Capture( ownerID, controlPoint );

	return 0; // Number of return values.
}
