/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "NetworkEngine.h"
#include <utility/PlatformDefinitions.h>
#if PLATFORM == PLATFORM_WINDOWS
#include <utility/RetardedWindowsIncludes.h>
#pragma comment( lib, "Ws2_32.lib" )
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif
#include <utility/ThreadingUtility.h>
#include <utility/NeatFunctions.h>
#include "PacketPump.h"
#include "network/NetworkMessages.h"
#include "network/NetworkInfo.h"

#define DISCONNECT_ALL -1

using namespace NetworkMessageTypes;

NetworkEngine& NetworkEngine::GetInstance()
{
	static NetworkEngine instance;
	return instance;
}

NetworkEngine::NetworkEngine()
{
	m_IsRunning = false;
}

NetworkEngine::~NetworkEngine()
{
	// Verified sockets
	for ( auto& socket : m_Sockets )
		pDelete( socket.second );

	// Unverified sockets
	for ( int i = 0; i < m_UnverifiedSockets.size( ); ++i )
		pDelete( m_UnverifiedSockets[i] );
	m_UnverifiedSockets.clear( );

	// Unhandled connection attempts
	IPv4Address* destination;
	while ( m_ConnectionRequests.Consume( destination ) )
		tDelete( destination );

	Message* message;
	while ( m_GameUserPackets.Consume( message ) )
		tDelete( message );
	while ( m_GameSimulationPackets.Consume( message ) )
		tDelete( message );
}

bool NetworkEngine::Initialize()
{
	if ( !m_IsInitialized )
	{
#if PLATFORM == PLATFORM_WINDOWS // Windows needs to start WSA
		WSADATA wsaData;
		m_IsInitialized = WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) == NO_ERROR; // Initialize WSA version 2.2
		if ( !m_IsInitialized )
			LogErrorMessage( "Failed to start engine because WSAStartup failed", "NetworkEngine" );

#elif PLATFORM == PLATFORM_LINUX
		m_IsInitialized = true;
#else
#if PLATFORM == PLATFORM_MAC
		Logger::Log("No built in Mac support. Things will probably crash.", "NetworkEngine", LogSeverity::WARNING_MSG);
#else
		Logger::Log("Platform is unknown. Things will probably crash.", "NetworkEngine", LogSeverity::WARNING_MSG);
#endif
		m_IsInitialized = true;
#endif
	}
	return m_IsInitialized;
}

void NetworkEngine::Terminate()
{
	if ( m_IsInitialized )
	{
		if ( m_IsRunning )
		{
			Logger::Log( "Engine was still running when terminate was called. Engine is stopping now", "NetworkEngine", LogSeverity::INFO_MSG );
			Stop( );
		}
#if PLATFORM == PLATFORM_WINDOWS
		WSACleanup( );
#endif
		Logger::Log( "NetworkEngine has been terminated", "NetworkEngine", LogSeverity::INFO_MSG );
	}
	else
		Logger::Log( "Cannot terminate without first initializing", "NetworkEngine", LogSeverity::WARNING_MSG );
}

void NetworkEngine::Start()
{
	if ( m_IsInitialized )
	{
		if ( !m_IsRunning )
		{
			m_IsRunning = true;
			m_UpdateThread = std::thread( &NetworkEngine::Update, this );
			m_PingTimer.Start();
			m_AccumulatedTimerTicks = 0;
			Logger::Log( "NetworkEngine has started and is ready to communicate", "NetworkEngine", LogSeverity::INFO_MSG );
		}
		else
			Logger::Log( "Cannot start - Already running", "NetworkEngine", LogSeverity::WARNING_MSG );
	}
	else
		Logger::Log( "Cannot start - Not initialized", "NetworkEngine", LogSeverity::WARNING_MSG );
}

