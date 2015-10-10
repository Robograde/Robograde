/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "Message.h"
#include <utility/SerializationUtility.h>

unsigned int Message::GetSerializationSize( ) const
{
	return sizeof( MESSAGE_TYPE_ENUM_UNDELYING_TYPE ) + sizeof( bool );
}

void Message::Serialize( char*& buffer ) const
{
	SerializationUtility::CopyAndIncrementDestination( buffer, &Type, sizeof( MESSAGE_TYPE_ENUM_UNDELYING_TYPE ) );
	SerializationUtility::CopyAndIncrementDestination( buffer, &IsSimulation, sizeof( bool ) );
}

void Message::Deserialize( const char*& buffer )
{
	SerializationUtility::CopyAndIncrementSource( &Type, buffer, sizeof( MESSAGE_TYPE_ENUM_UNDELYING_TYPE ) );
	SerializationUtility::CopyAndIncrementSource( &IsSimulation, buffer, sizeof( bool ) );
}