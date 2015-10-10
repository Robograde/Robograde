/**************************************************
2015 Daniel "MonzUn" Bengtsson & David Pejtersen
***************************************************/

#include "SSGameOver.h"
#include <SDL2/SDL_events.h>
#include <utility/Logger.h>
#include <utility/Colours.h>
#include "../../input/GameMessages.h"
#include "../../datadriven/EntityManager.h"
#include "../../datadriven/DenseComponentCollection.h"
#include "../../component/AgentComponent.h"
#include "../../component/OwnerComponent.h"
#include "../../component/DoodadComponent.h"
#include "../../utility/GameData.h"
#include "../../utility/PlayerData.h"
#include "../../utility/Alliances.h"
#include "../../subsystem/utility/SSMail.h"
#include "../../subsystem/gamelogic/SSControlPoint.h"
#include "../../utility/GameModeSelector.h"
#include "SSAI.h"


#define CONTROL_POINT_WIN_COUNTDOWN 30.0f

SSGameOver& SSGameOver::GetInstance( )
{
	static SSGameOver instance;
	return instance;
}

void SSGameOver::Startup()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
		m_PlayersDead[i] = false;

	m_ControlPointWinCountdown = CONTROL_POINT_WIN_COUNTDOWN;
	
	g_GUI.UseFont( FONT_ID_LEKTON_20 );

	m_ControlPointWinCountdownText = g_GUI.AddText( "ControlPointWinCountdown", GUI::TextDefinition( "", 0, 5 ), "InGameWindow" );
	m_ControlPointWinCountdownText->SetTextAlignment( GUI::ALIGNMENT::ALIGNMENT_TOP_CENTER );
	m_ControlPointWinCountdownText->SetVisible( false );



	m_GameOverWindow = g_GUI.AddWindow( "GameOver", GUI::Rectangle( (g_GameData.GetWindowWidth() / 2) - (m_GameOverWindowSize.x / 2), 
		(g_GameData.GetWindowHeight() / 2) - (m_GameOverWindowSize.y / 2), 
		m_GameOverWindowSize.x, m_GameOverWindowSize.y ), "InGameWindow", true );

	m_GameOverSprite = g_GUI.AddSprite( "", GUI::SpriteDefinition( "", 0, 0, 600, 300 ), "GameOver" );

	g_GUI.UseFont( FONT_ID_LEKTON_14 );
	GUI::Button* btn;
	btn = g_GUI.AddButton( "", GUI::Rectangle( 0, m_GameOverWindowSize.y - 32, m_GameOverWindowSize.x / 2, 32), "GameOver" );
	btn->SetText( "Continue" );
	btn->SetClickScript( "CloseWindow('GameOver')" );

	btn = g_GUI.AddButton( "", GUI::Rectangle( 0, m_GameOverWindowSize.y - 32, m_GameOverWindowSize.x / 2, 32), "GameOver" );
	btn->SetBackgroundImage( "Button.png" );
	btn->SetText( "Continue" );
	btn->SetClickScript( "CloseWindow('GameOver')" );

	btn = g_GUI.AddButton( "", GUI::Rectangle( m_GameOverWindowSize.x / 2, m_GameOverWindowSize.y - 32, m_GameOverWindowSize.x / 2, 32), "GameOver" );
	btn->SetBackgroundImage( "Button.png" );
	btn->SetText( "Main menu" );
	btn->SetClickScript( "SwitchGameMode('mainmenu')" );
}

