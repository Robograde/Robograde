/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "SSMainMenu.h"
#include <utility/Logger.h>
#include <SDL2/SDL.h>
#include "../input/SSKeyBinding.h"
#include "../audio/SSMusicManager.h"
#include "SSReplaySelectMenu.h"

SSMainMenu& SSMainMenu::GetInstance()
{
	static SSMainMenu instance;
	return instance;
}

void SSMainMenu::Startup()
{
	g_GUI.BringWindowToFront( "DebugWindow" );

	glm::ivec2 windowSize = g_GUI.GetWindowSize( "RootWindow" );
	g_GUI.AddWindow( "MainMenu", GUI::Rectangle( 0, 0, windowSize.x, windowSize.y ), "RootWindow" );
	g_GUI.OpenWindow( "MainMenu" );

	//g_GUI.AddSprite( "", GUI::SpriteDefinition( "", 0, 0, windowSize.x, windowSize.y, glm::vec4( 0.05f, 0.05f, 0.17f, 1.0f ) ), "MainMenu" );
	g_GUI.AddSprite( "", GUI::SpriteDefinition( "", 0, 0, windowSize.x, windowSize.y, glm::vec4( 0.05f, 0.05f, 0.1f, 1.0f ) ), "MainMenu" );

	int btnWidth = 256;
	int btnHeight = 64;
	int btnSpacing = btnHeight + 10;
	int numButtons = 6;
	
	int x = m_LeftOffset; //( windowSize.x / 2 ) - ( btnWidth / 2 );
	int y = ( windowSize.y / 2 ) - ( ( btnSpacing * numButtons ) - 10 ) / 2;

	g_GUI.AddSprite( "", GUI::SpriteDefinition( "Menu_bg.png", x + btnWidth + m_LeftOffset, ( windowSize.y / 2) - ( m_BackgroundSize.y / 2 ), m_BackgroundSize.x, m_BackgroundSize.y, glm::vec4( 1.0f ) ), "MainMenu" );
	g_GUI.AddSprite( "", GUI::SpriteDefinition( "Menu_logo.png", 0, y - m_LogoSize.y, m_LogoSize.x, m_LogoSize.y, glm::vec4( 1.0f ) ), "MainMenu" );
	
	g_GUI.UseFont( FONT_ID_LEKTON_20 );
	auto setButtonDesign = [] ( GUI::Button* btn )
	{
		btn->SetBackgroundImage( "Button.png" );
		btn->SetColour( glm::vec4( 1.0f ) ); 
		btn->SetHighLightColour( glm::vec4( 0.8f, 1.0f, 1.0f, 1.0f ) );
		btn->GetBackgroundRef().BorderSize = 0;
	};

	auto addMenuButton =[x, y, btnWidth, btnHeight, btnSpacing, setButtonDesign] ( const rString& text, const rString& script ) mutable -> GUI::Button*
	{
		GUI::Button* btn = g_GUI.AddButton( text, GUI::Rectangle( x, y, btnWidth, btnHeight ), "MainMenu" );
		btn->SetClickScript( script );
		btn->SetText( text );
		btn->DisableClickEffect();
		setButtonDesign( btn );
		y += btnSpacing;

		return btn;
	};

	//Add the menu buttons here
	m_SinglePlayerButton	= addMenuButton( "Singleplayer", "SwitchGameMode( 'singleplayerlobby' )" );
	m_MultiPlayerButton		= addMenuButton( "Multiplayer", "SwitchGameMode( 'netlob' )" );
	m_ReplayButton			= addMenuButton( "Replay", "OpenWindow('ReplaySelectWindow'); CloseWindow( 'MainMenu' )" );
	m_OptionsButton			= addMenuButton( "How to play", "OpenWindow('HowTo'); CloseWindow( 'MainMenu' )" );
	m_OptionsButton			= addMenuButton( "Options", "OpenWindow('Options'); CloseWindow( 'MainMenu' )" );
	m_ExitButton			= addMenuButton( "Exit", "GE_Exit()" );
	
	//Script functions that are to be called by buttons
	g_Script.Register( "GE_Exit", [] ( IScriptEngine* ) -> int { SDL_Event event; event.type = SDL_QUIT; SDL_PushEvent( &event ); return 0; } );
	
	//Debug stuff
	//g_Script.Perform( "OpenWindow('Options'); CloseWindow( 'MainMenu' )" );

	if ( !g_SSReplaySelectMenu.HasReplays() )
		m_ReplayButton->SetEnabled( false );

	if(g_SSMusicManager.LoadSceneMusic("../../../asset/audio/script/MainMenu.lua"))
		g_SSMusicManager.Activate();
}

void SSMainMenu::Shutdown()
{
	g_GUI.DeleteObject( "MainMenu" );
}

void SSMainMenu::UpdateUserLayer( const float deltaTime )
{
	if ( g_GUI.IsWindowOpen( "MainMenu" ) )
	{
		if ( g_SSKeyBinding.ConsumeFromPressStack( ACTION_GUI_ABORT ) )
		{
			SDL_Event event;
			event.type = SDL_QUIT;
			SDL_PushEvent( &event );
		}
	}
	g_GUI.BringWindowToFront( "DebugWindow" );
}
