/**************************************************
Zlib Copyright 2015 David Pejtersen
***************************************************/

#include "AllocationPatterns.h"

#pragma region pString
pString pToString( int value )
{
	char buffer[64];
	sprintf( buffer, "%d", value );
	return pString( buffer );
}

pString pToString( long value )
{
	char buffer[64];
	sprintf( buffer, "%ld", value );
	return pString( buffer );
}

pString pToString( long long value )
{
	char buffer[64];
	sprintf( buffer, "%lld", value );
	return pString( buffer );
}

pString pToString( unsigned value )
{
	char buffer[64];
	sprintf( buffer, "%u", value );
	return pString( buffer );
}

pString pToString( unsigned long value )
{
	char buffer[64];
	sprintf( buffer, "%lu", value );
	return pString( buffer );
}

pString pToString( unsigned long long value )
{
	char buffer[64];
	sprintf( buffer, "%llu", value );
	return pString( buffer );
}

pString pToString( float value )
{
	char buffer[64];
	sprintf( buffer, "%f", value );
	return pString( buffer );
}

pString pToString( double value )
{
	char buffer[64];
	sprintf( buffer, "%lf", value );
	return pString( buffer );
}

pString pToString( long double value )
{
	char buffer[64];
	sprintf( buffer, "%Lf", value );
	return pString( buffer );
}
#pragma endregion
#pragma region tString
tString tToString( int value )
{
	char buffer[64];
	sprintf( buffer, "%d", value );
	return tString( buffer );
}

tString tToString( long value )
{
	char buffer[64];
	sprintf( buffer, "%ld", value );
	return tString( buffer );
}

tString tToString( long long value )
{
	char buffer[64];
	sprintf( buffer, "%lld", value );
	return tString( buffer );
}

tString tToString( unsigned value )
{
	char buffer[64];
	sprintf( buffer, "%u", value );
	return tString( buffer );
}

tString tToString( unsigned long value )
{
	char buffer[64];
	sprintf( buffer, "%lu", value );
	return tString( buffer );
}

tString tToString( unsigned long long value )
{
	char buffer[64];
	sprintf( buffer, "%llu", value );
	return tString( buffer );
}

tString tToString( float value )
{
	char buffer[64];
	sprintf( buffer, "%f", value );
	return tString( buffer );
}

tString tToString( double value )
{
	char buffer[64];
	sprintf( buffer, "%lf", value );
	return tString( buffer );
}

tString tToString( long double value )
{
	char buffer[64];
	sprintf( buffer, "%Lf", value );
	return tString( buffer );
}
#pragma endregion
#pragma region fString
fString fToString( int value )
{
	char buffer[64];
	sprintf( buffer, "%d", value );
	return fString( buffer );
}

fString fToString( long value )
{
	char buffer[64];
	sprintf( buffer, "%ld", value );
	return fString( buffer );
}

fString fToString( long long value )
{
	char buffer[64];
	sprintf( buffer, "%lld", value );
	return fString( buffer );
}

fString fToString( unsigned value )
{
	char buffer[64];
	sprintf( buffer, "%u", value );
	return fString( buffer );
}

fString fToString( unsigned long value )
{
	char buffer[64];
	sprintf( buffer, "%lu", value );
	return fString( buffer );
}

fString fToString( unsigned long long value )
{
	char buffer[64];
	sprintf( buffer, "%llu", value );
	return fString( buffer );
}

fString fToString( float value )
{
	char buffer[64];
	sprintf( buffer, "%f", value );
	return fString( buffer );
}

fString fToString( double value )
{
	char buffer[64];
	sprintf( buffer, "%lf", value );
	return fString( buffer );
}

fString fToString( long double value )
{
	char buffer[64];
	sprintf( buffer, "%Lf", value );
	return fString( buffer );
}
#pragma endregion