/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once

#include "MessageLibraryDefine.h"

#define MESSAGE_TYPE_ENUM_UNDELYING_TYPE int

struct Message
{
public:
	MESSAGING_API Message() {}
	MESSAGING_API Message( MESSAGE_TYPE_ENUM_UNDELYING_TYPE type, bool isSimulation = true ) : Type( type ), IsSimulation( isSimulation ) {}
	MESSAGING_API virtual ~Message() {};

	MESSAGING_API virtual Message*		Clone()								const	= 0;
	MESSAGING_API virtual unsigned int	GetSerializationSize()				const;
	MESSAGING_API virtual void			Serialize( char*& buffer )			const;
	MESSAGING_API virtual void			Deserialize( const char*& buffer );
	
	MESSAGE_TYPE_ENUM_UNDELYING_TYPE	Type				= 0;
	bool								IsSimulation		= true;

	bool								CreatedFromPacket	= false;
};
