/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include <utility/GameTimer.h>

const int PING_INTERVAL = 3; // Seconds

class Pinger
{
public:
	Pinger( short targetNetworkID );
	void		HandlePingSend( );
	double		HandlePingReceive( );

	short		GetTargetNetworkID( ) const;
	double		GetPingTime( ) const;

private:
	Uint64		m_AccumulatedTimerTicks;
	short		m_TargetNetworkID = -1;
	bool		m_PingSent = false;
	double		m_LastPingTime = -1.0;
	GameTimer	m_Timer;
	double		m_PingSendTimeStamp = 0.0;
};