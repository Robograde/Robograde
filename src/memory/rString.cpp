/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#include "rString.h"

rString rToString( int value )
{
	char buffer[64];
	sprintf( buffer, "%d", value );
	return rString( buffer );
}

rString rToString( long value )
{
	char buffer[64];
	sprintf( buffer, "%ld", value );
	return rString( buffer );
}

rString rToString( long long value )
{
	char buffer[64];
	sprintf( buffer, "%lld", value );
	return rString( buffer );
}

rString rToString( unsigned value )
{
	char buffer[64];
	sprintf( buffer, "%u", value );
	return rString( buffer );
}

rString rToString( unsigned long value )
{
	char buffer[64];
	sprintf( buffer, "%lu", value );
	return rString( buffer );
}

rString rToString( unsigned long long value )
{
	char buffer[64];
	sprintf( buffer, "%llu", value );
	return rString( buffer );
}

rString rToString( float value )
{
	char buffer[64];
	sprintf( buffer, "%f", value );
	return rString( buffer );
}

rString rToString( double value )
{
	char buffer[64];
	sprintf( buffer, "%lf", value );
	return rString( buffer );
}

rString rToString( long double value )
{
	char buffer[64];
	sprintf( buffer, "%Lf", value );
	return rString( buffer );
}