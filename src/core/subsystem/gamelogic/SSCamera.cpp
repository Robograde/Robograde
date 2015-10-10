/**************************************************
Copyright 2015 Ola Enberg & Johan Melin
***************************************************/

#include "SSCamera.h"
#include "../../CompileFlags.h"
#include <utility/Logger.h>
#include <input/KeyBindings.h>
#include <gfx/GraphicsEngine.h>
#include "SSAI.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../datadriven/EntityManager.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/PlacementComponent.h"
#include "../../utility/GameModeSelector.h"
#include "../input/SSKeyBinding.h"
#include "SSControlPoint.h"

SSCamera& SSCamera::GetInstance( )
{
	static SSCamera instance;
	return instance;
}

void SSCamera::Startup( )
{
	g_Script.Register( "GE_SetCameraPosition",				std::bind( &SSCamera::ScriptSetPosition,		this, std::placeholders::_1 ) );
	g_Script.Register( "GE_CameraLookAt",					std::bind( &SSCamera::ScriptLookAt,				this, std::placeholders::_1 ) );
	g_Script.Register( "GE_SetCameraMoveSpeed",				std::bind( &SSCamera::ScriptSetMoveSpeed,		this, std::placeholders::_1 ) );
	g_Script.Register( "GE_SetCameraAlternativeMoveSpeed",	std::bind( &SSCamera::ScriptSetAltMoveSpeed,	this, std::placeholders::_1 ) );
	g_Script.Register( "GE_SetCameraFOV",					std::bind( &SSCamera::ScriptSetFOV,				this, std::placeholders::_1 ) );
	DEV(
			g_Script.Register( "GE_SetCameraBoundsActive",			std::bind( &SSCamera::ScriptSetBoundsActive,	this, std::placeholders::_1 ) );
			g_Script.Register( "GE_ActivateCameraRTS",				std::bind( &SSCamera::ScriptActivateCameraRTS,	this, std::placeholders::_1 ) );
			g_Script.Register( "GE_ActivateCameraFP",				std::bind( &SSCamera::ScriptActivateCameraFP,	this, std::placeholders::_1 ) );
	   );
	SetWindowSize( g_GameData.GetWindowWidth(), g_GameData.GetWindowHeight() );
	SetMapSize((float)g_GameData.GetFieldWidth(), (float)g_GameData.GetFieldHeight());

	if ( m_WindowWidth == 0 || m_WindowHeight == 0 )
	{
		m_WindowWidth = m_RTS.GetLens().WindowWidth;
		m_WindowHeight = m_RTS.GetLens().WindowHeight;
		Logger::Log( "Valid window size not set, defaulting to width: " + rToString( m_RTS.GetLens( ).WindowWidth ) +
			" height: " + rToString( m_RTS.GetLens( ).WindowHeight ), "SSCamera", LogSeverity::WARNING_MSG );
	}

	if ( m_MapSizeX == 0.0f || m_MapSizeZ == 0.0f )
	{
		m_MapSizeX = CAMERA_SUBSYSTEM_DEFAULT_MAP_WIDTH;
		m_MapSizeZ = CAMERA_SUBSYSTEM_DEFAULT_MAP_DEPTH;
		Logger::Log( "Valid map size not set, defaulting to width: " + rToString( CAMERA_SUBSYSTEM_DEFAULT_MAP_WIDTH ) +
			" depth: " + rToString( CAMERA_SUBSYSTEM_DEFAULT_MAP_DEPTH ), "SSCamera", LogSeverity::WARNING_MSG );
	}

	glm::vec3 startPosition = this->CalculateStartPosition();
	const GameModeType& gameModeType = g_GameModeSelector.GetCurrentGameMode().Type;

	m_RTS.GetEditableLens().WindowWidth	= m_WindowWidth;
	m_RTS.GetEditableLens().WindowHeight = m_WindowHeight;
	if ( gameModeType != GameModeType::AIOnly && gameModeType != GameModeType::Replay )
		m_RTS.GetEditableLens().Far = 500.0f;
	else
		m_RTS.GetEditableLens().Far = 1000.0f;

	m_RTS.GetEditableLens().Near = 1.0f;
	m_RTS.SetPosition( startPosition );

	float terrainmax = gfx::g_GFXTerrain.GetMaxHeight();
	float terrainmin = gfx::g_GFXTerrain.GetMinHeight();
	float maxheight = terrainmax * 1.5f;
	float minheight = terrainmax - terrainmax;

	float avgHeight = g_SSControlPoint.GetAverageHeight();

	if ( g_GameModeSelector.GetCurrentGameMode().Type != GameModeType::Replay && g_GameModeSelector.GetCurrentGameMode().Type != GameModeType::AIOnly )
	{
		m_MaxZoom = terrainmax * 0.5f;
		m_MinZoom = terrainmax * 1.2f;
	}
	else
	{
		m_MaxZoom = terrainmax - avgHeight;
		m_MinZoom = terrainmax + 120;
	}
	m_RTS.SetMovementBounds( glm::vec3( 0.0f, m_MaxZoom, 0.0f ), glm::vec3( m_MapSizeX, m_MinZoom, m_MapSizeZ ) );

	m_FirstPerson.GetEditableLens().WindowWidth = m_WindowWidth;
	m_FirstPerson.GetEditableLens().WindowHeight = m_WindowHeight;
	if ( gameModeType != GameModeType::AIOnly && gameModeType != GameModeType::Replay )
		m_FirstPerson.GetEditableLens().Far = 500.0f;
	else
		m_FirstPerson.GetEditableLens().Far = 1000.0f;
	m_FirstPerson.GetEditableLens().Near = 1.0f;
	m_FirstPerson.SetPosition( startPosition );

	m_Active = &m_RTS;

	Subsystem::Startup();
}

