/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include <utility/PlatformDefinitions.h>
#include "TCPListener.h"
#include "Pinger.h"
#include "NetworkLibraryDefine.h"
#include "NetworkMessages.h"

#define g_NetworkEngine NetworkEngine::GetInstance()

enum class NetworkCallbackEvent
{
	// Event type				Return type

	IncomingConnection,			// ConnectionData
	ConnectionAttemptSuccess,	// ConnectionData
	ConnectionAttemptFail,		// ConnectionData
	Disconnection,				// ConnectionData
	NUMBER_OF_NETWORK_CALLBACK_EVENTS	// MUST ALWAYS BE LAST AND NEVER BE REGISTERED
};

struct ConnectionData
{
	ConnectionData( short networkID, const IPv4Address& address )
	{ NetworkID = networkID; IPAddress = address; }

	short		NetworkID;
	IPv4Address IPAddress;
};

class NetworkEngine
{
public:
	static NETWORK_API NetworkEngine&		GetInstance			();
	
	// Performs platform specific initialization. Doesn't start the engine.
	NETWORK_API bool						Initialize			();

	// Performs platform specific shutdown/cleanup.
	NETWORK_API void						Terminate			();

	// Starts the engines execution thread so that it can be used.
	NETWORK_API void						Start				();

	// Stops the engines execution thread.
	NETWORK_API void						Stop				();

	// Puts a connection request into a queue that is handled by the execution thread
	NETWORK_API void						RequestConnection	( const rString& adress, const rString& port );
	NETWORK_API void						RequestConnection	( const rString& adress, const unsigned short port );

	// Puts a disconnection request into a queue that is handled by the execution thread
	NETWORK_API void						RequestDisconnection( const short networkID );

	// Opens the listetning socket on the specified port and starts its corresponding thread. This makes the system ready to receive connections
	NETWORK_API bool						StartListening		( const rString& port );
	NETWORK_API bool						StartListening		( const unsigned short port );

	// Closes the listening socket and terminates its corresponding thread.
	NETWORK_API void						StopListening		();

	// Registers a callback function that will be triggered on the inputed event
	NETWORK_API int							RegisterCallback( NetworkCallbackEvent callbackEvent, const std::function<void( const void* const )> callbackFunction );

	// Unregisters a previously registered callback function
	NETWORK_API bool						UnRegisterCallback( int callbackHandle );

	// Registers a game specific message so that the engine can identify it when it arrives
	NETWORK_API bool						RegisterMessage( const Message* gameMessage );

	// This is an ugly hack that stops the network engine from using certain networkIDs. (Required by the game lobby for AI players) 
	NETWORK_API bool						ReserveSlot( short index );
	NETWORK_API bool						UnreserveSlot( short index );

	// Gets a received game specific packet from the internal packet queue and pops it
	NETWORK_API bool						GetPacket( Message*& outPacket );

	// Sets the maximum number of allowed clients
	NETWORK_API void						SetMaxClientCount( short maxClients );

	// Returns true if the network engine is initialized
	NETWORK_API bool						IsInitialized() const;

	// Returns true if the network engine has been started
	NETWORK_API bool						IsRunning() const;

	// Returns true if the network engine has a listening socket running
	NETWORK_API bool						IsListening() const;

	// Returns true if the network engine has a registered callback function
	NETWORK_API bool						HasRegisteredCallbackFunction() const; // TODODB: Create these functions for all callbacks

	// Enqueues a packet from within the engine.
	void									PushInternalPacket	( const Message* const packet, const short networkID );

private:
	typedef std::queue<std::pair<const Message* const, const short>, rDeque<std::pair<const Message* const, const short>>> InternalPacketQueue;

	// No external instancing allowed
	NetworkEngine( );
	NetworkEngine( const NetworkEngine& rhs );
	~NetworkEngine( );
	NetworkEngine& operator=(const NetworkEngine& rhs);

	void	Update						();
	void	CheckListener				();
	void	HandleConnectionRequests	();
	void	HandleDisconnectionRequests	();
	void	HandleMessageRegistrations	();
	void	MeasureLatency				();
	void	HandleUnverifiedSockets		();
	void	HandleTraffic				();
	void	Disconnect					( const short networkIDToDisconnect );
	void	DisconnectAllUnverified		();
	short	FindEmptySocketSlot			();
	void	CallCallbacks( NetworkCallbackEvent callbackEvent, const void* const arg ) const;
	void	RegisterNetworkMessages		();
	
	rMap<int, std::function<void( const void* const )>> m_Callbacks[static_cast<int>( NetworkCallbackEvent::NUMBER_OF_NETWORK_CALLBACK_EVENTS )];
	rMap<char, NetworkMessage*> m_NetworkMessages;
	rMap<MESSAGE_TYPE_ENUM_UNDELYING_TYPE, const Message*>							m_GameMessages;
	LocklessQueue<const Message*>						m_GameMessagesToRegister;
								
	TCPListener									m_Listener;
	rMap<short, TCPSocket*>						m_Sockets;
	rVector<TCPSocket*>							m_UnverifiedSockets;
	rVector<short>								m_ReservedSlots;
	rMap<short, Pinger>							m_PingTargets;
	LocklessQueue<IPv4Address*>					m_ConnectionRequests;
	LocklessQueue<short>						m_DisconnectionRequests;
	LocklessQueue<Message*>						m_GamePackets;
	InternalPacketQueue							m_InternalPacketQueue;

	GameTimer									m_PingTimer;
	Uint64										m_AccumulatedTimerTicks;
									
	std::thread									m_UpdateThread;
	std::atomic_bool							m_IsRunning;
	std::mutex									m_ReservedSlotsMutex;
									
	bool										m_IsInitialized		= false;
	int											m_NextCallbackID	= 0;
	short										m_MaxClients		= 1;
};