void NetworkEngine::Stop()
{
	if ( m_IsInitialized && m_IsRunning )
	{
		m_PingTimer.Stop();
		m_ReservedSlots.clear();

		// Terminate to update thread
		m_IsRunning = false;
		ThreadingUtility::JoinThread( m_UpdateThread );

		// Stop listener
		if ( m_Listener.IsListening( ) )
			m_Listener.StopListening( );

		// Disconnect clients
		Disconnect( DISCONNECT_ALL );

		// Clean the game message queues
		Message* message;
		while ( m_GameUserPackets.Consume( message ) )
			tDelete( message );
		while ( m_GameSimulationPackets.Consume( message ) )
			tDelete( message );

		Logger::Log( "NetworkEngine has stopped", "NetworkEngine", LogSeverity::INFO_MSG );
	}
	else
		Logger::Log( "Attempted to stop non running engine", "NetworkEngine", LogSeverity::WARNING_MSG );
}

void NetworkEngine::RequestConnection( const rString& adress, const rString& port )
{
	unsigned short convertedPort = static_cast< unsigned short >(std::atoi( port.c_str( ) ));
	return RequestConnection( adress, convertedPort );
}

void NetworkEngine::RequestConnection( const rString& adress, const unsigned short port )
{
	if ( m_IsInitialized )
		if ( m_IsRunning )
			m_ConnectionRequests.Produce( tNew (IPv4Address, adress, port ) ); // TODODB: Check against duplicates
		else
			Logger::Log( "Attempt to request a connection without first starting the engine", "NetworkEngine", LogSeverity::WARNING_MSG );
	else
		Logger::Log( "Attempted to request a connection without first initializing the engine", "NetworkEngine", LogSeverity::WARNING_MSG );
}

void NetworkEngine::RequestDisconnection( const short IDToDisconnect )
{
	if ( m_IsInitialized )
		if ( m_IsRunning )
			m_DisconnectionRequests.Produce( IDToDisconnect ); // TODODB: Check against duplicates
		else
			Logger::Log( "Attempt to request a disconnection without first starting the engine", "NetworkEngine", LogSeverity::WARNING_MSG );
	else
		Logger::Log( "Attempted to request a disconnection without first initializing the engine", "NetworkEngine", LogSeverity::WARNING_MSG );
}

bool NetworkEngine::StartListening( const rString& port )
{
	unsigned short convertedPort = static_cast<unsigned short>( std::atoi( port.c_str() ) );
	return StartListening( convertedPort );
}

bool NetworkEngine::StartListening( const unsigned short port )
{
	if ( m_IsInitialized )
	{
		if ( m_IsRunning )
		{
			if ( !m_Listener.IsListening() )
			{
				if ( m_Listener.StartListening( port ) )
					return true;
			}
			else
				Logger::Log( "Attempted to start listener while it was already listening", "NetworkEngine", LogSeverity::WARNING_MSG );
		}
		else
			Logger::Log( "Attempted to start listener without first starting the engine", "NetworkEngine", LogSeverity::WARNING_MSG );
	}
	else
		Logger::Log( "Attempted to start listener without first initializing", "NetworkEngine", LogSeverity::WARNING_MSG );

	return false;
}

void NetworkEngine::StopListening()
{
	m_Listener.StopListening();
}

int NetworkEngine::RegisterCallback( NetworkCallbackEvent callbackEvent, const std::function<void( const void* const )> callbackFunction )
{
	m_Callbacks[static_cast< int >( callbackEvent )].emplace( m_NextCallbackID, callbackFunction );

	++m_NextCallbackID;
	return m_NextCallbackID - 1;
}

bool NetworkEngine::UnRegisterCallback( int callbackHandle )
{
	for ( int i = 0; i < static_cast< int >( NetworkCallbackEvent::NUMBER_OF_NETWORK_CALLBACK_EVENTS ); ++i )
	{
		if ( m_Callbacks[i].find( callbackHandle ) != m_Callbacks[i].end() )
			return m_Callbacks[i].erase( callbackHandle ) == 1;
	}

	return false;
}

bool NetworkEngine::ReserveSlot( short index )
{
	bool toReturn = true;

	m_ReservedSlotsMutex.lock();
	for ( int i = 0; i < m_ReservedSlots.size(); ++i )
		if ( m_ReservedSlots[i] == index )
			toReturn = false;
	if (toReturn)
		m_ReservedSlots.push_back( index );
	m_ReservedSlotsMutex.unlock();

	return toReturn;
}