void SSCamera::UpdateUserLayer( const float deltaTime )
{
	DEV(
	if ( g_SSKeyBinding.ActionUpDown( ACTION_TOGGLE_CAMERA ) )
	{
		this->ToggleCamera();
	}
	);

	if ( g_SSKeyBinding.ActionDown( ACTION_JUMP_TO_SELECTION ) )
	{
		if ( g_PlayerData.GetSelectedSquads().empty() )
		{
			if ( !g_PlayerData.GetSelectedEntities().empty() )
			{
				const glm::vec3& position = GetDenseComponent<PlacementComponent>( g_PlayerData.GetSelectedEntities()[0] )->Position;
				m_RTS.LookAtPosition( position );
			}
		}
		else
		{
			const glm::vec2& squadPosition = g_SSAI.GetSquadWithID( g_PlayerData.GetPlayerID(), g_PlayerData.GetSelectedSquads()[0] )->GetPosition();
			m_RTS.LookAtPosition( glm::vec3( squadPosition.x, gfx::g_GFXTerrain.GetHeightAtWorldCoord( squadPosition.x, squadPosition.y ), squadPosition.y ) );
		}
	}

	if ( m_TransitionTimer > 0.0f )
	{
		m_TransitionTimer -= deltaTime;
		if ( m_TransitionTimer > 0.0f )
		{
			float transitionLeft	= 1.0f - m_TransitionTimer / CAMERA_SUBSYSTEM_TRANSITION_TIME;
			glm::vec3 cameraPos		= m_TransitionPosBase + transitionLeft * m_TransitionPosChange;
			m_FirstPerson.SetPosition( cameraPos );

			glm::quat cameraRot		= m_TransitionRotBase + transitionLeft * m_TransitionRotChange;
			m_FirstPerson.SetOrientation( glm::normalize( cameraRot ) );
		}
		else
		{
			m_Active = &m_RTS;
		}
	}
	else
	{
		if ( !m_BlockMovement )
		{
			m_Active->Update( deltaTime );
		}
	}

	m_Active->CalculateViewProjection( );
}

void SSCamera::ToggleCamera()
{
	if ( m_Active == &m_RTS )
	{
		m_Active = &m_FirstPerson;
		m_FirstPerson.SetPosition( m_RTS.GetPosition() );
		m_FirstPerson.SetOrientation( m_RTS.GetOrientation() );
	}
	else
	{
		glm::vec3	rtsForward	= m_RTS.GetForward();
		float		scale		= (m_RTS.GetPosition().y - m_FirstPerson.GetPosition().y) / m_RTS.GetForward().y;

		m_RTS.SetPosition( m_FirstPerson.GetPosition() + scale * rtsForward );

		m_TransitionTimer		= CAMERA_SUBSYSTEM_TRANSITION_TIME;
		m_TransitionPosBase		= m_FirstPerson.GetPosition();
		m_TransitionPosChange	= m_RTS.GetPosition() - m_TransitionPosBase;
		m_TransitionRotBase		= m_FirstPerson.GetOrientation();
		m_TransitionRotChange.w	= m_RTS.GetOrientation().w - m_TransitionRotBase.w;
		m_TransitionRotChange.x	= m_RTS.GetOrientation().x - m_TransitionRotBase.x;
		m_TransitionRotChange.y	= m_RTS.GetOrientation().y - m_TransitionRotBase.y;
		m_TransitionRotChange.z	= m_RTS.GetOrientation().z - m_TransitionRotBase.z;
	}
}

Camera* SSCamera::GetActiveCamera()
{
	return m_Active;
}

CameraRTS* SSCamera::GetRTSCamera()
{
	return &m_RTS;
}

void SSCamera::SetWindowSize( int width, int height )
{
	m_WindowWidth = width;
	m_WindowHeight = height;
	m_RTS.GetEditableLens().WindowWidth = m_WindowWidth;
	m_RTS.GetEditableLens().WindowHeight = m_WindowHeight;
	m_FirstPerson.GetEditableLens().WindowWidth = m_WindowWidth;
	m_FirstPerson.GetEditableLens().WindowHeight = m_WindowHeight;
}

