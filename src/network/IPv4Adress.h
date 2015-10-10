/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include <utility/PlatformDefinitions.h>
#include <memory/Alloc.h>
#if PLATFORM == PLATFORM_WINDOWS
#include <utility/RetardedWindowsIncludes.h>
#else
#include <sys/socket.h>
#include <netinet/in.h> 
#endif
#include "NetworkLibraryDefine.h"

class IPv4Address
{
public:
	IPv4Address( );
	IPv4Address( const unsigned int adress, const unsigned short port );
	IPv4Address( const struct sockaddr_in destination );
	IPv4Address( const rString& address, const rString& port );
	IPv4Address( const rString& address, const unsigned short port );

	unsigned int			GetAdress( ) const;
	unsigned short			GetPort( ) const;
	sockaddr_in				GetSockAddr( ) const;

	NETWORK_API rString		GetPrintableAddressAndPort() const;
	NETWORK_API rString		GetPrintableAdress() const;
	NETWORK_API rString		GetPrintablePort( ) const;

	bool operator == (const IPv4Address& other) const;
	bool operator != (const IPv4Address& other) const;

private:
	unsigned int		m_Address = 0;
	unsigned short		m_Port = 0;
	sockaddr_in			m_SockAddr;
};