bool NetworkEngine::UnreserveSlot( short index )
{
	bool toReturn = false;

	m_ReservedSlotsMutex.lock();
	for ( int i = 0; i < m_ReservedSlots.size(); ++i )
		if ( m_ReservedSlots[i] == index )
		{
			m_ReservedSlots.erase( m_ReservedSlots.begin() + i );
			toReturn = true;
			break;
		}
	m_ReservedSlotsMutex.unlock();

	return toReturn;
}

bool NetworkEngine::GetSimulationPacket( Message*& outPacket )
{
	return m_GameSimulationPackets.Consume( outPacket );
}

bool NetworkEngine::GetUserPacket( Message*& outPacket )
{
	return m_GameUserPackets.Consume( outPacket );
}

void NetworkEngine::SetMaxClientCount( short maxClients )
{
	m_MaxClients = maxClients;
}

bool NetworkEngine::IsInitialized() const
{
	return m_IsInitialized;
}

bool NetworkEngine::IsRunning() const
{
	return m_IsRunning;
}

bool NetworkEngine::IsListening() const
{
	return m_Listener.IsListening();
}

void NetworkEngine::PushInternalPacket( const Message* const packet, const short networkID )
{
	m_InternalPacketQueue.push( std::pair<const Message* const, const short>( packet, networkID ) );
}

void NetworkEngine::Update()
{
	Logger::Log( "Network thread is starting ", "NetworkEngine", LogSeverity::INFO_MSG );
	while ( m_IsRunning )
	{
		CheckListener();				// Check for incoming connection attempts
		HandleConnectionRequests();		// Handle outgoing connection attempts
		HandleDisconnectionRequests();	// Handle game requests to disconnect clients
		HandleUnverifiedSockets();		// Perform handshakes and determine if the remote client should be connected to the game
		HandleTraffic();				// Handle network traffic from verified sockets

		if ( g_NetworkInfo.AmIHost() )
			MeasureLatency( );			// Only the host measures latency.
	}
	Logger::Log( "Network thread is terminating gracefully", "NetworkEngine", LogSeverity::INFO_MSG );
}

void NetworkEngine::CheckListener()
{
	TCPSocket* newConnection;
	while ( m_Listener.GetConnectionQueue( ).Consume( newConnection ) )
	{
		m_UnverifiedSockets.push_back( newConnection );

		Logger::Log( "Attempting to shake hand with newly accepted connection: " + newConnection->GetDestination( ).GetPrintableAdress( ), "NetworkEngine", LogSeverity::INFO_MSG );
		HandshakeMessage handShakePacket;
		newConnection->SendPacket( handShakePacket );
	}
}

void NetworkEngine::HandleConnectionRequests()
{
	IPv4Address* destination;
	while ( m_ConnectionRequests.Consume( destination ) ) // Get a connection request from the queue as long as it isn't empty
	{
		// Check against duplicates
		for ( auto& socket : m_Sockets )
		{
			if ( *destination == socket.second->GetDestination( ) )
			{
				Logger::Log( "Connection request to " + destination->GetPrintableAddressAndPort( ) + " was ignored since a connection to that destination already exists.", "NetworkEngine", LogSeverity::WARNING_MSG );
				ConnectionData connectionData( -1, *destination );
				CallCallbacks( NetworkCallbackEvent::ConnectionAttemptFail, &connectionData);
				tDelete( destination );
				return;
			}
		}

		// Check if there is room for another client and connect
		short socketSlot = FindEmptySocketSlot( );
		if ( socketSlot >= 0 )
		{
			TCPSocket* socket = pNew( TCPSocket );
			bool result = socket->Connect( *destination );	// If the attempt is successfull the socket saves a copy of the inputed IPv4Adress
			if ( result )
			{
				m_UnverifiedSockets.push_back( socket );	// If the connection was OK we now need to perform handshake
			}
			else
			{
				pDelete( socket );
				ConnectionData connectionData( -1, *destination );
				CallCallbacks( NetworkCallbackEvent::ConnectionAttemptFail, &connectionData);
			}
		}
		else
		{
			Logger::Log( "Attempted to connect to another client after maximum client count had been reached", "NetworkEngine", LogSeverity::WARNING_MSG );
			ConnectionData connectionData( -1, *destination );
			CallCallbacks( NetworkCallbackEvent::ConnectionAttemptFail, &connectionData);
		}
		tDelete( destination );
	}
}

