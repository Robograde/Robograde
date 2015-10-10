/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson & Henrik Johansson
***************************************************/

#include "GameTimer.h"

GameTimer::GameTimer( )
{
	m_StartTicks 		= 0;
	m_PausedTicks 		= 0;
	m_LastCheckedTicks 	= 0;
	m_Paused 			= true;
	m_Started 			= false;
}

void GameTimer::Start( )
{
	m_Started 			= true;
	m_Paused 			= false;
	m_StartTicks 		= SDL_GetPerformanceCounter( );
	m_LastCheckedTicks 	= SDL_GetPerformanceCounter( );
	m_TicksPerSec 		= SDL_GetPerformanceFrequency( );
}

void GameTimer::Stop( )
{
	m_StartTicks 	= 0;
	m_PausedTicks 	= 0;
	m_Paused 		= true;
	m_Started 		= false;
}

void GameTimer::Pause( )
{
	if ( !m_Paused && m_Started )
	{
		m_Paused = true;
		m_PausedTicks = SDL_GetPerformanceCounter( ) - m_StartTicks;
	}
}

void GameTimer::UnPause( )
{
	if ( m_Paused )
	{
		m_Paused = false;
		m_StartTicks = SDL_GetPerformanceCounter( ) - m_PausedTicks;
		m_PausedTicks = 0;
	}
}

/// <summary>
/// Calculates the time elapsed since this function was last called and sets the internal delta time to this value.
/// If the timer is paused or has not yet been started it sets the internal delta time value to 0.
/// </summary>
void GameTimer::Tick( )
{
	if ( !m_Paused && m_Started )
	{
		m_DeltaTicks = SDL_GetPerformanceCounter( ) - m_LastCheckedTicks;
		m_DeltaTime = m_DeltaTicks / static_cast<float>(m_TicksPerSec);
	}
	else
		m_DeltaTime = 0.0;

	m_LastCheckedTicks = SDL_GetPerformanceCounter( );
}

double GameTimer::Restart( )
{
	double elapsedTicks = GetTimeAsDouble();
	Start( );
	return elapsedTicks;
}

/// <summary>
/// Returns the time elapsed since the timer started.
/// If the timer is paused returns the time since it was paused
/// Returns 0 if the timer has not been started or has been stopped.
/// </summary>
float GameTimer::GetTimeAsFloat( ) const
{
	if ( m_Started )
	{
		if ( m_Paused )
			return static_cast<float>(m_PausedTicks);
		else
			return (SDL_GetPerformanceCounter( ) - m_StartTicks) / static_cast<float>(m_TicksPerSec);
	}
	return 0;
}

/// <summary>
/// Returns the time elapsed since the timer started.
/// If the timer is paused returns the time since it was paused
/// Returns 0 if the timer has not been started or has been stopped.
/// </summary>
double GameTimer::GetTimeAsDouble() const
{
	if ( m_Started )
	{
		if ( m_Paused )
			return static_cast<double>( m_PausedTicks );
		else
			return ( SDL_GetPerformanceCounter() - m_StartTicks ) / static_cast<double>( m_TicksPerSec );
	}
	return 0;
}

/// <summary>
/// Gets the delta time as float.
/// Returns 0 if the timer is paused.
/// </summary>
float GameTimer::GetDeltaAsFloat( ) const
{
	if ( !m_Paused && m_Started )
		return static_cast<float>(m_DeltaTime);
	else
		return 0.0f;
}

/// <summary>
/// Gets the delta time as double.
/// Returns 0 if the timer is paused.
/// </summary>
double GameTimer::GetDeltaAsDouble( ) const
{
	if ( !m_Paused && m_Started )
		return m_DeltaTime;
	else
		return 0.0;
}

/// <summary>
/// Gets a struct containing delta ticks, ticks per second and delta time as float
/// Returns a struct with 0 in all values if the timer is paused
/// </summary>
GameTimer::DeltaTimeInfo GameTimer::GetDeltaTimeInfo( ) const
{
	if ( !m_Paused && m_Started )
		return GameTimer::DeltaTimeInfo( m_DeltaTicks, m_TicksPerSec, static_cast<float>(m_DeltaTime) );
	else
		return GameTimer::DeltaTimeInfo( 0, 0, 0.0f );
}

/// <summary>
/// Gets the delta ticks as Uint64
/// Returns 0 if the timer is paused
/// </summary>
Uint64 GameTimer::GetDeltaTicks( ) const
{
	if ( !m_Paused && m_Started )
		return m_DeltaTicks;
	else
		return 0;
}

/// <summary>
/// Gets the ticks per sec as Uint64
/// Returns 0 if the timer is paused
/// </summary>
Uint64 GameTimer::GetTicksPerSec( ) const
{
	if ( !m_Paused && m_Started )
		return m_TicksPerSec;
	else
		return 0;
}