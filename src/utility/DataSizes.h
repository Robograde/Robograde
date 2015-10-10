/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once

#include <stdint.h>
#include <glm/gtc/quaternion.hpp>
					// TODODB: Remove the use of all unsigned sizes. Unsigned is guaranteed to be of the the same size as signed
namespace DataSizes // TODODB: Use strictly sized variables in messaging
{
	const short CHAR_SIZE = sizeof( char );
	static_assert( CHAR_SIZE == 1, "Sizeof char must be 1" );

	const short UNSIGNED_CHAR_SIZE = sizeof( unsigned char );
	static_assert( UNSIGNED_CHAR_SIZE == 1, "Sizeof unsigned char must be 1" );

	const short SHORT_SIZE = sizeof( short );
	static_assert( SHORT_SIZE == 2, "Sizeof short must be 2" );

	const short UNSIGNED_SHORT_SIZE = sizeof( unsigned short );
	static_assert( UNSIGNED_SHORT_SIZE == 2, "Sizeof unsigned short must be 2");

	const short	INT_SIZE = sizeof( int );
	static_assert( INT_SIZE == 4, "Sizeof int must be 4" );

	const short INT_16_SIZE = sizeof( INT_16_SIZE );
	static_assert( INT_16_SIZE == 2, "Sizeof int16 must be 2" );
	
	const short	INT_32_SIZE = sizeof( int32_t );
	static_assert( INT_32_SIZE == 4, "Sizeof int32 must be 4" );

	const short	INT_64_SIZE = sizeof( int64_t );
	static_assert( INT_64_SIZE == 8, "Sizeof int64 must be 8" );

	const short	UNSIGNED_INT_SIZE = sizeof( unsigned int );
	static_assert( UNSIGNED_INT_SIZE == 4, "Sizeof unsigned int must be 4");

	const short FLOAT_SIZE = sizeof( float );
	static_assert( FLOAT_SIZE == 4, "Sizeof float must be 4" );

	const short DOUBLE_SIZE = sizeof( double );
	static_assert( DOUBLE_SIZE == 8, "Sizeof double must be 8");

	const short	BOOL_SIZE = sizeof( bool );
	static_assert( BOOL_SIZE == 1, "Sizeof bool must be 2" );

	const short VEC2_SIZE = sizeof( glm::vec2 );
	static_assert( VEC2_SIZE == FLOAT_SIZE * 2, "Sizeof glm::vec2 must be float * 2" );

	const short VEC3_SIZE = sizeof( glm::vec3 );
	static_assert( VEC3_SIZE == FLOAT_SIZE * 3, "Sizeof glm::vec3 must be float * 3" );

	const short QUATERNION_SIZE = sizeof( glm::quat );
	static_assert( QUATERNION_SIZE == FLOAT_SIZE * 4, "Sizeof glm::quat must be float * 4" );
}