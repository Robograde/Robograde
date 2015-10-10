/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "NetworkUtility.h"
#include <algorithm>
#include <string>
#include <utility/StringUtility.h>
#if PLATFORM == PLATFORM_WINDOWS
#include <ws2tcpip.h>
#elif PLATFORM == PLATFORM_LINUX
#include <netdb.h>
#include <arpa/inet.h>
#endif

#define DOTS_IN_IPv4ADDRESS 3

NETWORK_API void NetworkUtility::GetIPAndPortFromString( const rString& input, rString& outIP, unsigned short& outPort )
{
	if ( input == "" )
		return;

	outIP	= "";
	outPort = INVALID_PORT;

	size_t colonCount = std::count( input.begin(), input.end(), ':' );

	if ( colonCount == 1 )
	{
		outIP = GetIPFromString( input );
		size_t colonPosition = input.find( ':' );
		rString portString = input.substr( colonPosition + 1 );
		if ( StringUtility::IsDigitsOnly( portString ) )
		{
			outPort = GetPortFromString( portString );
		}
	}
	else if ( StringUtility::IsDigitsOnly( input ) )
	{
		outPort = GetPortFromString( input );
	}
	else
	{
		outIP = GetIPFromString( input );
	}
}

rString NetworkUtility::GetIPFromString( const rString& input )
{
	if ( input.find_first_not_of( ".0123456789:" ) != std::string::npos ) // Check if DNS
	{
		return GetIPFromDNS( input );
	}
	size_t dotCount = std::count( input.begin(), input.end(), '.' );
	if ( dotCount != DOTS_IN_IPv4ADDRESS )
		return "";

	unsigned int adressParts[4];
	rString portString;
	rString processString = input;
	for ( int i = 0; i < 4; ++i )
	{
		size_t pos = processString.find_first_of( ".:" );
		rString currentAdressPart = processString.substr( 0, pos );
		try
		{
			adressParts[i] = static_cast<unsigned int>( std::stoul( currentAdressPart.c_str() ) );
		} 
		catch ( ... )
		{
			return "";
		}
		if ( adressParts[i] >= 256 )
		{
			return "";
		}
		processString.erase( 0, pos + 1 );
	}

	rOStringStream iposs;
	iposs << adressParts[0] << "." << adressParts[1] << "." << adressParts[2] << "." << adressParts[3];
	return iposs.str();
}

unsigned short NetworkUtility::GetPortFromString( const rString& input )
{
	if ( input != "" && StringUtility::IsDigitsOnly(input) )
	{
		unsigned int port = 0;
		try 
		{
			port = static_cast<unsigned int>( std::stoul( input.c_str() ) );
		}
		catch ( ... ) 
		{
			return 0;
		}
		if ( port <= SHRT_MAX )
		{
			return port;
		}
		else
			return 0;
	}
	else 
		return 0;
}

rString NetworkUtility::GetIPFromDNS( const rString& input )
{
	rString ip = "";
	size_t colonPos = input.find_first_of( ":" );
	rString dns = input;
	if ( colonPos != std::string::npos )
		dns = input.substr( 0, colonPos );

	addrinfo*	result	= nullptr;
	addrinfo	hints;

	memset(&hints, 0, sizeof( addrinfo ) );
	hints.ai_family		= AF_INET; // TODODB: When IPv6 implemented change this to AF_UNSPEC, for now force IPv4
	hints.ai_socktype	= SOCK_STREAM;
	hints.ai_protocol	= IPPROTO_TCP;

#if PLATFORM == PLATFORM_WINDOWS
	DWORD returnValue;
#elif PLATFORM == PLATFORM_LINUX
	int returnValue;
#endif

	returnValue	= getaddrinfo( dns.c_str(), nullptr, &hints, &result );

	if ( returnValue != 0 )
	{
		// TODODB: Handle dns lookup failure somehow
		Logger::Log( "Failed DNS lookup with error: " + rString( gai_strerror( returnValue ) ), "Network", LogSeverity::WARNING_MSG );
		return ip;
	}

	// result will be a linked list, use the first entry
	void *addr;
	if(result->ai_family == AF_INET)
	{
		sockaddr_in *ipv4 = (struct sockaddr_in *)result->ai_addr;
		addr = &(ipv4->sin_addr);

		char ipstr[INET_ADDRSTRLEN];
		// convert the IP to a string and print it:
		inet_ntop(result->ai_family, addr, ipstr, sizeof( ipstr ) );
		ip = rString( ipstr );
	}
	// TODODB: Handle IPv6 when implemented and move inet_ntop to relevant place
	//} else { // Is IPv6
	//struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
	//addr = &(ipv6->sin6_addr);
	//}

	// Free the linked list	
	freeaddrinfo( result );
	return ip;
}

// This function is ugly as fuck and I blame Microsoft
rString NetworkUtility::GetErrorName( int error )
{
#if PLATFORM == PLATFORM_WINDOWS
	wchar_t *wc = nullptr;
	FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		reinterpret_cast<LPWSTR>( &wc ), 0, NULL );
	std::wstring ws = wc;
	rString str;
	str.assign( ws.begin(), ws.end() );
	return str;
#else
	return strerror( error );
#endif
}