/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "GameSpeedController.h"
#include "../CompileFlags.h"
#include <network/NetworkInfo.h>
#include <utility/Logger.h>
#include <input/Input.h>
#include <input/KeyBindings.h>
#include <messaging/GameMessages.h>
#include "GameData.h"
#include "GameModeSelector.h"
#include "../subsystem/utility/SSMail.h"
#include "../subsystem/menu/SSInGameMenu.h"
#include "../utility/PlayerData.h"
#include "../subsystem/input/SSKeyBinding.h"

#define AUTO_DEBUG_SPEED_MULTIPLIER 3

GameSpeedController& GameSpeedController::GetInstance( )
{
	static GameSpeedController instance;
	return instance;
}

void GameSpeedController::Startup()
{
	if ( g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::AIOnly )
		for ( int i = 0; i < AUTO_DEBUG_SPEED_MULTIPLIER; ++i )
			IncreaseSpeed();

	DEV(
		g_Script.Register( "ToggleUserLayer", [&] ( IScriptEngine* ) -> int { ToggleUserLayer(); return 0; } );
	);
	g_Script.Register( "ToggleSimLayer", [&] ( IScriptEngine* ) -> int { ToggleSimLayer(); return 0; } );
}

void GameSpeedController::Update()
{
	DEV(
		const GameMode& currentGameMode = g_GameModeSelector.GetCurrentGameMode();
		if ( currentGameMode.IsSimulation && !g_GameData.IsDedicatedServer() )
		{
			if ( g_SSKeyBinding.ActionUpDown( ACTION_INCREASE_SIMULATION_RATE ) )
				IncreaseSpeed();
			else if ( g_SSKeyBinding.ActionUpDown( ACTION_DECREASE_SIMULATION_RATE ) )
				DecreaseSpeed();
		}
		if ( g_SSKeyBinding.ActionDownUp( ACTION_TOGGLE_USER_LAYER ) )
			ToggleUserLayer();
	);
}

void GameSpeedController::Reset()
{
	m_AccumulatedTicks	= 0;
	m_UpdateCount		= 0;
	m_UserLayerActive	= true;
	m_SimLayerActive	= true;
	m_Gamestarted		= false;
	m_TicksPerFrame		= m_TicksPerSecond / DEFAULT_SIMULATION_RATE;
}

bool GameSpeedController::ShouldUpdateUserLayer() const
{
	return m_UserLayerActive;
}

bool GameSpeedController::ShouldUpdateSimLayer( const GameTimer::DeltaTimeInfo& deltaTimeInfo )
{
	bool toReturn = false;

	if ( m_SimLayerActive && m_Gamestarted )
	{
		GameMode gameMode = g_GameModeSelector.GetCurrentGameMode();
		if ( gameMode.IsSimulation )
		{
			if ( !gameMode.IsNetworked || g_NetworkInfo.AmIHost() )
			{
				if ( m_UpdateCount > 2 ) // The first two deltaticks are bad since one is from the menu and the second is from loading screen
					m_AccumulatedTicks += deltaTimeInfo.DeltaTicks;

				++m_UpdateCount;

				if ( m_AccumulatedTicks >= m_TicksPerFrame ) // If it is time to take another step
				{
					m_AccumulatedTicks -= m_TicksPerFrame;
					toReturn = true;
				}
			}
			else if ( g_NetworkInfo.DecrementFramesToRun() ) // If host say we can run a frame  
				toReturn = true;
		}
	}
	return toReturn;
}

bool GameSpeedController::GetUserLayerActive() const
{
	return m_UserLayerActive;
}

bool GameSpeedController::GetSimLayerActive() const
{
	return m_SimLayerActive;
}

bool GameSpeedController::GetGamestarted() const
{
	return m_Gamestarted;
}

void GameSpeedController::SetUserLayerActive( bool shouldRunUserLayer )
{
	m_UserLayerActive = shouldRunUserLayer;
}

void GameSpeedController::SetSimLayerActive( bool shouldRunSimLayer )
{
	m_SimLayerActive = shouldRunSimLayer;
}

void GameSpeedController::SetGameStarted( bool gameStarted )
{
	m_Gamestarted = gameStarted;
}

void GameSpeedController::ToggleUserLayer()
{
	m_UserLayerActive = !m_UserLayerActive;
}

void GameSpeedController::ToggleSimLayer()
{
	m_SimLayerActive = !m_SimLayerActive;
}

void GameSpeedController::IncreaseSpeed( )
{
	if ( m_TicksPerSecond / m_TicksPerFrame < 920U )
	{
		m_TicksPerFrame = m_TicksPerFrame >> 1; // Division by 2
		Logger::Log( "Game speed was increased to " + rToString( m_TicksPerSecond / m_TicksPerFrame ) + " FPS", "GameSpeedController", LogSeverity::INFO_MSG );
	}
}

void GameSpeedController::DecreaseSpeed( )
{
	if ( m_TicksPerSecond / m_TicksPerFrame > 1 )
	{
		m_TicksPerFrame = m_TicksPerFrame << 1; // Multiplication with 2
		Logger::Log( "Game speed was decreased to " + rToString( m_TicksPerSecond / m_TicksPerFrame ) + " FPS", "GameSpeedController", LogSeverity::INFO_MSG );
	}
}
