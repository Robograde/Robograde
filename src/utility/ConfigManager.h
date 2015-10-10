/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#pragma once
#include "memory/Alloc.h"
#include "CallbackConfig.h"

#define g_ConfigManager ConfigManager::GetInstance()

class ConfigManager
{
public:
	UTILITY_API static ConfigManager& GetInstance();

	UTILITY_API ConfigManager();
	UTILITY_API ~ConfigManager();
	UTILITY_API CallbackConfig* GetConfig( const rString& path );
	UTILITY_API void			SaveDirty( );
	UTILITY_API const rString&	GetConfigFolderPath( ) const;
	UTILITY_API void 			SetConfigFolderPath( const rString& path );

private:
	rMap<rString, CallbackConfig*> 	m_Configs;
	rString 						m_ConfigFolderPath = "";
};
