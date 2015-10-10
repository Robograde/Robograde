/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include <string>
#include "SerializationUtility.h"

void SerializationUtility::CopyAndIncrementDestination( Byte*& destination, const void* const source, const size_t length )
{
	memcpy( destination, source, length );
	destination += length;
}

void SerializationUtility::CopyAndIncrementSource( void* const destination, const Byte*& source, const size_t length )
{
	memcpy( destination, source, length );
	source += length;
}