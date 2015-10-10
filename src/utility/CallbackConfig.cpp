/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "CallbackConfig.h"
#include "Logger.h"

CallbackConfig::CallbackConfig( const rString& configPath )
	: m_Path( configPath )
{

}

CallbackConfig::~CallbackConfig()
{
}

bool CallbackConfig::ReloadFile( )
{
	bool success = ReadFile();
	if ( success == false )
		return success;
	else
		TriggerCallbacks();
	return true;
}

bool CallbackConfig::ReadFile( const rString& filePath )
{
	return Config::ReadFile( m_Path );
}

bool CallbackConfig::SaveFile( const rString& filePath )
{
	bool success = Config::SaveFile( m_Path );
	if ( success == false )
		return success;
	else
	{
		TriggerCallbacks( );
	}
	return success;
}

CallbackConfigRegisterID CallbackConfig::RegisterInterest( CallbackConfigFunction callbackFunction ) 
{
	CallbackConfigRegisterID id = m_NextID++;
	m_Callbacks.emplace( id, callbackFunction );
	return id;
}

void CallbackConfig::UnregisterInterest( CallbackConfigRegisterID id )
{
	auto it = m_Callbacks.find( id );
	if ( it != m_Callbacks.end() )
		m_Callbacks.erase( it );
	else
		Logger::Log( "Failed to unregister for config change, register id: " + rToString( id ) + " does not exist", "CallbackConfig", LogSeverity::WARNING_MSG );
}

void CallbackConfig::TriggerCallbacks( )
{
	for ( auto& callback : m_Callbacks )
	{
		callback.second( this );
	}
}
