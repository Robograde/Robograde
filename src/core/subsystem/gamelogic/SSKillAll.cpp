/**************************************************
Copyright 2015 Johan Melin & Viktor Kelkkanen
***************************************************/

#include "SSKillAll.h"
#include "../../CompileFlags.h"

#include <script/ScriptEngine.h>
#include "../../datadriven/EntityManager.h"

SSKillAll& SSKillAll::GetInstance( )
{
	static SSKillAll instance;
	return instance;
}

void SSKillAll::Startup()
{
	DEV(
			g_Script.Register( "GE_CrashToDesktop", [] ( IScriptEngine* ) -> int { g_SSKillAll.CrashToDesktop(); return 0; } );
	   );
}

void SSKillAll::Shutdown( )
{
	for ( Entity i = 0; i < COMPONENTS_MAX_DENSE; ++i )
	{
		g_EntityManager.RemoveEntity( i );
	}

	g_EntityManager.Reset();
}

void SSKillAll::CrashToDesktop()
{
	abort();
}
