/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "IPv4Adress.h"

IPv4Address::IPv4Address( )
{
	memset( &m_SockAddr, 0, sizeof( m_SockAddr ) );
	m_SockAddr.sin_family		= AF_INET;
	m_SockAddr.sin_addr.s_addr	= -1;
	m_SockAddr.sin_port			= -1;
}

IPv4Address::IPv4Address( const unsigned int adress, const unsigned short port )
{
	m_Address	= adress;
	m_Port		= port;

	memset( &m_SockAddr, 0, sizeof( m_SockAddr ) );
	m_SockAddr.sin_family		= AF_INET;
	m_SockAddr.sin_addr.s_addr	= htonl( adress );
	m_SockAddr.sin_port			= htons( port );
}

IPv4Address::IPv4Address( const struct sockaddr_in destination )
{
	memset( &m_SockAddr, 0, sizeof( m_SockAddr ) );
	m_SockAddr.sin_family		= AF_INET;
	m_SockAddr.sin_addr.s_addr	= destination.sin_addr.s_addr;
	m_SockAddr.sin_port			= destination.sin_port;

	m_Address	= ntohl( destination.sin_addr.s_addr );
	m_Port		= ntohs( destination.sin_port );		// Local port if destination is a received connection
}

IPv4Address::IPv4Address( const rString& address, const rString& port ) : IPv4Address( address, static_cast< unsigned short >(std::stoul( port.c_str( ) )) )
{}

IPv4Address::IPv4Address( const rString& address, const unsigned short port )
{
	// Split the adress into its parts
	unsigned int adressParts[4];
	int startSearchPos = 0;
	for ( int i = 0; i < 4; ++i )
	{
		int stopSearchPos = static_cast< int >(address.find( '.', startSearchPos ));
		rString currentAdressPart = address.substr( startSearchPos, stopSearchPos - startSearchPos );
		startSearchPos = stopSearchPos + 1; // +1 to not find same delimiter on next search
		adressParts[i] = static_cast<unsigned int>(std::stoul( currentAdressPart.c_str() ) );
	}

	// Convert adress and port and set them
	m_Address	= (adressParts[0] << 24) | (adressParts[1] << 16) | (adressParts[2] << 8) | adressParts[3];
	m_Port		= port;

	memset( &m_SockAddr, 0, sizeof( m_SockAddr ) );
	m_SockAddr.sin_family		= AF_INET;
	m_SockAddr.sin_addr.s_addr	= htonl( m_Address );
	m_SockAddr.sin_port			= htons( m_Port );
}

unsigned int IPv4Address::GetAdress( ) const
{
	return m_Address;
}

unsigned short IPv4Address::GetPort( ) const
{
	return m_Port;
}

sockaddr_in IPv4Address::GetSockAddr( ) const
{
	return m_SockAddr;
}

rString IPv4Address::GetPrintableAddressAndPort( ) const
{
	return GetPrintableAdress() + ":" + GetPrintablePort( );
}

rString IPv4Address::GetPrintableAdress( ) const
{
	// Split adress in its components
	unsigned short ip1 = (m_Address >> 24);
	unsigned short ip2 = (m_Address >> 16) & 0xFF;
	unsigned short ip3 = (m_Address >> 8) & 0xFF;
	unsigned short ip4 =  m_Address & 0xFF;

	return rToString( ip1 ) + "." + rToString( ip2 ) + "." + rToString( ip3 ) + "." + rToString( ip4 );
}

rString IPv4Address::GetPrintablePort( ) const
{
	return rToString( m_Port );
}

bool IPv4Address::operator==(const IPv4Address& other) const
{
	return m_Address == other.m_Address && m_Port == other.m_Port;
}

bool IPv4Address::operator!=(const IPv4Address& other) const
{
	return m_Address != other.m_Address || m_Port != other.m_Port;
}