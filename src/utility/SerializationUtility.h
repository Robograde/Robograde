/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "PlatformDefinitions.h"
#include <stdint.h>
#include <glm/glm.hpp>
#include <memory/rString.h>

#if PLATFORM == PLATFORM_LINUX
#include <stdlib.h> // For size_t
#endif
#include "UtilityLibraryDefine.h"

namespace SerializationUtility {
	typedef char Byte;
	static_assert( sizeof( Byte ) == 1, "Byte must have a size of 1." );

	UTILITY_API void CopyAndIncrementDestination( Byte*& destination, const void* const source, const size_t length );
	UTILITY_API void CopyAndIncrementSource( void* const destination, const Byte*& source, const size_t length );

	UTILITY_API void WriteInt16(				int16_t			value,		Byte*& destination );
	UTILITY_API void WriteInt32(				int32_t			value,		Byte*& destination );
	UTILITY_API void WriteInt64(				int64_t			value,		Byte*& destination );
	UTILITY_API void WriteUint16(				uint16_t		value,		Byte*& destination );
	UTILITY_API void WriteUint32(				uint32_t		value,		Byte*& destination );
	UTILITY_API void WriteUint64(				uint64_t		value,		Byte*& destination );
	UTILITY_API void WriteFloat(				float			value,		Byte*& destination );
	UTILITY_API void WriteDouble(				double			value,		Byte*& destination );
	UTILITY_API void WriteBool(					bool			value,		Byte*& destination );
	UTILITY_API void WriteVec2(			const	glm::vec2&		value,		Byte*& destination );
	UTILITY_API void WriteVec3(			const	glm::vec3&		value,		Byte*& destination );
	UTILITY_API void WriteQuaternion(	const	glm::quat&		value,		Byte*& destination );
	UTILITY_API void WriteString(		const	rString&		value,		Byte*& destination );

	UTILITY_API void ReadInt16(					int16_t&		value,		const Byte*& source );
	UTILITY_API void ReadInt32(					int32_t&		value,		const Byte*& source );
	UTILITY_API void ReadInt64(					int64_t&		value,		const Byte*& source );
	UTILITY_API void ReadUInt16(				uint16_t&		value,		const Byte*& source );
	UTILITY_API void ReadUint32(				uint32_t&		value,		const Byte*& source );
	UTILITY_API void ReadUint64(				uint64_t&		value,		const Byte*& source );
	UTILITY_API void ReadFloat(					float&			value,		const Byte*& source );
	UTILITY_API void ReadDouble(				double&			value,		const Byte*& source );
	UTILITY_API void ReadBool(					bool&			value,		const Byte*& source );
	UTILITY_API void ReadVec2(					glm::vec2&		value,		const Byte*& source );
	UTILITY_API void ReadVec3(					glm::vec3&		value,		const Byte*& source );
	UTILITY_API void ReadQuaternion(			glm::quat&		value,		const Byte*& source );
	UTILITY_API void ReadString(				rString&		value,		const Byte*& source );
}