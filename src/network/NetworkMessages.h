/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include <limits.h>
#include <memory/Alloc.h>
#include <messaging/Message.h>

#define INVALID_SENDER_ID USHRT_MAX

#define CONNECTION_STATUS_ENUM_UNDERLYING_TYPE char

namespace NetworkMessageTypes
{
	enum NetworkMessageType : char // TODODB: Make define for this type and use it
	{
		BAD,
		PING,
		HANDSHAKE,
		NETWORKIDMESSAGE,
		CONNECTION_STATUS_MESSAGE,	// TODODB: Change other entries to this standard
		LATENCY_UPDATE_MESSAGE,
	};
}

namespace ConnectionStatusChanges
{
	enum ConnectionStatus : char
	{
		CONNECTED,
		DISCONNECTED,
	};
}

class NetworkMessage : public Message // TODODB: Use Byte instead of char for serialization buffers
{
public:
	NetworkMessage();
	NetworkMessage( NetworkMessageTypes::NetworkMessageType networkMessageType, unsigned short senderID );
	virtual ~NetworkMessage() {};

	virtual Message* Clone() const override = 0;
	virtual unsigned int GetSerializationSize() const override;
	virtual void Serialize( char*& buffer ) const override;
	virtual void Deserialize( const char*& buffer ) override;

	NetworkMessageTypes::NetworkMessageType NetworkMessageType = NetworkMessageTypes::BAD;
	unsigned short SenderID = INVALID_SENDER_ID;
};

class PingMessage : public NetworkMessage
{
public:
	PingMessage();

	Message* Clone() const override;
	unsigned int GetSerializationSize() const override;
	void Serialize( char*& buffer ) const override;
	void Deserialize( const char*& buffer ) override;
};

class LatencyUpdateMessage : public NetworkMessage
{
public:
	LatencyUpdateMessage();
	LatencyUpdateMessage( const rMap<short, double>& latencies );

	Message* Clone() const override;
	unsigned int GetSerializationSize() const override;
	void Serialize( char*& buffer ) const override;
	void Deserialize( const char*& buffer ) override;

	rMap<short, double> Latencies;
};

class HandshakeMessage : public NetworkMessage
{
public:
	HandshakeMessage();

	Message* Clone() const override;
	unsigned int GetSerializationSize() const override;
	void Serialize( char*& buffer ) const override;
	void Deserialize( const char*& buffer ) override;
};

class NetworkIDMessage : public NetworkMessage
{
public:
	NetworkIDMessage();
	NetworkIDMessage( short networkID, const rVector<short>& connectedClients );

	Message* Clone() const override;
	unsigned int GetSerializationSize() const override;
	void Serialize( char*& buffer ) const override;
	void Deserialize( const char*& buffer ) override;

	short NetworkID = -1;
	rVector<short> ConnectedClients;
};

class ConnectionStatusMessage : public NetworkMessage
{
public:
	ConnectionStatusMessage();
	ConnectionStatusMessage( short networkID, ConnectionStatusChanges::ConnectionStatus newStatus );

	Message* Clone() const override;
	unsigned int GetSerializationSize() const override;
	void Serialize( char*& buffer ) const override;
	void Deserialize( const char*& buffer ) override;

	short NetworkID;
	CONNECTION_STATUS_ENUM_UNDERLYING_TYPE ConnectionStatus;
};