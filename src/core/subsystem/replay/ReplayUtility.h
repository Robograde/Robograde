/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include <utility/SerializationUtility.h>
#include <messaging/Message.h>

#define REPLAY_DEBUG 0

struct ReplayFrame
{
	ReplayFrame( );
	~ReplayFrame( );
	ReplayFrame( const ReplayFrame& rhs );

	unsigned int GetSerializationSize( ) const;
	void Serialize( SerializationUtility::Byte*& buffer ) const;
	void Deserialize( const SerializationUtility::Byte*& buffer );

	unsigned int						ID = 0;
	unsigned int						Hash = 0;
	unsigned int						RandomCounter = 0;
	rVector<const Message*>				Messages;
};

namespace ReplayUtility
{
	const std::string	DEFAULT_REPLAY_FILE_PATH	= "../../../replay/replay";
	const std::string	REPLAY_FILE_EXTENSION		= ".ror";
	const unsigned int	MAX_SAVED_REPLAYS			= 500;
}