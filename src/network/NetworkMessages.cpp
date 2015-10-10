/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "NetworkMessages.h"
#include <memory/Alloc.h>
#include <utility/SerializationUtility.h>
#include "NetworkInfo.h"

using namespace SerializationUtility;
NetworkMessage::NetworkMessage() : Message( 0 )
{}

NetworkMessage::NetworkMessage( NetworkMessageTypes::NetworkMessageType networkMessageType, unsigned short senderID = g_NetworkInfo.GetNetworkID() ) : Message( 0 )
{
	NetworkMessageType	= networkMessageType;
	SenderID			= senderID;
}

Message* NetworkMessage::Clone() const
{
	return tNew( NetworkMessage, *this );
}

unsigned int NetworkMessage::GetSerializationSize() const // Measures 8 bytes
{
	return static_cast<unsigned int>( sizeof( NetworkMessageTypes::NetworkMessageType ) + sizeof(unsigned short) + Message::GetSerializationSize() );
}

void NetworkMessage::Serialize( char*& buffer ) const // Writes 8 bytes
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &NetworkMessageType, sizeof( NetworkMessageTypes::NetworkMessageType ) );
	CopyAndIncrementDestination( buffer, &SenderID, sizeof( unsigned short ) );
}

void NetworkMessage::Deserialize( const char*& buffer ) // Reads 8 bytes
{
	Message::Deserialize( buffer );
	SerializationUtility::CopyAndIncrementSource( &NetworkMessageType, buffer, sizeof( NetworkMessageTypes::NetworkMessageType ) );
	SerializationUtility::CopyAndIncrementSource( &SenderID, buffer, sizeof( unsigned short ) );
}

PingMessage::PingMessage() : NetworkMessage( NetworkMessageTypes::PING )
{}

Message* PingMessage::Clone() const
{
	return tNew( PingMessage, *this );
}

unsigned int PingMessage::GetSerializationSize() const
{
	return static_cast<unsigned int>( sizeof( double ) );
}

void PingMessage::Serialize( char*& buffer ) const
{
	NetworkMessage::Serialize( buffer );
}

void PingMessage::Deserialize( const char*& buffer )
{
	NetworkMessage::Deserialize( buffer );
}

LatencyUpdateMessage::LatencyUpdateMessage()
{}

LatencyUpdateMessage::LatencyUpdateMessage( const rMap<short, double>& latestPingTimes ) : NetworkMessage( NetworkMessageTypes::LATENCY_UPDATE_MESSAGE )
{
	Latencies = latestPingTimes;
}

Message* LatencyUpdateMessage::Clone() const
{
	return tNew( LatencyUpdateMessage, *this );
}

unsigned int LatencyUpdateMessage::GetSerializationSize() const
{
	unsigned int mapSize = static_cast<unsigned int>( Latencies.size() );
	return static_cast<unsigned int>( sizeof( unsigned int ) + ( mapSize * sizeof( short ) ) + ( mapSize  * sizeof( double ) ) + NetworkMessage::GetSerializationSize() );
}

void LatencyUpdateMessage::Serialize( char*& buffer ) const
{
	NetworkMessage::Serialize( buffer );

	unsigned int mapSize = static_cast< unsigned int >( Latencies.size() );
	CopyAndIncrementDestination( buffer, &mapSize, sizeof( unsigned int ) );
	for ( auto& pingTimePair : Latencies )
	{
		CopyAndIncrementDestination( buffer, &pingTimePair.first, sizeof(short) );
		CopyAndIncrementDestination( buffer, &pingTimePair.second, sizeof( double ) );
	}
}

void LatencyUpdateMessage::Deserialize( const char*& buffer )
{
	NetworkMessage::Deserialize( buffer );

	unsigned int mapSize;
	CopyAndIncrementSource( &mapSize, buffer, sizeof( unsigned int ) );
	short networkID;
	double pingTime;
	for ( unsigned int i = 0; i < mapSize; ++i )
	{
		CopyAndIncrementSource( &networkID, buffer, sizeof( short ) );
		CopyAndIncrementSource( &pingTime, buffer, sizeof( double ) );
		Latencies.emplace( networkID, pingTime );
	}
}

