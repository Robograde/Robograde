/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <list>
#include <map>
#include "memory/Alloc.h"
#include "CallbackMessage.h"
#include "CallbackCollection.h"
#include "UtilityLibraryDefine.h"

#define g_CallbackPump CallbackPump::GetInstance()

typedef rVector<CallbackMessage*> CallbackMessageList;
typedef rMap<CallbackMessageType, CallbackCollection*> CallbackMessageTypeMap;

enum class CallbackMessageResult
{
	OK,
	AlreadyRegistered,
	CallbackMessageTypeNotInSystem,
	CallbackMessageTypeAlreadyExist
};

struct CallbackMessageResultData
{
	CallbackMessageResult 		CallbackMessageResult;
	CallbackMessageRegisterID	CallbackMessageRegisterID;
};

class CallbackPump
{
public:
	UTILITY_API static CallbackPump&	GetInstance();
	UTILITY_API CallbackPump();
	UTILITY_API ~CallbackPump();

	UTILITY_API void						Update();
	UTILITY_API CallbackMessageResult		AddCallbackMessageToSystem( CallbackMessageType msgType );
	UTILITY_API CallbackMessageResultData	RegisterForCallbackMessage( CallbackMessageType msgType, CallbackMessageFunction callbackFunction );
	UTILITY_API CallbackMessageResult		UnregisterForCallbackMessage( CallbackMessageType msgType, int registerID );

	// The message will be deleted next update. Don't delete it yourself.
	UTILITY_API void						SendCallbackMessagePlease( CallbackMessage* msg );

private:
	CallbackMessageList			m_CallbackMessageQueue;
	CallbackMessageTypeMap 		m_CallbackMessageTypes;
};
