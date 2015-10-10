/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "CallbackPump.h"

CallbackPump& CallbackPump::GetInstance()
{
	static CallbackPump msgPump;
	return msgPump;
}

CallbackPump::CallbackPump ( )
{

}

CallbackPump::~CallbackPump( )
{
	for ( CallbackMessageTypeMap::iterator it = m_CallbackMessageTypes.begin( ); it != m_CallbackMessageTypes.end( ); ++it )
	{
		pDelete( it->second );
	}
}

void CallbackPump::Update()
{
	if ( m_CallbackMessageQueue.size() == 0 )
	{
		return;
	}

	do
	{
		CallbackMessage* msg = m_CallbackMessageQueue.back();
		m_CallbackMessageQueue.pop_back();

		auto it = m_CallbackMessageTypes.find( msg->m_Type );

		if ( it != m_CallbackMessageTypes.end() )
		{
			it->second->TriggerCallbacks( msg );
		}
		tDelete( msg );
	}
	while ( m_CallbackMessageQueue.size() > 0 );
}

CallbackMessageResult CallbackPump::AddCallbackMessageToSystem( CallbackMessageType msgType )
{
	auto it = m_CallbackMessageTypes.find( msgType );

	if ( it != m_CallbackMessageTypes.end() )
	{
		return CallbackMessageResult::CallbackMessageTypeAlreadyExist;
	}
	else
	{
		m_CallbackMessageTypes.emplace( msgType, pNew( CallbackCollection ) );
		return CallbackMessageResult::OK;
	}
}

CallbackMessageResultData CallbackPump::RegisterForCallbackMessage( CallbackMessageType msgType, CallbackMessageFunction callbackFunction )
{
	auto it = m_CallbackMessageTypes.find( msgType );

	CallbackMessageResultData mrd;
	mrd.CallbackMessageRegisterID = MESSAGE_REGISTER_ID_NONE;
	if ( it == m_CallbackMessageTypes.end() )
	{
		mrd.CallbackMessageResult = CallbackMessageResult::CallbackMessageTypeNotInSystem;
	}
	else
	{
		mrd.CallbackMessageRegisterID = it->second->Register( callbackFunction );
		mrd.CallbackMessageResult = CallbackMessageResult::OK;
	}
	return mrd;
}

CallbackMessageResult CallbackPump::UnregisterForCallbackMessage( CallbackMessageType msgType, int objectID )
{
	auto it = m_CallbackMessageTypes.find( msgType );

	if ( it == m_CallbackMessageTypes.end() )
	{
		return CallbackMessageResult::CallbackMessageTypeNotInSystem;
	}
	else
	{
		it->second->Unregister( objectID );
		return CallbackMessageResult::OK;
	}
}

void CallbackPump::SendCallbackMessagePlease( CallbackMessage* msg )
{
	m_CallbackMessageQueue.push_back( msg );
}
