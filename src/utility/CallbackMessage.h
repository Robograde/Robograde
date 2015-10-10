/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include "UtilityLibraryDefine.h"

#define MESSAGE_REGISTER_ID_NONE -1
typedef int CallbackMessageRegisterID;
typedef int CallbackMessageType;

class CallbackMessage
{
public:
	UTILITY_API CallbackMessage( CallbackMessageType msgType );
	UTILITY_API ~CallbackMessage() {};

	CallbackMessageType		m_Type;
	int 					m_From;
	int 					m_To;
};

template <typename T>
class DataCallbackMessage : public CallbackMessage
{
public:
	 DataCallbackMessage( CallbackMessageType msgType, T data ) : CallbackMessage( msgType ), m_Data( data ) {};
	 ~DataCallbackMessage() {};

	T			m_Data;
};