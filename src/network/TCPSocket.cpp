/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include <utility/PlatformDefinitions.h>
#include "TCPSocket.h"
#include <utility/Logger.h>
#include <memory/Alloc.h>
#include <messaging/GameMessages.h>
#if PLATFORM == PLATFORM_LINUX
#include <unistd.h>
#include <netinet/tcp.h>	// Needed for TCP_NODELAY
#import <fcntl.h>			// Needed in order to set socket to non blocking mode
#endif
#include "NetworkMessages.h"
#if PLATFORM == PLATFORM_WINDOWS
	#define SENDFLAGS				0
#elif PLATFORM == PLATFORM_LINUX
	#define SENDFLAGS				MSG_NOSIGNAL
#endif

#if PLATFORM == PLATFORM_WINDOWS
	#define SHOULD_WAIT_FOR_TIMEOUT static_cast<bool>( GET_NETWORK_ERROR == WSAEWOULDBLOCK )
#elif PLATFORM == PLATFORM_LINUX
	#define SHOULD_WAIT_FOR_TIMEOUT static_cast<bool>( GET_NETWORK_ERROR == EINPROGRESS )
#endif

#define CONNECTION_TIMEOUT_SECONDS 2

TCPSocket::TCPSocket()
{
	m_ExpectedHeaderBytes	= sizeof( int );
	m_ExpectedPayloadBytes	= -1;
	m_PayloadData			= nullptr;
	m_Pos					= reinterpret_cast<Byte*>( &m_ExpectedPayloadBytes );
}

TCPSocket::TCPSocket( Socket socket, IPv4Address destination ) : TCPSocket::TCPSocket( )
{
	m_Socket		= socket;
	m_Destination	= destination;
	m_Connected		= true;

	SetBlockingMode( false );
	SetNoDelay();
}

TCPSocket::~TCPSocket()
{
	// Delete any half received messages
	if ( m_PayloadData != nullptr )
		tFree( m_PayloadData );
}

bool TCPSocket::Connect( const rString& adress, const unsigned short port )
{
	IPv4Address destination( adress, port );
	return Connect( destination );
}

bool TCPSocket::Connect( const IPv4Address& destination )
{
	//Set up the socket
	m_Socket = static_cast< int >( socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) );	// Adress Family = INET and the protocol to be used is TCP
	if ( m_Socket <= 0 )
	{
		LogErrorMessage( "Failed to create socket", "TCPSocket" );
		return false;
	}

	SetBlockingMode( false );

	// Try to connect
	Logger::Log( "Attempting to connect to " + destination.GetPrintableAddressAndPort(), "TCPSocket", LogSeverity::INFO_MSG );
	sockaddr_in addr = destination.GetSockAddr();

	timeval timeOut;
	timeOut.tv_sec = CONNECTION_TIMEOUT_SECONDS;
	timeOut.tv_usec = 0;

	fd_set set;
	FD_ZERO( &set );
	FD_SET( m_Socket, &set );

	int returnVal;
	if ( ( returnVal = connect( m_Socket, ( sockaddr * )&addr, sizeof( sockaddr_in ) ) ) == INVALID_SOCKET )
	{
		if ( !SHOULD_WAIT_FOR_TIMEOUT )
			return false;
	}
	returnVal = select( static_cast<int>( m_Socket + 1 ), NULL, &set, NULL, &timeOut );

	if ( returnVal > 0 ) // A socket was connected
	{
		m_Destination = destination;
		m_Connected = true;

		SetNoDelay();

		Logger::Log( "Connection attempt to " + destination.GetPrintableAddressAndPort() + " was successfull!", "TCPSocket", LogSeverity::INFO_MSG );
		return true;
	}
	else if ( returnVal == 0 )
	{
		Logger::Log( "Connection attempt to " + destination.GetPrintableAddressAndPort() + " timed out", "TCPSocket", LogSeverity::INFO_MSG );
		return false;
	}
	else
	{
		LogErrorMessage( "Connection attempt to " + destination.GetPrintableAddressAndPort() + " failed", "TCPSocket" );
		return false;
	}
}

void TCPSocket::Disconnect()
{
	int result;
#if PLATFORM == PLATFORM_WINDOWS
	result = shutdown( m_Socket, SD_BOTH );
#else
	result = shutdown( m_Socket, SHUT_RDWR );
#endif
	if ( result != 0 )
		LogErrorMessage( "Failed to shut down socket", "TCPSocket" );

#if PLATFORM == PLATFORM_WINDOWS
	result = closesocket( m_Socket );
#else
	result = close( m_Socket );
#endif
	if ( result != 0 )
		LogErrorMessage( "Failed to close socket", "TCPSocket" );
	
	m_Socket = INVALID_SOCKET;
	m_Connected = false;
}

