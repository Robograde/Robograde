/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson & Henrik Johansson
***************************************************/

#pragma once
#include <SDL2/SDL.h>
#include "UtilityLibraryDefine.h"
class GameTimer
{
public:
	UTILITY_API struct DeltaTimeInfo;

	UTILITY_API GameTimer();

	UTILITY_API void			Start	( );
	UTILITY_API void			Stop	( );
	UTILITY_API void			Pause	( );
	UTILITY_API void			UnPause	( );
	UTILITY_API void			Tick	( );
	UTILITY_API double			Restart	( );
	UTILITY_API float			GetTimeAsFloat	( )	const;
	UTILITY_API double			GetTimeAsDouble ( )	const;
	UTILITY_API float			GetDeltaAsFloat	( ) const;
	UTILITY_API double			GetDeltaAsDouble( ) const;
	UTILITY_API DeltaTimeInfo	GetDeltaTimeInfo( ) const;
	UTILITY_API Uint64			GetDeltaTicks	( ) const;
	UTILITY_API Uint64			GetTicksPerSec	( ) const;

	struct DeltaTimeInfo
	{
		Uint64	DeltaTicks;
		Uint64	TicksPerSec;
		float	DeltaTime;

		DeltaTimeInfo( Uint64 deltaTicks, Uint64 ticksPerSec, float deltaTime ) 
			: DeltaTicks( deltaTicks ), TicksPerSec( ticksPerSec ), DeltaTime( deltaTime ) {}
	};

private:
	Uint64	m_StartTicks, m_PausedTicks, m_LastCheckedTicks;
	Uint64	m_TicksPerSec;
	Uint64	m_DeltaTicks;
	bool	m_Started, m_Paused;
	double	m_DeltaTime;
};
