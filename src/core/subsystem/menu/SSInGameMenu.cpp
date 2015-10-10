/**************************************************
Copyright 2015 Daniel Bengtsson
***************************************************/

#include "SSInGameMenu.h"
#include <SDL2/SDL.h>
#include <messaging/GameMessages.h>
#include <input/KeyBindings.h>
#include <network/NetworkEngine.h>
#include "../../utility/GameSpeedController.h"
#include "../../utility/GameModeSelector.h"
#include "../../utility/PlayerData.h"
#include "../../CompileFlags.h"
#include "../network/SSNetworkController.h"
#include "../utility/SSMail.h"
#include "../gui/SSSquadFlag.h"
#include "../input/SSKeyBinding.h"
#include "../input/SSInput.h"
#include "../input/SSPicking.h"
#include "../gfx/SSAnimation.h"
#include "../gamelogic/SSCamera.h"

SSInGameMenu& SSInGameMenu::GetInstance()
{
	static SSInGameMenu instance;
	return instance;
}

void SSInGameMenu::Startup()
{
	// Set up subscriber
	g_SSMail.RegisterSubscriber( this );
	m_UserInterests = MessageTypes::USER_SIGNAL;

	// Create window
	GUI::Window* parentWindow = g_GUI.GetWindow( m_WindowNameParent );
	m_WindowInGameMenu = g_GUI.AddWindow( m_WindowNameInGameMenu, parentWindow->GetBoundingBoxRef(), m_WindowNameParent );
	m_WindowInGameMenu->Close();

	// Button scripts
	g_Script.Register( m_ScriptNameResume.c_str(), [&] ( IScriptEngine* ) -> int { OnResumeButtonClicked(); return 0; } );
	g_Script.Register( m_ScriptNameMainMenu.c_str(), [&] ( IScriptEngine* ) -> int { OnMainMenuButtonClicked(); return 0; } );
	g_Script.Register( m_ScriptNameQuit.c_str(), [&] ( IScriptEngine* ) -> int { OnQuitButtonClicked(); return 0; } );
	g_Script.Register( m_ScriptNamePause.c_str(), [&] ( IScriptEngine* ) -> int { OnPauseButtonClicked(); return 0; } );

	// Button placement and style
	int btnSpacing = m_HeightButton + 10; // TODODB: Move this to member
	int numButtons = g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Multiplayer ? 3 : 2;

	int originPosX = ( parentWindow->GetBoundingBoxRef().Width / 2 ) - ( m_WidthButton / 2 );
	int originPosY = ( parentWindow->GetBoundingBoxRef().Height / 2 ) - ( ( btnSpacing * numButtons ) - 10 ) / 2;

	g_GUI.UseFont( FONT_ID_LEKTON_20 );
	auto setButtonDesign = [] ( GUI::Button* btn )
	{
		btn->SetBackgroundImage( "Button.png" );
		btn->SetColour( glm::vec4( 1.0f ) );
		btn->SetHighLightColour( glm::vec4( 0.8f, 1.0f, 1.0f, 1.0f ) );
		btn->GetBackgroundRef().BorderSize = 0;
	};

	auto addMenuButton = [originPosX, originPosY, this, btnSpacing, setButtonDesign] ( const rString& text, const rString& script ) mutable -> GUI::Button*
	{
		GUI::Button* btn = g_GUI.AddButton( text, GUI::Rectangle( originPosX, originPosY, m_WidthButton, m_HeightButton ), "InGameMenu" );
		btn->SetClickScript( script );
		btn->SetText( text );
		btn->DisableClickEffect();
		setButtonDesign( btn );
		originPosY += btnSpacing;

		return btn;
	};

	//m_ButtonResume		= addMenuButton( "Close",		m_ScriptNameResume	+ "()" ); // TODODB: Remove the rest of the code related to this button
	if (g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Multiplayer)
		m_ButtonPause		= addMenuButton( "Pause",		m_ScriptNamePause	+ "()" );
	m_ButtonMainMenu	= addMenuButton( "Main Menu",	m_ScriptNameMainMenu+ "()" );
	m_ButtonQuit		= addMenuButton( "Quit",		m_ScriptNameQuit	+ "()" );

	m_TextPause = g_GUI.AddText( "", GUI::TextDefinition( "", originPosX, originPosY - 20, m_WidthButton, 0 ), "RootWindow" );
	m_TextPause->SetTextAlignment( GUI::ALIGNMENT::ALIGNMENT_BOTTOM_CENTER );
}

