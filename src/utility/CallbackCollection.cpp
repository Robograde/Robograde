/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "CallbackCollection.h"

CallbackCollection::CallbackCollection()
{

}

CallbackCollection::~CallbackCollection()
{

}

CallbackMessageRegisterID CallbackCollection::Register( CallbackMessageFunction callbackFunction )
{
	m_Callbacks.emplace( ++m_CallbackMessageIDNew, callbackFunction );
	return m_CallbackMessageIDNew;
}

void CallbackCollection::Unregister( CallbackMessageRegisterID objectID )
{
	auto cit = m_Callbacks.find( objectID );

	if ( cit != m_Callbacks.cend() )
	{
		m_Callbacks.erase( cit );
	}
}

void CallbackCollection::TriggerCallbacks( CallbackMessage* msg )
{
	for ( auto it = m_Callbacks.begin(); it != m_Callbacks.end(); ++it )
	{
		it->second( msg );
	}
}