HandshakeMessage::HandshakeMessage() : NetworkMessage( NetworkMessageTypes::HANDSHAKE )
{}

Message* HandshakeMessage::Clone() const
{
	return tNew( HandshakeMessage, *this );
}

unsigned int HandshakeMessage::GetSerializationSize() const
{
	return static_cast<unsigned int>( NetworkMessage::GetSerializationSize() );
}

void HandshakeMessage::Serialize( char*& buffer ) const
{
	NetworkMessage::Serialize( buffer );
}

void HandshakeMessage::Deserialize( const char*& buffer )
{
	NetworkMessage::Deserialize( buffer );
}

NetworkIDMessage::NetworkIDMessage()
{}

NetworkIDMessage::NetworkIDMessage( short networkID, const rVector<short>& connectedClients) : NetworkMessage( NetworkMessageTypes::NETWORKIDMESSAGE )
{
	NetworkID			= networkID;
	ConnectedClients	= connectedClients;
}

Message* NetworkIDMessage::Clone() const
{
	return tNew( NetworkIDMessage, *this );
}

unsigned int NetworkIDMessage::GetSerializationSize() const
{
	return static_cast<unsigned int>( sizeof( short ) + sizeof( unsigned int ) + ( ConnectedClients.size() * sizeof( short ) ) + NetworkMessage::GetSerializationSize() );
}

void NetworkIDMessage::Serialize( char*& buffer ) const
{
	NetworkMessage::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &NetworkID, sizeof( short ) );

	unsigned int vectorSize = static_cast<unsigned int>( ConnectedClients.size() );
	CopyAndIncrementDestination( buffer, &vectorSize, sizeof( unsigned int ) );
	CopyAndIncrementDestination( buffer, ConnectedClients.data(), vectorSize * sizeof( short ) );
}

void NetworkIDMessage::Deserialize( const char*& buffer )
{
	NetworkMessage::Deserialize( buffer );
	CopyAndIncrementSource( &NetworkID, buffer, sizeof( short ) );

	unsigned int vectorSize;
	CopyAndIncrementSource( &vectorSize, buffer, sizeof( unsigned int ) );
	ConnectedClients.insert( ConnectedClients.begin(), reinterpret_cast<const short*>( buffer ), reinterpret_cast<const short*>( buffer )+vectorSize );
	buffer += sizeof( short ) * vectorSize;
}

ConnectionStatusMessage::ConnectionStatusMessage()
{}

ConnectionStatusMessage::ConnectionStatusMessage( short networkID, ConnectionStatusChanges::ConnectionStatus newStatus ) : NetworkMessage( NetworkMessageTypes::CONNECTION_STATUS_MESSAGE )
{
	NetworkID = networkID;
	ConnectionStatus = newStatus;
}

Message* ConnectionStatusMessage::Clone() const
{
	return tNew( ConnectionStatusMessage, *this );
}

unsigned int ConnectionStatusMessage::GetSerializationSize() const
{
	return ( sizeof( short ) + sizeof( CONNECTION_STATUS_ENUM_UNDERLYING_TYPE ) + NetworkMessage::GetSerializationSize() );
}

void ConnectionStatusMessage::Serialize( char*& buffer ) const
{
	NetworkMessage::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &NetworkID, sizeof( short ) );
	CopyAndIncrementDestination( buffer, &ConnectionStatus, sizeof( CONNECTION_STATUS_ENUM_UNDERLYING_TYPE ) );
}

void ConnectionStatusMessage::Deserialize( const char*& buffer )
{
	NetworkMessage::Deserialize( buffer );
	CopyAndIncrementSource( &NetworkID, buffer, sizeof( short ) );
	CopyAndIncrementSource( &ConnectionStatus, buffer, sizeof( CONNECTION_STATUS_ENUM_UNDERLYING_TYPE ) );
}