void NetworkEngine::HandleDisconnectionRequests()
{
	// Handle external disconnection requests (From game)
	short IDToDisconnect;
	while ( m_DisconnectionRequests.Consume( IDToDisconnect ) )
	{
		Disconnect( IDToDisconnect );
		if ( m_PingTargets.find( IDToDisconnect ) != m_PingTargets.end() ) // If the sockets destination was a pingtarget, remove it.
			m_PingTargets.erase( IDToDisconnect );
	}

	// Handle internal disconnection requests (From TCPSocket)
	short networkIDToDisconnect;
	do 
	{
		networkIDToDisconnect = INVALID_NETWORK_ID;

		// Verified sockets
		for ( auto& socket : m_Sockets ) // Find socket with the conncted flag set to false
		{
			if ( !socket.second->IsConnected() )
			{
				networkIDToDisconnect = socket.first;
				break;
			}
		}

		// Unverified sockets
		if ( networkIDToDisconnect == INVALID_NETWORK_ID )
		{
			for ( int i = 0; i < m_UnverifiedSockets.size(); ++i )
			{
				if ( !m_UnverifiedSockets[i]->IsConnected() )
				{
					TCPSocket* toDisconnect = m_UnverifiedSockets[i];
					toDisconnect->Disconnect();
					pDelete( toDisconnect );
					m_UnverifiedSockets.erase( m_UnverifiedSockets.begin() + i );
					--i;
					break;
				}
			}
		}

		// Disconnect found socket
		if ( networkIDToDisconnect != INVALID_NETWORK_ID )
		{
			Disconnect( networkIDToDisconnect );
			if ( m_PingTargets.find( networkIDToDisconnect ) != m_PingTargets.end() ) // If the sockets destination was a pingtarget, remove it.
				m_PingTargets.erase( networkIDToDisconnect );
		}
	}
	while ( networkIDToDisconnect != INVALID_NETWORK_ID );
}

void NetworkEngine::MeasureLatency()
{
	if ( m_Sockets.size() > 0 )
	{
		for ( auto& pingTarget : m_PingTargets )
		{
			pingTarget.second.HandlePingSend();
		}

		m_PingTimer.Tick();
		GameTimer::DeltaTimeInfo	deltaTimeInfo		= m_PingTimer.GetDeltaTimeInfo();
		static Uint64				ticksToAccumulate	= deltaTimeInfo.TicksPerSec * PING_INTERVAL;

		m_AccumulatedTimerTicks	+= deltaTimeInfo.DeltaTicks;
		if ( m_AccumulatedTimerTicks > ticksToAccumulate )
		{
			rMap <short, double> latencies;
			for ( auto& pingTargets : m_PingTargets )
				latencies.emplace( pingTargets.first, pingTargets.second.GetPingTime() );
			LatencyUpdateMessage latencyUpdateMessage = LatencyUpdateMessage( latencies );

			for ( auto& sockets : m_Sockets )
				sockets.second->SendPacket( latencyUpdateMessage );

			m_AccumulatedTimerTicks -= ticksToAccumulate;
		}
	}
}

