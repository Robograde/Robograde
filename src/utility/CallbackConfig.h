/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include "Config.h"
#include <functional>
#include "UtilityLibraryDefine.h"

class CallbackConfig;

typedef std::function<void( CallbackConfig* cfg )> CallbackConfigFunction;
typedef unsigned int CallbackConfigRegisterID;

class CallbackConfig : public Config
{
public:
	UTILITY_API CallbackConfig( const rString& configPath );
	UTILITY_API ~CallbackConfig( );
	UTILITY_API bool ReloadFile( );
	UTILITY_API bool ReadFile( const rString& filePath = "" ) override;
	UTILITY_API bool SaveFile( const rString& filePath = "" ) override;

	UTILITY_API CallbackConfigRegisterID RegisterInterest( CallbackConfigFunction callbackFunction );
	UTILITY_API void UnregisterInterest( CallbackConfigRegisterID id );

private:
	void TriggerCallbacks( );

	rString m_Path = "";
	rMap<CallbackConfigRegisterID, CallbackConfigFunction> m_Callbacks;
	CallbackConfigRegisterID m_NextID = 0;
};