void SSGameOver::UpdateSimLayer( const float timestep )
{
	if ( !m_GameOver )
	{
		// Update game over data
		CheckPlayersDead();
		CheckControlPoints( timestep );

		if ( g_GameModeSelector.GetCurrentGameMode().Type != GameModeType::AIOnly )
		{
			// Check kill victory condition
			if ( m_PlayersDead[g_PlayerData.GetPlayerID()] ) //TODODB: Protect against players dying the same frame (Desync)
			{
				m_GameOverState	= GameOverState::LOST_KILL;
				m_GameOverSprite->SetTexture( "Defeat.png" );
				m_GameOverWindow->Open();
				m_GameOver = true;
			}
			else
			{
				unsigned int deadEnemies = 0;
				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					if ( m_PlayersDead[i] ) // Previous if-statement makes sure we can't count ourselves here
						++deadEnemies;
				}

				if ( deadEnemies == MAX_PLAYERS - 1 || g_Alliances.AlliedVictoryCheck( g_PlayerData.GetPlayerID( ) ) )
				{
					m_GameOverState = GameOverState::WON_KILL;
					m_GameOverSprite->SetTexture( "Victory.png" );
					m_GameOverWindow->Open();

					m_GameOver = true;
				}
			}

			//Check control point victory condition
			if ( m_ControlPointWinCountdown <= 0.0f )
			{
				m_GameOverState									= m_PlayerOwningAllControlPoints == g_PlayerData.GetPlayerID() ? GameOverState::WON_CONTROL_POINT_TIMEOUT : GameOverState::LOST_CONTROL_POINT_TIMEOUT;
				m_GameOverSprite->SetTexture( m_PlayerOwningAllControlPoints == g_PlayerData.GetPlayerID() || g_Alliances.IsAllied( m_PlayerOwningAllControlPoints, g_PlayerData.GetPlayerID( ) ) ? "Victory.png" : "Defeat.png" );
				m_GameOverWindow->Open();
				m_GameOver = true;

				m_ControlPointWinCountdownText->SetVisible( false );
			}
		}
		else
		{
			unsigned int playersDeadCount = 0;
			for ( int i = 0; i < MAX_PLAYERS; ++i )
			{
				if ( m_PlayersDead[i] )
					++playersDeadCount;
			}

			if ( playersDeadCount == MAX_PLAYERS )
			{
				Logger::Log( "Last players died on the same frame!", "SSGameOver", LogSeverity::DEBUG_MSG );
				m_GameOver = true;
			}
			else if ( playersDeadCount == MAX_PLAYERS - 1 )
			{
				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					if ( !m_PlayersDead[i] )
					{
						m_GameOver = true;
						break;
					}
				}
			}
			else 
			if ( m_ControlPointWinCountdown <= 0.0f )
				m_GameOver = true;

			if ( m_GameOver )
			{
				for (int i = 0; i < MAX_PLAYERS; ++i)
				{
					if (!m_PlayersDead[i])
					{
						g_SSAI.SetWinner(i);
					}
				}

				SDL_Event quitEvent;
				quitEvent.type = SDL_QUIT;
				SDL_PushEvent( &quitEvent );
			}
		}

		if ( m_GameOver )
			g_SSMail.PushToNextFrame( GameOverMessage( static_cast<int>( m_GameOverState ) ) );

		if ( m_PlayerOwningAllControlPoints != -1 )
		{
			m_ControlPointWinCountdownText->SetVisible( true );
			m_ControlPointWinCountdownText->SetText( "Research station game over in: " + rToString( static_cast<int>( std::ceilf(m_ControlPointWinCountdown ) ) ) );
		}
		else
			m_ControlPointWinCountdownText->SetVisible( false );
	}
	else
		m_ControlPointWinCountdownText->SetVisible( false );
}  

void SSGameOver::Shutdown()
{
	g_GUI.DeleteObject( m_GameOverWindow->GetName() );
	g_GUI.DeleteObject( "ControlPointWinCountdown" );

	m_GameOver = false;
	m_GameOverState = GameOverState::NO_GAME_OVER;

	m_PlayerOwningAllControlPoints = -1;
}

void SSGameOver::CheckPlayersDead()
{
	bool			gameOver		= false;
	unsigned int	playerUnits[MAX_PLAYERS];
	for ( int i = 0; i < MAX_PLAYERS; ++i )
		playerUnits[i] = 0;

	short			localPlayerID	= g_PlayerData.GetPlayerID();
	unsigned int	entityID		= 0;
	EntityMask		agentAndOwnerFlag = GetDenseComponentFlag<AgentComponent>() | GetDenseComponentFlag<OwnerComponent>();
	for ( auto& entityMask : g_EntityManager.GetEntityMasks() ) // Count agents per player
	{
		if ( ( entityMask & agentAndOwnerFlag ) == agentAndOwnerFlag )
		{
			OwnerComponent* ownerComponent = GetDenseComponent<OwnerComponent>( entityID );
			if ( ownerComponent->OwnerID >= 0 )
				++playerUnits[ownerComponent->OwnerID];
		}
		++entityID;
	}

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		if ( !m_PlayersDead[i] && playerUnits[i] < 1 )
		{
			if (g_SSControlPoint.GetOwnedControlPointsCount(i) == 0)
			{
				m_PlayersDead[i] = true;
				Logger::Log("Player " + rToString(i) + " lost all units", "SSGameOver", LogSeverity::INFO_MSG);

				g_Alliances.RemoveFromAlliance(i);

				//needed for gene stuff
				g_SSAI.PlayerLost(i);
			}
		}
	}
}

void SSGameOver::CheckControlPoints( const float timestep )
{
	// Check if there is one single player that owns all control points
	m_PlayerOwningAllControlPoints = -1;
	int maxControlPoints = g_SSControlPoint.GetTotalControlPointCount( );

	for ( short i = 0; i < MAX_PLAYERS; ++i )
	{
		int ownedControlPoints = g_SSControlPoint.GetOwnedControlPointsCount( i );

		if ( ownedControlPoints == maxControlPoints )
		{
			m_PlayerOwningAllControlPoints = i;
			g_SSAI.SetWinner(i);
			break;
		}
	}

	// Decrement or reset timer depending on if a single player owned all the control points
	if ( m_PlayerOwningAllControlPoints == -1 )
		m_ControlPointWinCountdown = CONTROL_POINT_WIN_COUNTDOWN;
	else
		m_ControlPointWinCountdown -= timestep;
}