void NetworkEngine::HandleUnverifiedSockets()
{
	for ( int i = 0; i < m_UnverifiedSockets.size(); ++i )
	{
		bool socketDone = false;
		Message* packet;
		while ( socketDone == false && ( packet = m_UnverifiedSockets[i]->Receive() ) != nullptr )  // Handle clients buffered traffic
		{
			if ( packet->Type == 0 )
			{
				NetworkMessage* networkMessage = static_cast< NetworkMessage* >( packet );
				switch ( networkMessage->NetworkMessageType )
				{
					case BAD: // Uninitialized packet
					{
						Logger::Log( "Received BAD packet from unverified client", "NetworkEngine", LogSeverity::WARNING_MSG );
					} break;

					case HANDSHAKE:
					{
						if ( g_NetworkInfo.AmIHost() )
						{
							short networkID = FindEmptySocketSlot(); // Find an open slot for the client
							if ( networkID >= 0 )
							{
								// Send the client the ID so it knows who it is and that it has been verified. Also send all known connections to brign the new client up to date
								rVector<short> connections;
								NeatFunctions::MapKeysToVector( m_Sockets, connections );
								NetworkIDMessage IDpacket = NetworkIDMessage( networkID, connections );
								m_UnverifiedSockets[i]->SendPacket( IDpacket );

								// Tell the other clients that there is a new player
								for ( auto& socketPair : m_Sockets )
									socketPair.second->SendPacket( ConnectionStatusMessage( networkID, ConnectionStatusChanges::CONNECTED ) );

								g_PacketPump.AddQueue( networkID );	// Add a queue to the Sender interface so that packets can be sent to this client from the game
								g_NetworkInfo.RegisterConnection( ConnectionInfo( networkID, ConnectionSlotTypes::PLAYER, &m_UnverifiedSockets[i]->GetDestination() ) ); // Register the connection in the Info interface so the game knows that this connection exists

								// Move the client to the map with verified sockets
								m_Sockets.emplace( std::pair<short, TCPSocket*>( networkID, m_UnverifiedSockets[i] ) );
								Logger::Log( m_UnverifiedSockets[i]->GetDestination().GetPrintableAdress() + " has been assigned the ID " + rToString( networkID ) + " and is now ready to communicate", "NetworkEngine", LogSeverity::INFO_MSG );
								m_UnverifiedSockets.erase( m_UnverifiedSockets.begin() + i );

								// Start pinging the verified client
								m_PingTargets.emplace( std::pair<short, Pinger>( networkID, Pinger( networkID ) ) );

								ConnectionData connectionData( networkID, m_Sockets.at( networkID )->GetDestination() );
								CallCallbacks( NetworkCallbackEvent::IncomingConnection, &connectionData );

								i--;
								socketDone = true; // Break the while loop. We are done reading from this client
							}
							else
							{
								Logger::Log( "Could not verify incoming connection since there was no connection slot open", "NetworkEngine", LogSeverity::WARNING_MSG );
								m_UnverifiedSockets[i]->Disconnect();
								pDelete( m_UnverifiedSockets[i] );
								m_UnverifiedSockets.erase( m_UnverifiedSockets.begin() + i );
								socketDone = true;
							}
						}
						else
						{
							// Respond to the handshake
							Logger::Log( "Responded to handshake from " + m_UnverifiedSockets[i]->GetDestination().GetPrintableAdress(), "NetworkEngine", LogSeverity::INFO_MSG );
							networkMessage->SenderID = g_NetworkInfo.GetNetworkID();

							m_UnverifiedSockets[i]->SendPacket( *networkMessage );
						}
					} break;

					case NETWORKIDMESSAGE:
					{
						NetworkIDMessage* IDpacket = static_cast<NetworkIDMessage*>( networkMessage );

						// Set the NetworkID and the host ID so we know who we and the host are
						g_NetworkInfo.SetNetworkID( IDpacket->NetworkID );
						g_NetworkInfo.SetHostID( IDpacket->SenderID );

						// Expose the connection to the game logic
						g_NetworkInfo.RegisterConnection( ConnectionInfo( IDpacket->SenderID, ConnectionSlotTypes::PLAYER, &m_UnverifiedSockets[i]->GetDestination() ) );
						g_PacketPump.AddQueue( IDpacket->SenderID );

						// Read the connection the host says it has to other client
						for ( int j = 0; j < IDpacket->ConnectedClients.size(); ++j )
							g_NetworkInfo.RegisterConnection( ConnectionInfo( IDpacket->ConnectedClients[j], ConnectionSlotTypes::PLAYER ) );

						m_Sockets.emplace( std::pair<short, TCPSocket*>( IDpacket->SenderID, m_UnverifiedSockets[i] ) );
						m_UnverifiedSockets.erase( m_UnverifiedSockets.begin() + i );

						ConnectionData connectionData( IDpacket->NetworkID, m_Sockets.at( IDpacket->SenderID )->GetDestination() );
						CallCallbacks( NetworkCallbackEvent::ConnectionAttemptSuccess, &connectionData);

						i--;
						socketDone = true;
					} break;

					default:
					{
						Logger::Log( "Received packet with unknown Type from unverified client", "NetworkEngine", LogSeverity::WARNING_MSG );
					}
					break;
				}
			}
			else
				Logger::Log( "Received non handshake packet from unverified socket", "NetworkEngine", LogSeverity::WARNING_MSG );
			tFree( packet );
		}
	}
}

