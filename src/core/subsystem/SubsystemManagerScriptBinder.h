/**************************************************
2015 Johan Melin
***************************************************/

#pragma once

#include "../CompileFlags.h"
#include "SubsystemManager.h"
#include <script/ScriptEngine.h>

static void BindSubsystemManagerToLua( )
{
	g_Script.Register( "GE_PrintStartupOrder",			[] ( IScriptEngine* ) -> int { g_SubsystemManager.PrintStartupOrder();			return 0; } );
	g_Script.Register( "GE_PrintUpdateOrder",			[] ( IScriptEngine* ) -> int { g_SubsystemManager.PrintUpdateOrder();			return 0; } );
	g_Script.Register( "GE_PrintShutdownOrder",			[] ( IScriptEngine* ) -> int { g_SubsystemManager.PrintStartupOrder();			return 0; } );
	g_Script.Register( "GE_PrintStartedSubsystems",		[] ( IScriptEngine* ) -> int { g_SubsystemManager.PrintStartedSubsystems();		return 0; } );
	g_Script.Register( "GE_PrintUnstartedSubsystems",	[] ( IScriptEngine* ) -> int { g_SubsystemManager.PrintUnstartedSubsystems();	return 0; } );

	DEV(
			g_Script.Register( "GE_RestartSubsystem",		[](IScriptEngine* scriptEngine) -> int { g_SubsystemManager.RestartSubsystem(scriptEngine->PopString());		return 0; } );
			g_Script.Register( "GE_Pause",					[](IScriptEngine* scriptEngine) -> int { g_SubsystemManager.PauseSubsystem(scriptEngine->PopString());			return 0; } );
			g_Script.Register( "GE_Unpause",				[](IScriptEngine* scriptEngine) -> int { g_SubsystemManager.UnpauseSubsystem(scriptEngine->PopString());		return 0; } );
	);
}
