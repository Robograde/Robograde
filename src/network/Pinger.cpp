/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "Pinger.h"
#include "NetworkMessages.h"
#include "NetworkEngine.h"

Pinger::Pinger( short targetNetworkID )
{
	m_TargetNetworkID = targetNetworkID;
	m_Timer.Start( );
	m_AccumulatedTimerTicks = m_Timer.GetDeltaTimeInfo( ).TicksPerSec * PING_INTERVAL;
}

void Pinger::HandlePingSend( )
{
	m_Timer.Tick( );
	GameTimer::DeltaTimeInfo	deltaTimeInfo		= m_Timer.GetDeltaTimeInfo( );
	static Uint64				ticksToAccumulate	= deltaTimeInfo.TicksPerSec * PING_INTERVAL;

	m_AccumulatedTimerTicks	+= deltaTimeInfo.DeltaTicks;
	if ( !m_PingSent && (m_AccumulatedTimerTicks > ticksToAccumulate) )
	{
		g_NetworkEngine.PushInternalPacket( tNew( PingMessage ), m_TargetNetworkID );
		m_PingSent = true;
		m_AccumulatedTimerTicks -= ticksToAccumulate;
		m_PingSendTimeStamp = m_Timer.GetTimeAsDouble();	
	}
}

double Pinger::HandlePingReceive( )
{
	m_PingSent = false;
	m_LastPingTime = ( m_Timer.GetTimeAsDouble( ) - m_PingSendTimeStamp ) * 1000.0;
	return m_LastPingTime;
}

short Pinger::GetTargetNetworkID( ) const
{
	return m_TargetNetworkID;
}

double Pinger::GetPingTime( ) const
{
	return m_LastPingTime;
}