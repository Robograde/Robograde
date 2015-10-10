/**************************************************
2015 Isak Almgren
***************************************************/

#include "SSGraphicsSettings.h"
#include <utility/ConfigManager.h>
#include <gfx/GraphicsEngine.h>
#include "../../utility/GameData.h"
#include "../gamelogic/SSCamera.h"
#include "../SubsystemManager.h"
#include <gui/GUIEngine.h>
#include <core\utility\GameModeSelector.h>

SSGraphicsSettings& SSGraphicsSettings::GetInstance( )
{
	static SSGraphicsSettings instance;
	return instance;
}

void SSGraphicsSettings::Startup( )
{
	CallbackConfig* cfg = g_ConfigManager.GetConfig( m_GraphicsConfigPath );
	assert( cfg );
	m_CallbackConfigRegisterID = cfg->RegisterInterest( std::bind( &SSGraphicsSettings::GraphicsConfigChangeCallback, this, std::placeholders::_1 ) );
}

void SSGraphicsSettings::Shutdown( )
{
	CallbackConfig* cfg = g_ConfigManager.GetConfig( m_GraphicsConfigPath );
	assert( cfg );
	cfg->UnregisterInterest( m_CallbackConfigRegisterID );
}

void SSGraphicsSettings::GraphicsConfigChangeCallback( CallbackConfig* cfg )
{
	bool useFogOfWar = !(g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Editor);

	g_GameData.ReadGraphicsConfig( );
	gfx::g_GFXEngine.ReinitializeWindow( g_GameData.GetWindowWidth( ), g_GameData.GetWindowHeight( ), g_GameData.m_MSAA,
		g_GameData.m_Fullscreen, g_GameData.m_Vsync, useFogOfWar);

	g_SSCamera.SetWindowSize( g_GameData.GetWindowWidth( ), g_GameData.GetWindowHeight( ) );
	g_GUI.GetWindow( "RootWindow" 	)->SetSize( g_GameData.GetWindowWidth( ), g_GameData.GetWindowHeight( ) );
	g_GUI.GetWindow( "DebugWindow" 	)->SetSize( g_GameData.GetWindowWidth( ), g_GameData.GetWindowHeight( ) );
	g_GUI.GetWindow( "InGameWindow" )->SetSize( g_GameData.GetWindowWidth( ), g_GameData.GetWindowHeight( ) );
	g_SubsystemManager.RestartSubsystem( "MiniMap" );		  	// Window
	g_SubsystemManager.RestartSubsystem( "UpgradeWindow" );   	// Window
	g_SubsystemManager.RestartSubsystem( "Cursor" );		  	// Lock mouse
	g_SubsystemManager.RestartSubsystem( "FrameCounter" );		// Window
	g_SubsystemManager.RestartSubsystem( "SquadControlGUI" ); 	// Window
	g_SubsystemManager.RestartSubsystem( "Console" ); 			// Window
}
