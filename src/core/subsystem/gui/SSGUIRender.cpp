/**************************************************
2015 Isak Almgren
***************************************************/

#include "SSGUIRender.h"
#include "../../utility/GameData.h"

#include <input/Input.h>
#include <SDL2/SDL.h>



SSGUIRender&SSGUIRender::GetInstance()
{
	static SSGUIRender instance;
	return instance;
}

void SSGUIRender::Startup()
{
	g_GUI.InitializeRoot( g_GameData.GetWindowWidth(), g_GameData.GetWindowHeight() );
}

void SSGUIRender::Shutdown()
{
}

void SSGUIRender::UpdateUserLayer( const float deltaTime )
{
	g_GUI.Render( g_GameData.GetWindowWidth(), g_GameData.GetWindowHeight() );
}