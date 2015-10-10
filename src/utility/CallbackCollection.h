/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include <functional>

#include "memory/Alloc.h"
#include "CallbackMessage.h"
#include "UtilityLibraryDefine.h"

typedef std::function<void( CallbackMessage* msg )> CallbackMessageFunction;
typedef rMap<int, CallbackMessageFunction> CallbackMessageMap;

class CallbackCollection
{
public:
	UTILITY_API CallbackCollection();
	UTILITY_API ~CallbackCollection();

	UTILITY_API CallbackMessageRegisterID	Register( CallbackMessageFunction callbackFunction );
	UTILITY_API void				 		Unregister( CallbackMessageRegisterID objectID );
	UTILITY_API void						TriggerCallbacks( CallbackMessage* msg );

	CallbackMessageMap	m_Callbacks;
	int 				m_CallbackMessageIDNew = 0;
};
