/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include <memory/Alloc.h>
#include "SerializationUtility.h"
#include "DataSizes.h"
#include <glm/gtc/quaternion.hpp>

using namespace DataSizes;

//************************** Utility **************************

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

//************************** Writing **************************

void SerializationUtility::WriteInt16( int16_t value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value, INT_16_SIZE );
}

void SerializationUtility::WriteInt32( int32_t value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value, INT_32_SIZE );
}

void SerializationUtility::WriteInt64( int64_t value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value, INT_64_SIZE );
}

void SerializationUtility::WriteUint16( uint16_t value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value, INT_16_SIZE );
}

void SerializationUtility::WriteUint32( uint32_t value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value, INT_32_SIZE );
}

void SerializationUtility::WriteUint64( uint64_t value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value, INT_64_SIZE );
}

void SerializationUtility::WriteFloat( float value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value, FLOAT_SIZE );
}

void SerializationUtility::WriteDouble( double value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value, DOUBLE_SIZE );
}

void SerializationUtility::WriteBool( bool value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value, BOOL_SIZE );
}

void SerializationUtility::WriteVec2( const	glm::vec2&	value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value.x, FLOAT_SIZE );
	CopyAndIncrementDestination( destination, &value.y, FLOAT_SIZE );
}

void SerializationUtility::WriteVec3( const	glm::vec3&	value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value.x, FLOAT_SIZE );
	CopyAndIncrementDestination( destination, &value.y, FLOAT_SIZE );
	CopyAndIncrementDestination( destination, &value.z, FLOAT_SIZE );
}

void SerializationUtility::WriteQuaternion( const glm::quat& value, Byte*& destination )
{
	CopyAndIncrementDestination( destination, &value.w, FLOAT_SIZE );
	CopyAndIncrementDestination( destination, &value.x, FLOAT_SIZE );
	CopyAndIncrementDestination( destination, &value.y, FLOAT_SIZE );
	CopyAndIncrementDestination( destination, &value.z, FLOAT_SIZE );
}

void SerializationUtility::WriteString( const rString& value, Byte*& destination )
{
	uint32_t stringLength = static_cast<uint32_t>( value.size() );
	CopyAndIncrementDestination( destination, &stringLength, INT_32_SIZE );
	CopyAndIncrementDestination( destination, value.data(), stringLength );
}

//************************** Reading **************************

void SerializationUtility::ReadInt16( int16_t& value, const Byte*& source )
{
	CopyAndIncrementSource( &value, source, INT_16_SIZE );
}

void SerializationUtility::ReadInt32( int32_t& value, const Byte*& source )
{
	CopyAndIncrementSource( &value, source, INT_32_SIZE );
}

void SerializationUtility::ReadInt64( int64_t& value, const Byte*& source )
{
	CopyAndIncrementSource( &value, source, INT_64_SIZE );
}

void SerializationUtility::ReadUInt16( uint16_t& value, const Byte*& source )
{
	CopyAndIncrementSource( &value, source, INT_16_SIZE );
}

void SerializationUtility::ReadUint32( uint32_t& value, const Byte*& source )
{
	CopyAndIncrementSource( &value, source, INT_32_SIZE );
}

void SerializationUtility::ReadUint64( uint64_t& value, const Byte*& source )
{
	CopyAndIncrementSource( &value, source, INT_64_SIZE );
}

void SerializationUtility::ReadFloat( float& value, const Byte*& source )
{
	CopyAndIncrementSource( &value, source, FLOAT_SIZE );
}

void SerializationUtility::ReadDouble( double& value, const Byte*& source )
{
	CopyAndIncrementSource( &value, source, DOUBLE_SIZE );
}

void SerializationUtility::ReadBool( bool& value, const Byte*& source )
{
	CopyAndIncrementSource( &value, source, BOOL_SIZE );
}

void SerializationUtility::ReadVec2( glm::vec2&	value, const Byte*& source )
{
	CopyAndIncrementSource( &value.x, source, FLOAT_SIZE );
	CopyAndIncrementSource( &value.y, source, FLOAT_SIZE );
}

void SerializationUtility::ReadVec3( glm::vec3&	value, const Byte*& source )
{
	CopyAndIncrementSource( &value.x, source, FLOAT_SIZE );
	CopyAndIncrementSource( &value.y, source, FLOAT_SIZE );
	CopyAndIncrementSource( &value.z, source, FLOAT_SIZE );
}

void SerializationUtility::ReadQuaternion( glm::quat&	value, const Byte*& source )
{
	CopyAndIncrementSource( &value.w, source, FLOAT_SIZE );
	CopyAndIncrementSource( &value.x, source, FLOAT_SIZE );
	CopyAndIncrementSource( &value.y, source, FLOAT_SIZE );
	CopyAndIncrementSource( &value.z, source, FLOAT_SIZE );
}

void SerializationUtility::ReadString( rString&	value, const Byte*& source ) // TODODB: Optimize this one~ (Try to avoid unnecessary memory allocation)
{
	uint32_t stringLength;
	CopyAndIncrementSource( &stringLength, source, INT_32_SIZE );

	char* buffer = tAlloc( char, stringLength + 1 ); // +1 for the null terminator //TODODB: See if we really need to place this null terminator or if it will be placed there by the std::string ctor
	CopyAndIncrementSource( buffer, source, stringLength );
	buffer[stringLength] = '\0';

	value = rString( buffer );
	tFree( buffer );
}