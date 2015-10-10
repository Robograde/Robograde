/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "PlatformDefinitions.h"
#if PLATFORM == PLATFORM_LINUX
#include <stdlib.h> // For size_t
#endif
#include "UtilityLibraryDefine.h"

namespace SerializationUtility {
	typedef char Byte;
	static_assert( sizeof( Byte ) == 1, "Byte must have a size of 1." );

	UTILITY_API void CopyAndIncrementDestination( Byte*& destination, const void* const source, const size_t length );
	UTILITY_API void CopyAndIncrementSource( void* const destination, const Byte*& source, const size_t length );
}