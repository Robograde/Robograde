/**************************************************
2015 Isak Almgren
***************************************************/

#include "SSGUIUpdate.h"
#include <input/Input.h>
#include <SDL2/SDL.h>
#include "../../input/GameMessages.h"
#include "../input/SSPicking.h"
#include "../utility/SSMail.h"


SSGUIUpdate& SSGUIUpdate::GetInstance()
{
	static SSGUIUpdate instance;
	return instance;
}

void SSGUIUpdate::Startup()
{
	glm::ivec2 winSize = g_GUI.GetWindowSize( "RootWindow" );
	g_GUI.AddWindow( "DebugWindow", GUI::Rectangle( 0, 0, winSize.x, winSize.y ), "RootWindow" );
	g_GUI.OpenWindow( "DebugWindow" );
	
	g_GUI.AddWindow( "InGameWindow", GUI::Rectangle( 0, 0, winSize.x, winSize.y ), "RootWindow" );
	g_GUI.OpenWindow( "InGameWindow" );
	
	
	g_Script.Register( "PlayButtonSound", [this](IScriptEngine* scriptEngine) -> int 
	{
		SFXButtonClickMessage message;
		message.ButtonName = scriptEngine->PopString();	
		g_SSMail.PushToNextFrame( message );
		return 0; 
	} );


	

}

void SSGUIUpdate::Shutdown()
{
	g_GUI.DeleteObject( "DebugWindow" );
	g_GUI.DeleteObject( "InGameWindow" );
}

void SSGUIUpdate::UpdateUserLayer( const float deltaTime )
{
	if( g_SSPicking.IsBoxSelecting() )
		g_GUI.ToggleInput( false );
	else
		g_GUI.ToggleInput( true );
	
	
	g_GUI.Update( deltaTime );
}