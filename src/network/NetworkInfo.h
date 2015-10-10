/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include "NetworkUtility.h"
#include "IPv4Adress.h"
#include <atomic>
#include <mutex>
#include <memory/Alloc.h>
#include <utility/LocklessQueue.h>
#include <limits.h>
#include "NetworkLibraryDefine.h"

#define g_NetworkInfo NetworkInfo::GetInstance()

struct	ConnectionInfo;

namespace ConnectionSlotTypes
{
	enum ConnectionSlotType
	{
		PLAYER,
		SPECTATOR,
	};
}

class NetworkInfo
{
public:
	NETWORK_API static NetworkInfo& GetInstance( ) { static NetworkInfo networkInfo; return networkInfo; };

	NETWORK_API void								Reset					();

	NETWORK_API bool								AmIHost					() const;
	NETWORK_API bool								IsClientHost			( short ID ) const;

	NETWORK_API short								GetNetworkID			( ) const;
	NETWORK_API short								GetHostID				( ) const;
	NETWORK_API unsigned int						GetHostStep				( ) const;
	NETWORK_API const rMap<short, ConnectionInfo>	GetConnections			( );
	NETWORK_API rString								GetHostIPString			( );
	NETWORK_API short								GetConnectedPlayerCount	( );
	NETWORK_API double								GetClientLatency		( const short networkID );
	NETWORK_API	rString								GetClientIPString		( const short networkID );

	NETWORK_API void								SetNetworkID			( const short newNetworkID );
	NETWORK_API void								SetHostID				( const short newHostID );
	NETWORK_API	void								SetClientLatency		( const short clientNetworkID, const double latency );
	NETWORK_API	void								SetHostStep				( const unsigned int latestHostStep );

	NETWORK_API	void								PushHostHash			( const unsigned int hash );
	NETWORK_API	unsigned int						PopHostHash				();
	NETWORK_API	void								PushHostRandomCount		( const unsigned int randomCount );
	NETWORK_API	unsigned int						PopHostRandomCount		();
	
	NETWORK_API bool								IsConnectedTo			( const rString& address, const rString& port );
	NETWORK_API bool								IsConnectedTo			( const rString& address, const unsigned short port );
	NETWORK_API bool								IsConnectedToPlayerCount( const short playerCount );

				void								RegisterConnection		( const ConnectionInfo& connection );
				void								UnRegisterConnection	( const short NetworkID );

	NETWORK_API	void								IncrementFramesToRun();
	NETWORK_API bool								DecrementFramesToRun	();

private:
	// No external instancing allowed
	NetworkInfo( );
	NetworkInfo( const NetworkInfo& rhs );
	~NetworkInfo( );
	NetworkInfo operator =(const NetworkInfo& rhs);

	bool										IsConnectedTo			( const IPv4Address& destination );

	short										m_NetworkID;
	short										m_HostID;

	rMap<short, ConnectionInfo>					m_Connections;
	LocklessQueue<unsigned int>					m_HostHashQueue;
	LocklessQueue<unsigned int>					m_HostRandomCountsQueue;
	
	std::atomic_uint							m_FramesToRun;
	std::atomic_uint							m_LatestHostStep;
	std::mutex									m_ConnectionsLock;
};

struct ConnectionInfo
{
	ConnectionInfo( short networkID, ConnectionSlotTypes::ConnectionSlotType type, const IPv4Address* destination = nullptr )
		: NetworkID( networkID ), Type( type ), Destination( destination ) {}

	short										NetworkID;
	double										Latency = 0.0;
	ConnectionSlotTypes::ConnectionSlotType		Type;
	const IPv4Address*							Destination; // Try to make this a copy instead

	bool operator==( const ConnectionInfo& rhs ) const { return this->NetworkID == rhs.NetworkID; }
	bool operator!=( const ConnectionInfo& rhs ) const { return this->NetworkID != rhs.NetworkID; }
};