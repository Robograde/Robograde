/**************************************************
Copyright 2015 Ola Enberg
***************************************************/

#include "SSStartupScript.h"

#include <script/ScriptEngine.h>

SSStartupScript& SSStartupScript::GetInstance( )
{
    static SSStartupScript instance;
    return instance;
}

void SSStartupScript::Startup( )
{
	g_Script.Perform( "dofile( SRC_DIR .. \"personal/Startup.lua\" )" );
}