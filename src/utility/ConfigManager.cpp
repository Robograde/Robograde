/**************************************************
Zlib Copyright 2015 Johan Melin
***************************************************/

#include "ConfigManager.h"
#include "Logger.h"

ConfigManager& ConfigManager::GetInstance( )
{
	static ConfigManager instance;
	return instance;
}

ConfigManager::ConfigManager( )
{
}

ConfigManager::~ConfigManager( )
{
	for( auto& config : m_Configs )
	{
		pDelete( config.second );
	}
	m_Configs.clear( );
}

CallbackConfig* ConfigManager::GetConfig( const rString& path )
{
	rString pathWithPrefix = m_ConfigFolderPath + path;

	auto config = m_Configs.find( pathWithPrefix );
	if( config == m_Configs.end( ) )
	{
		CallbackConfig* newConfig = pNew( CallbackConfig, pathWithPrefix );

		if( newConfig->ReadFile( ) )
		{
			m_Configs.emplace( pathWithPrefix, newConfig );
		}
		else
		{
			Logger::Log( "Failed to read configuration file: " + pathWithPrefix + ". Default values will be used", "ConfigManager",
						 LogSeverity::INFO_MSG );
			// Delete dirty config
			pDelete( newConfig );
			// Return a fresh new one
			CallbackConfig* freshConfig = pNew( CallbackConfig, pathWithPrefix );
			m_Configs.emplace( pathWithPrefix, freshConfig );
			return freshConfig;
		}

		return newConfig;
	}
	else
	{
		return config->second;
	}
}

void ConfigManager::SaveDirty( )
{
	for ( auto& config : m_Configs )
	{
		if ( config.second->IsDirty( ) )
		{
			Logger::Log( "Saving dirty config file: " + config.first, "ConfigManager", LogSeverity::INFO_MSG );
			config.second->SaveFile( );
		}
	}
}

const rString& ConfigManager::GetConfigFolderPath( ) const
{
	return m_ConfigFolderPath;
}

void ConfigManager::SetConfigFolderPath( const rString& path )
{
	m_ConfigFolderPath = path;
}
