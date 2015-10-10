/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include <SDL2/SDL.h>
#include <utility/GameTimer.h>

#define g_GameSpeedController GameSpeedController::GetInstance()
#define DEFAULT_SIMULATION_RATE 30
static_assert( DEFAULT_SIMULATION_RATE > 0, "DEFAULT_SIMULATION_RATE cannot be lower than 1" );

class GameSpeedController
{
public:
	static GameSpeedController& GetInstance();
	
	void Startup();
	void Update();
	void Reset();
	bool ShouldUpdateUserLayer() const;
	bool ShouldUpdateSimLayer( const GameTimer::DeltaTimeInfo& deltaTimeInfo );

	bool GetUserLayerActive() const;
	bool GetSimLayerActive() const;
	bool GetGamestarted() const;

	void SetUserLayerActive( bool shouldRunUserLayer );
	void SetSimLayerActive( bool shouldRunSimLayer );
	void SetGameStarted( bool gameStarted );

	void ToggleUserLayer();
	void ToggleSimLayer();

private:
	// No external instancing allowed
	GameSpeedController() { }
	GameSpeedController( const GameSpeedController& rhs );
	~GameSpeedController() { }
	GameSpeedController& operator=( const GameSpeedController& rhs );

	void IncreaseSpeed();
	void DecreaseSpeed();

	const Uint64	m_TicksPerSecond		= SDL_GetPerformanceFrequency( );
	Uint64			m_TicksPerFrame			= m_TicksPerSecond / DEFAULT_SIMULATION_RATE;
	Uint64			m_AccumulatedTicks		= 0;
	Uint64			m_UpdateCount			= 0;
	bool			m_UserLayerActive		= true;
	bool			m_SimLayerActive		= true;
	bool			m_Gamestarted			= false;
};