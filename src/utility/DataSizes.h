/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once

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

	const short	UNSIGNED_INT_SIZE = sizeof( unsigned int );
	static_assert(UNSIGNED_INT_SIZE == 4, "Sizeof unsigned int must be 4");

	const short FLOAT_SIZE = sizeof( float );
	static_assert( FLOAT_SIZE == 4, "Sizeof float must be 4" );

	const short DOUBLE_SIZE = sizeof( double );
	static_assert( DOUBLE_SIZE == 8, "Sizeof double must be 8");

	const short	BOOL_SIZE	= sizeof( bool );
	static_assert( BOOL_SIZE == 1, "Sizeof bool must be 2" );
}