void SSCamera::SetMapSize( float x, float z )
{
	m_MapSizeX = x;
	m_MapSizeZ = z;

	// TODOOE Get old y value for rts cam
	//m_RTS.SetMovementBounds( glm::vec3( 0.0f , 0.5f , 0.0f ) , glm::vec3( x , 80.0f , z ) );
}

void SSCamera::SetBlockMovement( bool newBlockMovement )
{
	m_BlockMovement = newBlockMovement;
}

glm::vec3 SSCamera::CalculateStartPosition()
{
	const GameModeType& gameModeType = g_GameModeSelector.GetCurrentGameMode().Type;
	if ( gameModeType != GameModeType::AIOnly && gameModeType != GameModeType::Replay )
	{
		if ( g_SSAI.m_Teams.size() == 0 )
			return glm::vec3( 20 , 40 , 20 );

		if ( g_SSAI.m_Teams[g_PlayerData.GetPlayerID()].Squads.empty() || g_SSAI.m_Teams[g_PlayerData.GetPlayerID()].Squads[0]->GetAgents().empty() )
		{
			return glm::vec3( 20, 40, 20 );
		}

		Entity		playerUnitID	= g_SSAI.m_Teams[g_PlayerData.GetPlayerID()].Squads[0]->GetAgents()[0]->GetEntityID();
		EntityMask	playerUnitMask	= g_EntityManager.GetEntityMask( playerUnitID );
		EntityMask	placementFlag	= DenseComponentCollection<PlacementComponent>::GetInstance().GetComponentTypeFlag();

		if ( !( playerUnitMask & placementFlag ) )
		{
			return glm::vec3( 20, 40, 20 );
		}

		PlacementComponent* placementComponent = GetDenseComponent<PlacementComponent>( playerUnitID );

		vec3 startpos = placementComponent->Position - 80.0f * m_RTS.GetForward();
		startpos.y = gfx::g_GFXTerrain.GetHeightAtWorldCoord(placementComponent->Position.x, placementComponent->Position.z) + 80.0f;
		return startpos;
	}
	else
	{
		return glm::vec3( g_GameData.GetFieldWidth() / 2.0f, 450.0f, ( g_GameData.GetFieldHeight() / 2 ) + 360 ); // Here be magic numbers that only works for levels of dimensions 512 x 512
	}
}

int SSCamera::ScriptSetPosition( IScriptEngine* scriptEngine )
{
	glm::vec3	newPosition;
				newPosition.z	= scriptEngine->PopFloat();
				newPosition.y	= scriptEngine->PopFloat();
				newPosition.x	= scriptEngine->PopFloat();

	m_Active->SetPosition( newPosition );

	return 0; // Number of return values.
}

int SSCamera::ScriptLookAt( IScriptEngine* scriptEngine )
{
	glm::vec3	targetPosition;
				targetPosition.z	= scriptEngine->PopFloat();
				targetPosition.y	= scriptEngine->PopFloat();
				targetPosition.x	= scriptEngine->PopFloat();

	m_Active = &m_RTS;
	m_RTS.LookAtPosition( targetPosition );

	return 0; // Number of return values.
}

int SSCamera::ScriptSetMoveSpeed( IScriptEngine* scriptEngine )
{
	float newSpeed = scriptEngine->PopFloat();

	m_Active->SetMoveSpeed( newSpeed );

	return 0;	// Number of return values.
}

int SSCamera::ScriptSetAltMoveSpeed( IScriptEngine* scriptEngine )
{
	float newSpeed = scriptEngine->PopFloat();

	m_FirstPerson.SetAlternativeMoveSpeed( newSpeed );
	
	return 0;	// Number of return values.
}

int SSCamera::ScriptSetFOV( IScriptEngine* scriptEngine )
{
	float newFOV = scriptEngine->PopFloat();

	m_Active->GetEditableLens().VerticalFOV = newFOV;
	
	return 0;	// Number of return values.
}

int SSCamera::ScriptSetBoundsActive( IScriptEngine* scriptEngine )
{
	bool boundsActive = scriptEngine->PopBool();

	m_RTS.SetBoundsActive( boundsActive );
	
	return 0;	// Number of return values.
}

int SSCamera::ScriptActivateCameraRTS( IScriptEngine* scriptEngine )
{
	if ( m_Active != &m_RTS )
	{
		this->ToggleCamera();
	}
	
	return 0;	// Number of return values.
}

int SSCamera::ScriptActivateCameraFP( IScriptEngine* scriptEngine )
{
	if ( m_Active != &m_FirstPerson )
	{
		this->ToggleCamera();
	}
	
	return 0;	// Number of return values.
}