bool TCPSocket::SendPacket( const Message& packet )
{
	bool result = false;
	unsigned int dataSize = sizeof( unsigned int ) + packet.GetSerializationSize();
	Byte* serializedPacket = static_cast<Byte*>( tMalloc( dataSize ) );
	Byte* walker = serializedPacket;
	SerializationUtility::CopyAndIncrementDestination( walker, &dataSize, sizeof( unsigned int ) );
	packet.Serialize( walker );

#if NETWORK_DEBUG
	if ( walker != serializedPacket + dataSize )
	{
		int differentiatingSize = static_cast<int>(walker - serializedPacket - dataSize);
		assert(false);
	}
	MemoryAllocator::VerifyPointer( serializedPacket );
#endif

	if ( serializedPacket != nullptr )
	{
		result = SendRawData( serializedPacket, dataSize );
		tFree( serializedPacket );
	}

	return result;
}

bool TCPSocket::SendRawData( const Byte* const data, const unsigned int dataSize )
{
	if ( m_Socket == INVALID_SOCKET )
	{
		Logger::Log( "Attempted to send to invalid socket", "TCPSocket", LogSeverity::WARNING_MSG );
		return false;
	}

	int bytesSent = send( m_Socket, data, dataSize, SENDFLAGS );
	if ( bytesSent != dataSize )
	{
		int error = GET_NETWORK_ERROR;
		if ( error == ROBOECONNECTIONABORTED || error == ROBOEWOULDBLOCK || error == EPIPE || error == ROBOECONNRESET )
		{
			m_Connected = false;
			Logger::Log( "Connection to " + m_Destination.GetPrintableAddressAndPort() + " was aborted", "TCPSocket", LogSeverity::INFO_MSG );
		}
		else
			LogErrorMessage( "Sending of packet with length " + rToString( dataSize ) + " and destination " + m_Destination.GetPrintableAddressAndPort() + " failed", "TCPSocket" );

		return false;
	}
	return true;
}

Message* TCPSocket::Receive()
{
	if ( m_Socket == INVALID_SOCKET )
	{
		Logger::Log( "Attempted to receive from invalid socket", "TCPSocket", LogSeverity::WARNING_MSG );
		return nullptr;
	}

	int nrOfBytesReceived;
	if ( m_ExpectedHeaderBytes > 0 ) // If we are waiting for header data
	{
		nrOfBytesReceived = recv( m_Socket, m_Pos, m_ExpectedHeaderBytes, 0 ); // Try to receive header

		if ( nrOfBytesReceived == 0 ) // Check if the socket was gracefully disconnected
		{
			m_Connected = false;
			Logger::Log( "Connection to " + m_Destination.GetPrintableAddressAndPort() + " terminated gracefully", "TCPSocket", LogSeverity::INFO_MSG );
		} 
		else if ( nrOfBytesReceived == -1 )
		{
			int error = GET_NETWORK_ERROR;
			if ( error != ROBOEWOULDBLOCK )
			{
				if ( error == ROBOECONNECTIONABORTED || error == ROBOECONNRESET )
				{
					m_Connected = false;
					Logger::Log( "Connection to " + m_Destination.GetPrintableAddressAndPort( ) + " was aborted", "TCPSocket", LogSeverity::INFO_MSG );
				}
				else
					LogErrorMessage( "An unhandled error occured while receiving header data", "TCPSocket" );
			}
			return nullptr; // No data was ready to be received or there was an error
		}

		if ( nrOfBytesReceived == m_ExpectedHeaderBytes )
		{
			m_ExpectedHeaderBytes = 0;	// We received the full header

			// Get the size of the packet (Embedded as first part) and create a buffer of that size
			m_PayloadData = static_cast<Byte*>( tMalloc( m_ExpectedPayloadBytes ) );

			m_ExpectedPayloadBytes -= sizeof( unsigned int ); // We have already received the size value
			m_Pos = m_PayloadData;	// Pos now points to the new buffer since that is where we will want to write on the next recv
		}
		else // Only a part of the header was received. Account for this and handle it on next function call
		{
			m_ExpectedHeaderBytes -= nrOfBytesReceived;
			m_Pos += nrOfBytesReceived;
			return nullptr;
		}
	}

	nrOfBytesReceived = recv( m_Socket, m_Pos, m_ExpectedPayloadBytes, 0 ); // Try to receive payload

	if ( nrOfBytesReceived == -1 )
	{
		int error = GET_NETWORK_ERROR;
		if ( error != ROBOEWOULDBLOCK )
		{
			if ( error == ROBOECONNECTIONABORTED )
			{
				m_Connected = false;
				Logger::Log( "Connection to " + m_Destination.GetPrintableAddressAndPort( ) + " was aborted", "TCPSocket", LogSeverity::INFO_MSG );
			}
			else
				LogErrorMessage( "An unhandled error occured while receiving payload data", "TCPSocket" );

		}
		return nullptr; // No data was ready to be received or there was an error
	}

	// If all data was received. Clean up, prepare for next call and return the buffer as a packet (Will need to be cast to the correct type on the outside using the Type field)
	if ( nrOfBytesReceived == m_ExpectedPayloadBytes )
	{
		MessageTypes::MessageType messageType;
		Byte* walker = m_PayloadData;

		memcpy( &messageType, walker, sizeof( MESSAGE_TYPE_ENUM_UNDELYING_TYPE ) );
		Message* packet;
		if ( messageType == 0 )
		{
			NetworkMessageTypes::NetworkMessageType networkedMessageType;
			memcpy( &networkedMessageType, walker + sizeof( MESSAGE_TYPE_ENUM_UNDELYING_TYPE ) + sizeof( bool ), sizeof( NetworkMessageTypes::NetworkMessageType ) );
			packet = NetworkMessages::GetDefaultMessage( networkedMessageType );
		}
		else
		{
			packet = Messages::GetDefaultMessage( messageType );
		}
		packet->Deserialize( ( const char*& )walker );

#if NETWORK_DEBUG
		if ( walker != m_PayloadData + packet->GetSerializationSize( ) )
		{
			int differentiatingSize = walker - m_PayloadData - packet->GetSerializationSize( );
			assert( false );
		}
		MemoryAllocator::VerifyPointer( packet );
#endif

		tFree( m_PayloadData );
		m_PayloadData = nullptr;
		m_Pos = reinterpret_cast<Byte*>( &m_ExpectedPayloadBytes );
		m_ExpectedPayloadBytes = -1;
		m_ExpectedHeaderBytes = sizeof( unsigned int );

		return packet;
	}
	else // Only part of the payload was received. Account for this and try to receive the rest in an upcoming function call
	{
		m_ExpectedPayloadBytes -= nrOfBytesReceived;
		m_Pos += nrOfBytesReceived;
		return nullptr;
	}
}

