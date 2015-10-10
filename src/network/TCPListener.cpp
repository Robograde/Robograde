/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "TCPListener.h"
#include <utility/Logger.h>
#include <utility/ThreadingUtility.h>
#include <utility/PlatformDefinitions.h>
#include <memory/Alloc.h>
#if PLATFORM == PLATFORM_LINUX
#include <unistd.h>
#endif
#include "NetworkUtility.h"

TCPListener::TCPListener( )
{
	m_IsRunning = false;
}

TCPListener::~TCPListener( )
{
	TCPSocket* queuedSocket;
	while ( m_ConnectionQueue.Consume( queuedSocket ) )
	{
		pDelete( queuedSocket );
	}
}

bool TCPListener::StartListening( const unsigned short port )
{
	if ( !m_IsListening )
	{
		// Create listening socket
		m_ListeningSocket = static_cast<Socket>(socket( AF_INET, SOCK_STREAM, IPPROTO_TCP )); // Will be used to listen for incoming connections
		if ( m_ListeningSocket == INVALID_SOCKET )
		{
			LogErrorMessage( "Failed to set up listening socket", "TCPListener" );
			return false;
		}

		// Allow reuse of listening socket port
		char reuse = 1;
		setsockopt( m_ListeningSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( char ) ); // TODODB: Check return value

		// Set up the sockaddr for the listenign socket
		sockaddr_in sockAddr;
		memset( &sockAddr, 0, sizeof( sockAddr ) );
		sockAddr.sin_family			= AF_INET;
		sockAddr.sin_addr.s_addr	= htonl( INADDR_ANY );
		sockAddr.sin_port			= htons( port );

		//Bind the listening socket object to an actual socket.
		if ( bind( m_ListeningSocket, ( sockaddr* )&sockAddr, sizeof( sockAddr ) ) < 0 )
		{
			LogErrorMessage( "Failed to bind listening socket", "TCPListener" );
			return false;
		}

		// Start listening for incoming connections
		if ( listen( m_ListeningSocket, NetworkUtility::MAX_LISTENING_BACKLOG ) < 0 )
		{
			LogErrorMessage( "Failed to start listening socket", "TCPListener" );
			return false;
		}

		m_IsRunning			= true;
		m_IsListening		= true;
		m_ListenerThread	= std::thread( &TCPListener::ListenForConnection, this, port );
		return true;
	}
	else
	{
		Logger::Log( "Attempted to start listening on an already listening socket", "TCPListener", LogSeverity::WARNING_MSG );
		return false;
	}
}

void TCPListener::StopListening()
{
	if ( m_IsListening )
	{
		m_IsRunning = false;
		int result;
		#if PLATFORM == PLATFORM_WINDOWS
			result = closesocket( m_ListeningSocket );
		#else
			shutdown( m_ListeningSocket, SHUT_RDWR ); // TODODB: Throwing warnings/errors on Linux. But must be here or else freeze.
			result = close( m_ListeningSocket );
		#endif
		if ( result != 0 )
			LogErrorMessage( "Failed to close listening socket", "TCPListener" );

		m_ListeningSocket = INVALID_SOCKET;
		ThreadingUtility::JoinThread( m_ListenerThread );
		m_IsListening = false;
	}
	else
		Logger::Log( "Attempted to stop listening socket while it was not listening", "TCPListener", LogSeverity::WARNING_MSG );
}

LocklessQueue<TCPSocket*>& TCPListener::GetConnectionQueue()
{
	return m_ConnectionQueue;
}

bool TCPListener::SetTimeout( unsigned int seconds )
{
	bool result;

	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;

	result = setsockopt( m_ListeningSocket, SOL_SOCKET, SO_RCVTIMEO, ( char * )&timeout, sizeof( timeout ) ) < 0;
	result = setsockopt( m_ListeningSocket, SOL_SOCKET, SO_SNDTIMEO, ( char * )&timeout, sizeof( timeout ) ) < 0;

	if ( !result )
		LogErrorMessage( "Failed to set timeout time", "TCPSocket" );

	return result;
}

bool TCPListener::IsListening( ) const
{
	return m_IsListening;
}

void TCPListener::ListenForConnection( const unsigned short port )
{
	Logger::Log( "Listening for incoming connections on port " + rToString( port ), "TCPListener", LogSeverity::INFO_MSG );
	while ( m_IsRunning )
	{
		Socket					incomingConnectionSocket;
		sockaddr_in				incomingConnectionInfo;
		socklen_t				incomingConnectionInfoLength = sizeof( incomingConnectionInfo );

		// Wait for a connection or fetch one from the backlog
		incomingConnectionSocket = static_cast<Socket>( accept( m_ListeningSocket, ( sockaddr* )&incomingConnectionInfo, &incomingConnectionInfoLength ) ); // Blocking
		if ( incomingConnectionSocket != INVALID_SOCKET )
		{
 			TCPSocket* newConnection = pNew ( TCPSocket, incomingConnectionSocket, IPv4Address( incomingConnectionInfo ) );
			m_ConnectionQueue.Produce( newConnection );
		}
		else
		{
			int error = GET_NETWORK_ERROR;
			if ( error != ROBOEINTR ) // The socket was killed on purpose
				LogErrorMessage( "Incoming connection attempt failed", "TCPListener" );
		}
	}
	Logger::Log( "Listening thread is terminating gracefully", "TCPListener", LogSeverity::INFO_MSG );
}