void SSInGameMenu::Shutdown()
{
	g_SSMail.UnregisterSubscriber( this );
	g_GUI.DeleteObject( m_WindowNameInGameMenu );

	for ( int i = 0; i < m_NetworkCallbackHandles.size(); ++i )
		g_NetworkEngine.UnRegisterCallback( m_NetworkCallbackHandles[i] );

	m_TextPause->SetVisible( false );
}

void SSInGameMenu::UpdateUserLayer( const float deltaTime )
{
	const GameMode& currentGameMode = g_GameModeSelector.GetCurrentGameMode();

	if ( g_SSKeyBinding.ConsumeFromPressStack( ACTION::ACTION_GUI_ABORT ) )
	{	
		if ( g_GameModeSelector.GetCurrentGameMode().Type != GameModeType::Multiplayer )
			g_SSMail.PushToUserLayer( UserSignalMessage( UserSignalType::PAUSE, g_PlayerData.GetPlayerID() ) );

		if ( g_GUI.IsWindowOpen( m_WindowInGameMenu->GetName() ) )
		{
			m_WindowInGameMenu->Close();

			g_GUI.OpenWindow( "MiniMapWindow" );
			g_GUI.OpenWindow( "SquadControl" );
			g_GUI.OpenWindow( "SquadInfoCenter" );
			g_GUI.OpenWindow( "UpgradeWindow" );
			g_GUI.OpenWindow( "GameTimer" );
			g_GUI.OpenWindow( "UISquadFlags" );
		}
		else
		{
			m_WindowInGameMenu->Open();

			g_GUI.CloseWindow( "MiniMapWindow" );
			g_GUI.CloseWindow( "SquadControl" );
			g_GUI.CloseWindow( "SquadInfoCenter" );
			g_GUI.CloseWindow( "UpgradeWindow" );
			g_GUI.CloseWindow( "GameTimer" );
			g_GUI.CloseWindow( "UISquadFlags" );

			g_GUI.CloseWindow( "PlayerGenericHelp" );
			g_GUI.CloseWindow( "InfoWindow1" );
			g_GUI.CloseWindow( "InfoWindow2" );
		}
	}

	for ( unsigned int i = 0; i < m_UserMailbox.size(); ++i )
	{
		const UserSignalMessage* specificMessage = static_cast<const UserSignalMessage*>( m_UserMailbox[i] );
		switch ( specificMessage->SignalType )
		{
			case UserSignalType::PAUSE:
			{
				if ( specificMessage->CreatedFromPacket || g_NetworkInfo.AmIHost() || g_GameModeSelector.GetCurrentGameMode().Type != GameModeType::Multiplayer )
					g_GameSpeedController.ToggleSimLayer();

				if ( g_GameSpeedController.GetSimLayerActive() )
				{
					if ( g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Multiplayer )
						m_TextPause->SetVisible( false );

					g_SSPicking.Unpause();
					g_SSSquadFlag.Unpause();
					g_SSInput.Unpause();
					g_SSAnimation.Unpause();
					g_SSCamera.Unpause();
				}
				else
				{
					if ( g_GameModeSelector.GetCurrentGameMode().Type == GameModeType::Multiplayer )
					{
						m_TextPause->SetVisible( true );
						m_TextPause->SetText( g_SSNetworkController.GetPlayerName( specificMessage->SenderID ) + " paused the game" );
					}

					g_SSPicking.Pause();
					g_SSSquadFlag.Pause();
					g_SSInput.Pause();
					g_SSAnimation.Pause();
					g_SSCamera.Pause();
				}
			} break;

			default:
				break;
		}
	}
}

void SSInGameMenu::OnResumeButtonClicked()
{
	m_WindowInGameMenu->Close();
}

void SSInGameMenu::OnPauseButtonClicked()
{
	g_SSMail.PushToUserLayer( UserSignalMessage( UserSignalType::PAUSE, g_PlayerData.GetPlayerID() ) );
}

void SSInGameMenu::OnMainMenuButtonClicked()
{
	g_GameSpeedController.ToggleSimLayer();
	g_GameModeSelector.SwitchToGameMode( GameModeType::MainMenu );
	m_WindowInGameMenu->Close();
}

void SSInGameMenu::OnQuitButtonClicked()
{
	SDL_Event quitEvent;
	quitEvent.type = SDL_QUIT;
	SDL_PushEvent( &quitEvent );
}