/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "ReplayUtility.h"
#include "../../input/GameMessages.h"

using namespace SerializationUtility;
ReplayFrame::ReplayFrame( )
{}

ReplayFrame::~ReplayFrame( )
{
	for ( int i = 0; i < Messages.size( ); ++i )
		tDelete( Messages[i] );
}

ReplayFrame::ReplayFrame( const ReplayFrame& rhs )
{
	this->ID = rhs.ID;
	this->Hash = rhs.Hash;
	this->RandomCounter = rhs.RandomCounter;
	for ( int i = 0; i < rhs.Messages.size( ); ++i )
		this->Messages.push_back( rhs.Messages[i]->Clone( ) );
}

unsigned int ReplayFrame::GetSerializationSize( ) const
{
	unsigned int size = static_cast<unsigned int>(sizeof(unsigned int)* 4); // ID, Hash, RandomCounter, and vector size
	for ( auto& message : Messages )
		size += message->GetSerializationSize();

	return size;
}

void ReplayFrame::Serialize( Byte*& buffer ) const
{
	// Serialize ID and Hash
	CopyAndIncrementDestination( buffer, &ID, sizeof(unsigned int) );
	CopyAndIncrementDestination( buffer, &Hash, sizeof(unsigned int) );
	CopyAndIncrementDestination( buffer, &RandomCounter, sizeof(unsigned int) );

	// Serialize all messages for the current frame
	unsigned int recordedMessagesCount = static_cast<unsigned int>(Messages.size( ));
	CopyAndIncrementDestination( buffer, &recordedMessagesCount, sizeof(unsigned int) );
	for ( auto& message : Messages )
		message->Serialize( buffer );
}

void ReplayFrame::Deserialize( const Byte*& buffer )
{
	// Deserialize ID and Hash
	CopyAndIncrementSource( &ID, buffer, sizeof(unsigned int) );
	CopyAndIncrementSource( &Hash, buffer, sizeof(unsigned int) );
	CopyAndIncrementSource( &RandomCounter, buffer, sizeof(unsigned int) );

	// Deserialize all messages for the current frame
	unsigned int recordedMessagesCount;
	CopyAndIncrementSource( &recordedMessagesCount, buffer, sizeof(unsigned int) );
	Messages.reserve( recordedMessagesCount );
	for ( unsigned int i = 0; i < recordedMessagesCount; ++i )
	{
		MessageTypes::MessageType messageType;
		memcpy( &messageType, buffer, sizeof( MESSAGE_TYPE_ENUM_UNDELYING_TYPE ) ); // Deserialize the message type

		Message* message = Messages::GetDefaultMessage( messageType );	// Use the type to run the specific message types constructor
		message->Deserialize( buffer );	// Read the specific data for this message
		Messages.push_back( message );	// Add it to this frames recorded messages
	}
}