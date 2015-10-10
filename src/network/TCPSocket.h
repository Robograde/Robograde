/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include <messaging/Message.h>
#include "IPv4Adress.h"
#include "NetworkUtility.h"

using namespace NetworkUtility;
class TCPSocket
{
public:
	TCPSocket( );
	TCPSocket( Socket socket, IPv4Address destination );
	~TCPSocket( );

	bool				Connect( const rString& adress, const unsigned short port );
	bool				Connect( const IPv4Address& destination );
	void				Disconnect();

	bool				SendPacket( const Message& packet );
	bool				SendRawData( const Byte* const data, const unsigned int dataSize );
	Message*			Receive();

	bool				SetTimeout( unsigned int seconds );
	
	bool				IsConnected() const;
	const IPv4Address&	GetDestination() const;

private:
	bool				SetBlockingMode( bool shouldBlock );
	bool				SetNoDelay();
	
	Socket				m_Socket;
	IPv4Address			m_Destination;

	bool				m_Connected	= false;

	// Receive
	int					m_ExpectedHeaderBytes;		// How many more bytes of header we expect for the current packet
	int					m_ExpectedPayloadBytes;		// How many more bytes of payload we expect for the current packet
	Byte*				m_PayloadData;				// Dynamic buffer for packet payload
	Byte*				m_Pos;						// Pointer to where the next recv should write to
};