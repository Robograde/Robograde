/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include <utility/Logger.h>
#include <utility/PlatformDefinitions.h>
#if PLATFORM == PLATFORM_WINDOWS
	#include <utility/RetardedWindowsIncludes.h>
#elif PLATFORM == PLATFORM_LINUX
#include <string.h> // Needed for strerror
#endif
#include "NetworkLibraryDefine.h"

#if PLATFORM == PLATFORM_WINDOWS
	#define GET_NETWORK_ERROR WSAGetLastError()
#elif PLATFORM == PLATFORM_LINUX
	#define GET_NETWORK_ERROR errno
#endif

#if PLATFORM != PLATFORM_WINDOWS // winsock already defines this
#define INVALID_SOCKET	~0
#define SOCKET_ERROR	-1
#endif

// Always output errors through this define or rString constructors may overwrite errno
#define LogErrorMessage(outputMessage, channel) {int __errorCode = GET_NETWORK_ERROR; Logger::Log( rString(outputMessage " - Error: ") + GetErrorName( __errorCode ), channel, LogSeverity::ERROR_MSG );}
#define LOCALHOST_IP	"127.0.0.1"
#define INVALID_PORT	0
#define INVALID_NETWORK_ID -1

#define NETWORK_DEBUG 0

// Define error codes in a cross platform manner
#if PLATFORM == PLATFORM_WINDOWS
#define ROBOEWOULDBLOCK			WSAEWOULDBLOCK
#define ROBOECONNECTIONABORTED	WSAECONNABORTED
#define ROBOECONNRESET			WSAECONNRESET
#define ROBOEINTR				WSAEINTR
#elif PLATFORM == PLATFORM_LINUX
#define ROBOEWOULDBLOCK			EWOULDBLOCK
#define ROBOECONNECTIONABORTED	ECONNABORTED
#define ROBOECONNRESET			ECONNRESET
#define ROBOEINTR				EINTR
#endif

namespace NetworkUtility
{
	typedef char Byte;
	static_assert( sizeof( Byte ) == 1, "Byte must have a size of 1." );
	typedef unsigned long long Socket;

	const int	MAX_MESSAGE_SIZE		= 576;				// Max "safe" size is 576 in IPv4
	const int	MAX_LISTENING_BACKLOG	= 16;				// How many incoming connections that can be connecting at the same time

	NETWORK_API void			GetIPAndPortFromString( const rString& input, rString& outIP, unsigned short& outPort );
	NETWORK_API rString			GetIPFromString( const rString& input );
	NETWORK_API unsigned short	GetPortFromString( const rString& input );
	NETWORK_API rString 		GetIPFromDNS( const rString& input );
	rString						GetErrorName( int error );
}