void NetworkEngine::HandleTraffic()
{
	// Handle outgoing traffic coming from within the network engine
	while ( !m_InternalPacketQueue.empty() )
	{
		short targetNetworkID = m_InternalPacketQueue.front().second;
		const Message* packet = m_InternalPacketQueue.front().first;

		m_Sockets.at( targetNetworkID )->SendPacket( *packet );

		tDelete( packet );
		m_InternalPacketQueue.pop();
	}
	
	for ( auto& it : m_Sockets )
	{
		// Handle outgoing traffic coming from outside of the network engine
		{
			const Message* packet;
			while ( ( packet = g_PacketPump.GetPacketFromQueue( it.first ) ) != nullptr )
			{
				it.second->SendPacket( *packet );
				tDelete( packet );
			}
		}

		// Handle Incoming Traffic
		Message* packet;
		while ( (packet = it.second->Receive()) != nullptr )
		{
			bool messageForwarded = false; // TODODB This variable can be removed by refactoring into the forwarding
			if ( packet->Type == 0 )
			{
				NetworkMessage* networkMessage = static_cast<NetworkMessage*>( packet );
				switch ( networkMessage->NetworkMessageType )
				{
					case BAD:
					{
						Logger::Log( "Received packet with Type BAD", "NetworkEngine", LogSeverity::WARNING_MSG );
					} break;

					case PING:
					{
						if ( !g_NetworkInfo.AmIHost() )
						{
							// Change sender and return the packet
							networkMessage->SenderID = g_NetworkInfo.GetNetworkID();
							it.second->SendPacket( *networkMessage );
						}
						else
						{
							m_PingTargets.at( networkMessage->SenderID ).HandlePingReceive(); // Take Measurement
							g_NetworkInfo.SetClientLatency( networkMessage->SenderID, m_PingTargets.at( networkMessage->SenderID ).GetPingTime() ); // Update measurement in NetworkInfo
						}
					} break;

					case LATENCY_UPDATE_MESSAGE:
					{
						LatencyUpdateMessage* latencyUpdateMessage = static_cast<LatencyUpdateMessage*>( networkMessage );

						short networkID;
						for ( auto& pingPair : latencyUpdateMessage->Latencies )
						{
							pingPair.first == g_NetworkInfo.GetNetworkID() ? networkID = g_NetworkInfo.GetHostID() :  networkID = pingPair.first;
							g_NetworkInfo.SetClientLatency( networkID, pingPair.second );
						}
					} break;

					case HANDSHAKE:
					{
						Logger::Log( "Responded to handshake from client " + rToString( it.first ), "NetworkEngine", LogSeverity::INFO_MSG );
						it.second->SendPacket( *networkMessage );
					} break;

					case NETWORKIDMESSAGE:
					{
						Logger::Log( "Received NetworkID after verification", "NetworkEngine", LogSeverity::WARNING_MSG );
					} break;

					case CONNECTION_STATUS_MESSAGE:
					{
						ConnectionStatusMessage* connectionStatusPacket = static_cast<ConnectionStatusMessage*>( networkMessage );
						if ( !g_NetworkInfo.AmIHost() )
						{
							if ( connectionStatusPacket->ConnectionStatus == ConnectionStatusChanges::CONNECTED )
							{
								g_NetworkInfo.RegisterConnection( ConnectionInfo( connectionStatusPacket->NetworkID, ConnectionSlotTypes::PLAYER ) );
								ConnectionData connectionData( connectionStatusPacket->NetworkID, IPv4Address() );
								CallCallbacks( NetworkCallbackEvent::IncomingConnection, &connectionData); // TODODB: Handle propagation of connection info outside of the network engine
							}
							else if ( connectionStatusPacket->ConnectionStatus == ConnectionStatusChanges::DISCONNECTED )
							{
								ConnectionData connectionData(connectionStatusPacket->NetworkID, IPv4Address() );
								CallCallbacks( NetworkCallbackEvent::Disconnection, &connectionData ); // TODODB: Handle propagation of connection info outside of the network engine
							}
							else
								Logger::Log( "Received connection status message with unknown connection status", "NetworkEngine", LogSeverity::WARNING_MSG );
						}
						else
							Logger::Log( "Received connection status packet as host", "NetworkEngine", LogSeverity::WARNING_MSG );
					} break;


					default:
					{
						
					} break;
				}
			}
			else
			{
				packet->IsSimulation ? m_GameSimulationPackets.Produce( packet ) : m_GameUserPackets.Produce( packet );
				messageForwarded = true;
			}
			if ( !messageForwarded )
				tDelete( packet );
		}
	}
}

