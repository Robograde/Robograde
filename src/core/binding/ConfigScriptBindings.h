/**************************************************
2015 Johan Melin
***************************************************/

#pragma once

#include <utility/ConfigManager.h>
#include <utility/Logger.h>
#include <script/ScriptEngine.h>


namespace ConfigScriptBinder
{
void BindConfigScripts( )
{
	g_Script.Register( "ReloadConfig", []( IScriptEngine* scriptEngine ) -> int
	{
		rString toLoad = scriptEngine->PopString( );
		CallbackConfig* cfg = g_ConfigManager.GetConfig( toLoad );
		if( cfg )
		{
			cfg->ReloadFile( );
		}
		else
		{
			Logger::Log( "Failed to reload config file: " + toLoad, "Config", LogSeverity::WARNING_MSG );
		}
		return 0;
	} );
}
}
