/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "NetworkInfo.h"

#define DEFAULT_NETWORK_ID -1
#define DEFAULT_HOST_ID -2

NetworkInfo::NetworkInfo()
{
	m_NetworkID				= DEFAULT_NETWORK_ID;
	m_HostID				= DEFAULT_HOST_ID;
	m_FramesToRun			= 0;
	m_LatestHostStep		= 0;
}

NetworkInfo::~NetworkInfo()
{}

void NetworkInfo::Reset()
{
	m_NetworkID = DEFAULT_NETWORK_ID;
	m_HostID	= DEFAULT_HOST_ID;

	m_HostHashQueue.Clear();
	m_HostRandomCountsQueue.Clear();

	m_FramesToRun = 0;
	m_LatestHostStep = 0;
}

bool NetworkInfo::AmIHost() const
{
	return m_NetworkID == m_HostID;
}

bool NetworkInfo::IsClientHost( short ID ) const
{
	return ID == m_HostID;
}

short NetworkInfo::GetNetworkID() const
{
	return m_NetworkID;
}

short NetworkInfo::GetHostID() const
{
	return m_HostID;
}

unsigned int NetworkInfo::GetHostStep() const
{
	return m_LatestHostStep;
}


const rMap<short, ConnectionInfo> NetworkInfo::GetConnections()
{
	m_ConnectionsLock.lock();
		rMap<short, ConnectionInfo> toReturn = m_Connections;
	m_ConnectionsLock.unlock();

	return toReturn;
}

rString NetworkInfo::GetHostIPString( )
{
	return AmIHost() ? "LocalHost" : GetClientIPString( m_HostID );
}

short NetworkInfo::GetConnectedPlayerCount()
{
	m_ConnectionsLock.lock();
	short conectedPlayerCount = static_cast<short>( m_Connections.size( ) );
	m_ConnectionsLock.unlock();

	return conectedPlayerCount;
}

double NetworkInfo::GetClientLatency( const short networkID )
{
	double toReturn = 0.0;
	m_ConnectionsLock.lock();
	auto connectionPair = m_Connections.find( networkID );
	if ( connectionPair != m_Connections.end() )
	{
		toReturn = m_Connections.at( networkID ).Latency ;
	}
	m_ConnectionsLock.unlock();

	return toReturn;
}

rString NetworkInfo::GetClientIPString( const short networkID )
{
	rString toReturn;
	m_ConnectionsLock.lock();
	auto connectionPair = m_Connections.find( networkID );
	if ( connectionPair != m_Connections.end() )
	{
		const IPv4Address* destination = m_Connections.at( networkID ).Destination;
		if ( destination != nullptr )
		{
			toReturn = destination->GetPrintableAdress();
		}
	}
	m_ConnectionsLock.unlock();

	return toReturn;
}

void NetworkInfo::SetNetworkID( const short newNetworkID )
{
	m_NetworkID = newNetworkID;
}

void NetworkInfo::SetHostID( const short newHostID )
{
	m_HostID = newHostID;
}

void NetworkInfo::SetHostStep( const unsigned int latestHostStep )
{
	m_LatestHostStep = latestHostStep;
}

void NetworkInfo::SetClientLatency( const short networkID, const double latency )
{
	m_ConnectionsLock.lock( );
	m_Connections.at( networkID ).Latency = latency;
	m_ConnectionsLock.unlock();
}

void NetworkInfo::PushHostHash( const unsigned int hash )
{
	m_HostHashQueue.Produce( hash );
}

unsigned int NetworkInfo::PopHostHash()
{
	unsigned int toReturn;
	if ( !m_HostHashQueue.Consume( toReturn ) )
	{
		Logger::Log( "Attempted to pop hash from host but there were none left. Are we out of sync?", "NetworkInfo", LogSeverity::WARNING_MSG );
		return 0;
	}
	return toReturn;
}

void NetworkInfo::PushHostRandomCount( const unsigned int randomCount )
{
	m_HostRandomCountsQueue.Produce( randomCount );
}

unsigned int NetworkInfo::PopHostRandomCount()
{
	unsigned int toReturn;
	if ( !m_HostRandomCountsQueue.Consume( toReturn ) )
	{
		Logger::Log( "Attempted to pop randomCount from host but there were none left. Are we out of sync?", "NetworkInfo", LogSeverity::WARNING_MSG );
		return 0;
	}
	return toReturn;
}

bool NetworkInfo::IsConnectedTo( const rString& address, const rString& port )
{
	return IsConnectedTo( IPv4Address( address, port ));
}

bool NetworkInfo::IsConnectedTo( const rString& address, const unsigned short port )
{
	return IsConnectedTo( IPv4Address( address, port ) );
}

bool NetworkInfo::IsConnectedTo( const IPv4Address& destination )
{
	bool result = false;
	m_ConnectionsLock.lock();
	for ( auto& connectionPair : m_Connections )
	{
		if ( connectionPair.second.Destination->GetPrintableAdress() == destination.GetPrintableAdress() ) // Ports cannot be compared here since internal ports are used for received connections
		{
			result = true;
			break;
		}
	}
	m_ConnectionsLock.unlock();
	return result;
}

void NetworkInfo::RegisterConnection( const ConnectionInfo& newConnection )
{
	m_ConnectionsLock.lock();
	for ( auto& connection : m_Connections)
	{
		if ( newConnection == connection.second )
		{
			Logger::Log( "Attempted to register already registered connection", "NetworkInfo", LogSeverity::WARNING_MSG );
			m_ConnectionsLock.unlock();
			return;
		}
	}

	m_Connections.emplace( newConnection.NetworkID, newConnection );
	m_ConnectionsLock.unlock();
}

bool NetworkInfo::IsConnectedToPlayerCount( const short playerCount )
{
	m_ConnectionsLock.lock( );
	bool result = m_Connections.size( ) >= playerCount;
	m_ConnectionsLock.unlock();

	return result;
}

void NetworkInfo::UnRegisterConnection( const short networkID )
{
	m_ConnectionsLock.lock();

	auto connectionInfo = m_Connections.find( networkID );
	if ( connectionInfo != m_Connections.end() )
	{
		m_Connections.erase( connectionInfo );
	}
	else
		Logger::Log( "Attempted to unregister a non registered connection", "NetworkInfo", LogSeverity::WARNING_MSG );

	m_ConnectionsLock.unlock();
}

void NetworkInfo::IncrementFramesToRun()
{
	++m_FramesToRun;
}

bool NetworkInfo::DecrementFramesToRun()
{
	if ( m_FramesToRun > 0 )
	{
		--m_FramesToRun;
		return true;
	}
	return false;
}