bool TCPSocket::SetTimeout( unsigned int seconds )
{
	bool result1;
	bool result2;
	bool toReturn;

	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;

	result1 = setsockopt( m_Socket, SOL_SOCKET, SO_RCVTIMEO, ( char * )&timeout, sizeof( timeout ) ) != SOCKET_ERROR;
	result2 = setsockopt( m_Socket, SOL_SOCKET, SO_SNDTIMEO, ( char * )&timeout, sizeof( timeout ) ) != SOCKET_ERROR;

	toReturn = result1 && result2;
	if ( !toReturn )
		LogErrorMessage( "Failed to set timeout time", "TCPSocket" );

	return toReturn;
}

bool TCPSocket::IsConnected() const
{
	return m_Connected;
}

const IPv4Address& TCPSocket::GetDestination() const
{
	return m_Destination;
}

bool TCPSocket::SetBlockingMode( bool shouldBlock)
{
	int result;
#if PLATFORM == PLATFORM_WINDOWS
	unsigned long nonBlocking = static_cast<unsigned long>( !shouldBlock );
	result = ioctlsocket( m_Socket, FIONBIO, &nonBlocking );
#else
	if( shouldBlock ) // TODODB: Do this with inline if
		result = fcntl( m_Socket, F_SETFL, fcntl( m_Socket, F_GETFL, 1 ) | O_NONBLOCK );
	else
		result = fcntl( m_Socket, F_SETFL, fcntl( m_Socket, F_GETFL, 0 ) | O_NONBLOCK );
#endif
	if ( result == 0 )
		return true;

	Logger::Log( "Failed to set socket to non blocking mode (Socket destination = " + m_Destination.GetPrintableAddressAndPort( ) + " )", "TCPSocket", LogSeverity::WARNING_MSG );
	return false;
}

bool TCPSocket::SetNoDelay()
{
	int flag = 1;
	int result = setsockopt( m_Socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof( int ) );
	if ( result < 0 )
	{
		Logger::Log( "Failed to set TCP_NODELAY for socket with destination "+ m_Destination.GetPrintableAddressAndPort( ) + " (Error: " + rToString( result ) + ")", "TCPSocket", LogSeverity::ERROR_MSG );
		return false;
	}
	return true;
}