short NetworkEngine::FindEmptySocketSlot()
{
	for ( int i = 0; i < m_MaxClients + 1; ++i ) // +1 for the local client
	{
		if ( g_NetworkInfo.GetNetworkID() == i ) // You found yourself *slow clap*
			continue;

		m_ReservedSlotsMutex.lock();
		bool shouldContinue = false;
		for ( int j = 0; j < m_ReservedSlots.size(); ++j )
			if ( m_ReservedSlots[j] == i )
			{
				shouldContinue = true;
				break;
			}
		m_ReservedSlotsMutex.unlock();

		if ( shouldContinue )
			continue;

		if ( m_Sockets.find( i ) == m_Sockets.end( ) )
			return i;
	}
	return -1; // There was no empty slot
}

void NetworkEngine::Disconnect( const short networkIDToDisconnect )
{
	if ( networkIDToDisconnect != DISCONNECT_ALL )
	{
		// Find the correct socket and disconnect it
		if ( m_Sockets.find( networkIDToDisconnect ) != m_Sockets.end() )
		{
			TCPSocket* toDisconnect = m_Sockets.at( networkIDToDisconnect );
			toDisconnect->Disconnect();

			// Unregister the socket from the interfacees so that the game won't see it as connected
			g_NetworkInfo.UnRegisterConnection( networkIDToDisconnect );
			g_PacketPump.RemoveQueue( networkIDToDisconnect );
			Logger::Log( "Client on " + toDisconnect->GetDestination().GetPrintableAddressAndPort( ) + " has been disconnected", "NetworkEngine", LogSeverity::INFO_MSG );

			ConnectionData connectionData(networkIDToDisconnect, toDisconnect->GetDestination() );
			CallCallbacks( NetworkCallbackEvent::Disconnection, &connectionData );

			pDelete( toDisconnect );
			m_Sockets.erase( networkIDToDisconnect );
		}
		else
			Logger::Log( "Attempted to disconnect nonexistant socket (ID = " + rToString( networkIDToDisconnect ) + " )", "NetworkEngine", LogSeverity::WARNING_MSG );
	}
	else // Disconnect all sockets
	{
		for ( auto& socket : m_Sockets )
		{
			socket.second->Disconnect();

			ConnectionData connectionData(networkIDToDisconnect, socket.second->GetDestination() );
			CallCallbacks( NetworkCallbackEvent::Disconnection, &connectionData );

			g_NetworkInfo.UnRegisterConnection( socket.first );
			g_PacketPump.RemoveQueue( socket.first );
			Logger::Log( "Client on " + socket.second->GetDestination().GetPrintableAddressAndPort() + " has been disconnected", "NetworkEngine", LogSeverity::INFO_MSG );

			pDelete( socket.second );
		}
		m_Sockets.clear( );
	}
}

void NetworkEngine::CallCallbacks( NetworkCallbackEvent callbackEvent, const void* const arg ) const
{
	assert( static_cast<int>(callbackEvent) < static_cast<int>(NetworkCallbackEvent::NUMBER_OF_NETWORK_CALLBACK_EVENTS ) );

	for ( auto callbackFunction : m_Callbacks[static_cast<int>( callbackEvent )] )
		callbackFunction.second( arg );
}
