/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#include "SSGameLobby.h"
#include <fstream>
#include <utility/Logger.h>
#include <utility/FileUtility.h>
#include <utility/PlatformDefinitions.h>
#include <utility/Randomizer.h>
#include <utility/Colours.h>
#include <utility/ConfigManager.h>
#include <network/NetworkEngine.h>
#include <network/NetworkInfo.h>
#include <network/PacketPump.h>
#include <input/Input.h>
#include <messaging/GameMessages.h>
#include <gfx/GraphicsEngine.h>
#include "../../EntityFactory.h"
#include "../gamelogic/SSSceneLoader.h"
#include "../../utility/GameModeSelector.h"
#include "../../utility/Alliances.h"
#include "../../utility/GameData.h"
#include "../network/SSNetworkController.h"
#include "../gui/SSChat.h"
#include "../SubsystemManager.h"
#include "../input/SSKeyBinding.h"
#include "../gui/SSMiniMap.h"
#include "../utility/SSMail.h"
#include "../../CompileFlags.h"
#include "../gamelogic/SSAI.h"
#include "../gamelogic/SSResourceManager.h"

SSGameLobby& SSGameLobby::GetInstance()
{
	static SSGameLobby instance;
	return instance;
}

void SSGameLobby::Startup()
{
	// Seed the randomizer
	const unsigned int randomSeed = static_cast<const unsigned int>( SDL_GetPerformanceCounter() );
	g_Randomizer.Seed( randomSeed );

	// Register subscriber
	g_SSMail.RegisterSubscriber( this );
	m_UserInterests = MessageTypes::COLOUR_CHANGE 
		| MessageTypes::TEAM_CHANGE 
		| MessageTypes::SPAWN_POINT_CHANGE 
		| MessageTypes::CHANGE_SPAWN_COUNT
		| MessageTypes::RESERVE_AI_PLAYER;

	// Set all player slots to not ready
	for ( int i = 0; i < MAX_PLAYERS; ++i )
		m_PlayersReady[i] = false;

	// Register callback to network engine
	m_NetworkCallbackHandle = g_NetworkEngine.RegisterCallback( NetworkCallbackEvent::Disconnection, std::bind( &SSGameLobby::OnDisconnection, this, std::placeholders::_1 ) );

	// Player colours
	m_ColourEntries.push_back( ColourEntry	{ "Green",	Colours::KINDA_GREEN, Colours::KINDA_GREEN	, -1 } );
	m_ColourEntries.push_back( ColourEntry	{ "Red", 	Colours::KINDA_RED 	, Colours::KINDA_RED	, -1 } );
	m_ColourEntries.push_back( ColourEntry	{ "Blue", 	Colours::KINDA_BLUE	, Colours::KINDA_BLUE	, -1 } );
	m_ColourEntries.push_back( ColourEntry	{ "Pink",	Colours::KINDA_PINK	, Colours::KINDA_PINK	, -1 } );
	m_ColourEntries.push_back( ColourEntry	{ "Yellow", Colours::VERY_YELLOW, Colours::VERY_YELLOW	, -1 } );
	m_ColourEntries.push_back( ColourEntry	{ "Cyan",	Colours::LTBLUE		, Colours::LTBLUE		, -1 } );
	// Text colour modified
	m_ColourEntries.push_back( ColourEntry	{ "Black",	Colours::BLACK		, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), -1 } );

	g_GameData.SetPlayerColour( 0, Colours::KINDA_GREEN );
	g_GameData.SetPlayerColour( 1, Colours::KINDA_RED );
	g_GameData.SetPlayerColour( 2, Colours::KINDA_BLUE );
	g_GameData.SetPlayerColour( 3, Colours::KINDA_PINK );

	// Set up GUI
	g_GUI.UseFont( FONT_ID_LEKTON_16 );

	m_ConnectionState			= ConnectionState::EnteringIP;
	m_ConnectionAttemptFailed	= false;
	g_NetworkEngine.RegisterCallback( NetworkCallbackEvent::ConnectionAttemptSuccess, std::bind( &SSGameLobby::OnConnectionSuccess, this, std::placeholders::_1 ) ); // TODODB: Check against this variable already being set eller nånting.
	g_NetworkEngine.RegisterCallback( NetworkCallbackEvent::ConnectionAttemptFail, std::bind( &SSGameLobby::OnConnectionFail, this, std::placeholders::_1 ) ); // TODODB: Check against this variable already being set eller nånting.

	// Button scripts
	g_Script.Register( m_ScriptNameReady.c_str(), [&] ( IScriptEngine* ) -> int { OnReadyButtonClicked(); return 0; } );
	g_Script.Register( m_ScriptNameStartGame.c_str(), [&] ( IScriptEngine* ) -> int { StartGame(); return 0; } );
	g_Script.Register(m_ScriptChangeGameSetting.c_str(), [this](IScriptEngine* script) -> int
	{
		m_NumberOfSpawns += script->PopInt();
		if (m_NumberOfSpawns < 1)
			m_NumberOfSpawns = 1;
		if (m_NumberOfSpawns > SQUAD_MAXIMUM_UNIT_COUNT)
			m_NumberOfSpawns = SQUAD_MAXIMUM_UNIT_COUNT;
		ChangeSpawnCount( m_NumberOfSpawns );
		return 0;
	} );

	GUI::Window* parentWindow = g_GUI.GetWindow( m_WindowNameParent );
	glm::ivec2 middleParent = glm::ivec2(parentWindow->GetBoundingBoxRef().X, parentWindow->GetBoundingBoxRef().Y) + parentWindow->GetSize() / 2;
	glm::ivec2 sizeParent = parentWindow->GetSize( );

	{ // Enter ip
		m_WindowEnterIP = g_GUI.AddWindow( m_WindowNameEnterIP, GUI::Rectangle( middleParent.x - m_WidthEnterIP / 2, middleParent.y - m_HeightEnterIP / 2, m_WidthEnterIP, m_HeightEnterIP ), m_WindowNameParent, true );

		m_TextBoxEnterIP = g_GUI.AddTextBox( "", GUI::Rectangle( m_WidthButton, m_HeightEnterIP - m_HeightTextBox, m_WidthTextBox, m_HeightTextBox) , m_WindowNameEnterIP);

		// Connect
		m_ButtonConnect = g_GUI.AddButton( "", GUI::Rectangle( m_WidthEnterIP - m_WidthButton * 2, m_HeightEnterIP - m_HeightButton, m_WidthButton, m_HeightButton), m_WindowNameEnterIP );
		m_ButtonConnect->SetText( "Connect" );
		g_Script.Register( m_ScriptNameConnect.c_str(), [this](IScriptEngine*){ ConnectToGame(m_TextBoxEnterIP->GetText()); return 0; } );
		m_ButtonConnect->SetClickScript( m_ScriptNameConnect + "()" );
		m_ButtonConnect->GetBackgroundRef().Texture = m_TextureNameButtons;

		// Host
		m_ButtonHost = g_GUI.AddButton( "", GUI::Rectangle( m_WidthEnterIP - m_WidthButton, m_HeightEnterIP - m_HeightButton, m_WidthButton, m_HeightButton), m_WindowNameEnterIP );
		m_ButtonHost->SetText( "Host" );
		g_Script.Register( m_ScriptNameHost.c_str(), [this](IScriptEngine*){ HostGame(); return 0; } );
		m_ButtonHost->SetClickScript( m_ScriptNameHost + "()" );
		m_ButtonHost->GetBackgroundRef().Texture = m_TextureNameButtons;

		// Text
		GUI::Text* infoText = g_GUI.AddText( "", GUI::TextDefinition( "Please enter an IP adress to connect to", 0, m_TopPadding, m_WidthEnterIP, m_HeightInfoText ), m_WindowNameEnterIP );
		infoText->GetTextDefinitionRef().Alignment = GUI::ALIGNMENT_TOP_CENTER;
		m_TextError = g_GUI.AddText( "", GUI::TextDefinition( "", 0, m_TopPadding + m_HeightInfoText, m_WidthEnterIP, m_HeightErrorText ), m_WindowNameEnterIP );

		// Back button
		m_ButtonBackEnterIP = g_GUI.AddButton( "", GUI::Rectangle( 0, m_WindowEnterIP->GetSize( ).y - m_HeightButton, m_WidthButton, m_HeightButton ), m_WindowNameEnterIP );
		g_Script.Register( m_ScriptNameBackEnterIP.c_str(), [this](IScriptEngine*){ BackFromEnterIP( ); return 0; } );
		m_ButtonBackEnterIP->SetClickScript( m_ScriptNameBackEnterIP + "()" );
		m_ButtonBackEnterIP->SetText( "Back" );
		m_ButtonBackEnterIP->GetBackgroundRef().Texture = m_TextureNameButtons;

		// Enter ip is default window
		m_WindowEnterIP->ToggleOpen();

		// Default IP
		CallbackConfig* networkConfig = g_ConfigManager.GetConfig( "network.cfg" );
		int defaultPort = networkConfig->GetInt( "DefaultListeningPort", 40606, "Default port for establishing connections" );
		rString savedIP = networkConfig->GetString( "SavedIP", "", "The IP of the last successfull connection attempt" );

		m_TextBoxEnterIP->SetText( savedIP );
	}
	{ // Lobby
		m_WindowLobby = g_GUI.AddWindow( m_WindowNameLobby, parentWindow->GetBoundingBoxRef(), m_WindowNameParent );
		glm::ivec2 sizeLobby = m_WindowLobby->GetSize( );
		// Start game (host)
		m_ButtonStartGame = g_GUI.AddButton( "", GUI::Rectangle( sizeLobby.x - m_WidthButton * 2, sizeLobby.y - m_HeightButton, m_WidthButton, m_HeightButton ), m_WindowNameLobby );
		m_ButtonStartGame->SetText( "Start" );
		m_ButtonStartGame->SetClickScript( m_ScriptNameStartGame + "()" );
		m_ButtonStartGame->SetVisible( false );
		m_ButtonStartGame->SetEnabled( false );
		m_ButtonStartGame->GetBackgroundRef().Texture = m_TextureNameButtons;

		// Back button
		m_ButtonBackLobby = g_GUI.AddButton( "", GUI::Rectangle( 0, sizeLobby.y - m_HeightButton, m_WidthButton, m_HeightButton ), m_WindowNameLobby );
		g_Script.Register( m_ScriptNameBackLobby.c_str(), [this](IScriptEngine*) 
			{
				BackFromLobby();
				return 0; 
			} );
		m_ButtonBackLobby->SetClickScript( m_ScriptNameBackLobby + "()" );
		m_ButtonBackLobby->SetText( "Back" );
		m_ButtonBackLobby->GetBackgroundRef().Texture = m_TextureNameButtons;

		// Ready (client)
		m_ButtonReady = g_GUI.AddButton( "", GUI::Rectangle( sizeLobby.x - m_WidthButton, sizeLobby.y - m_HeightButton, m_WidthButton, m_HeightButton ), m_WindowNameLobby );
		m_ButtonReady->GetBackgroundRef().Texture = m_TextureNameButtons;
		m_ButtonReady->SetText( "Ready" );
		m_ButtonReady->SetClickScript( m_ScriptNameReady + "()" );
		{ // Connected clients 
			m_WindowConnectedClients = g_GUI.AddWindow( m_WindowNameConnectedClients,
					GUI::Rectangle( 0, 0,
						sizeParent.x / 2, sizeParent.y / 2 ),
					m_WindowNameLobby, true );
			m_WindowConnectedClients->ToggleOpen( );
			g_GUI.UseFont( FONT_ID_LEKTON_20 );
			g_Script.Register( m_ScriptNameColourChange.c_str(), [this](IScriptEngine* script) -> int
					{
						int playerID = script->PopInt( );
						assert( playerID >= 0 && playerID < MAX_PLAYERS );
						int selectedIndex = m_ConnectedPlayerEntries[playerID].Colour->GetSelectedIndex( );
						TryChangeColour( selectedIndex, playerID, false );
						return 0;
					} );
			g_Script.Register( m_ScriptNameTeamChange.c_str(), [this](IScriptEngine* script) -> int
					{
						int playerID = script->PopInt( );
						assert( playerID >= 0 && playerID < MAX_PLAYERS );
						int selectedIndex = m_ConnectedPlayerEntries[playerID].Team->GetSelectedIndex( );
						TryChangeTeam( selectedIndex, playerID, false );
						return 0;
					} );
			g_Script.Register( m_ScriptNameKick.c_str( ), [this](IScriptEngine* script) -> int
					{
						int playerID = script->PopInt( );
						if ( m_ConnectedPlayerEntries[ playerID ].IsAI )
						{
							ChangeReserveAI( playerID, false );
							OnDisconnection( &playerID );
						}
						else
							g_NetworkEngine.RequestDisconnection( playerID );
						return 0;
					} );
			g_Script.Register( m_ScriptNameReserveForAI.c_str( ), [this](IScriptEngine* script) -> int
					{
						int playerID = script->PopInt( );
						if ( !m_ConnectedPlayerEntries[playerID].IsAI )
						{
							OnNewClientConnect( playerID );
						}
						ChangeReserveAI( playerID, true );
						g_NetworkEngine.RequestDisconnection( playerID );
						return 0;
					} );
			// Players
			int entryY = m_HeightPlayerEntry * (MAX_PLAYERS - 1) + m_TopPadding;
			for ( int i = MAX_PLAYERS -1 ; i >= 0; --i )
			{
				int x = m_WindowConnectedClients->GetSize().x - m_WidthReady;
				ConnectedPlayer& playerEntry = m_ConnectedPlayerEntries[i];
				playerEntry.Name = g_GUI.AddText( "", GUI::TextDefinition( "TODOJM", m_LeftPadding, entryY, m_WidthName, m_HeightPlayerEntry ), m_WindowNameConnectedClients );

				playerEntry.Ready = g_GUI.AddSprite( "", GUI::SpriteDefinition( m_TextureNameReadySprite, m_WindowConnectedClients->GetSize().x - m_WidthReady, entryY, m_WidthReady, m_HeightPlayerEntry ), m_WindowNameConnectedClients );
				x -= m_WidthTeam;
				playerEntry.Team = g_GUI.AddComboBox( "", GUI::Rectangle( x, entryY, m_WidthTeam, m_HeightPlayerEntry ), m_WindowNameConnectedClients );
				playerEntry.Team->AddItem( "Team 1" );
				playerEntry.Team->AddItem( "Team 2" );
				playerEntry.Team->AddItem( "Team 3" );
				playerEntry.Team->AddItem( "Team 4" );
				playerEntry.Team->SetClickScript( m_ScriptNameTeamChange + "(" + rToString( i ) + ")");
				playerEntry.Team->GetBackgroundRef().Texture = m_TextureNameComboBox;
				playerEntry.Team->SetItemBackgroundImage( m_TextureNameComboBoxItem );
				x -= m_WidthColour;
				playerEntry.Colour = g_GUI.AddComboBox( "", GUI::Rectangle( x, entryY, m_WidthColour, m_HeightPlayerEntry ), m_WindowNameConnectedClients );
				for( auto& colourEntry : m_ColourEntries )
				{
					playerEntry.Colour->AddItem( colourEntry.Text );
					playerEntry.Colour->GetItem( playerEntry.Colour->GetNumItems() - 1 ).Text.Colour = colourEntry.TextColour;
				}
				playerEntry.Colour->SetClickScript( m_ScriptNameColourChange + "(" + rToString( i ) + ")" );
				playerEntry.Colour->GetBackgroundRef().Texture = m_TextureNameComboBox;
				playerEntry.Colour->SetItemBackgroundImage( m_TextureNameComboBoxItem );
				x -= m_WidthKick;
				playerEntry.Kick = g_GUI.AddButton( "", GUI::Rectangle( x, entryY, m_WidthKick, m_HeightPlayerEntry ), m_WindowNameConnectedClients );
				playerEntry.Kick->SetClickScript( m_ScriptNameKick + "(" + rToString( i ) + ")" );
				playerEntry.Kick->GetBackgroundRef().Texture = m_TextureNameKickSprite;
				x -= m_WidthReserveForAI;
				playerEntry.ReserveForAIBtn = g_GUI.AddButton( "", GUI::Rectangle( x, entryY, m_WidthReserveForAI, m_HeightPlayerEntry ), m_WindowNameConnectedClients );
				playerEntry.ReserveForAIBtn->SetClickScript( m_ScriptNameReserveForAI + "(" + rToString( i ) + ")" );
				playerEntry.ReserveForAIBtn->GetBackgroundRef().Texture = m_TextureNameButtons;
				playerEntry.ReserveForAIBtn->GetTextDefinitionRef().Text = "AI";

				playerEntry.PlayerID = i;

				entryY -= m_HeightPlayerEntry;
			}
		}
		{ // Select level window
			m_WindowSelectLevel = g_GUI.AddWindow( m_WindowNameSelectLevel, 
					GUI::Rectangle( sizeLobby.x / 2, 0, sizeLobby.x / 2, sizeLobby.y / 2 ), 
					m_WindowNameLobby, true );
			glm::ivec2 slSize = m_WindowSelectLevel->GetSize();
			m_WindowSelectLevel->ToggleOpen( );
			m_ComboBoxSelectLevel = g_GUI.AddComboBox( "", GUI::Rectangle(
						0, m_WindowSelectLevel->GetSize().y - m_HeightComboBoxSelectLevel, m_WindowSelectLevel->GetSize().x, m_HeightComboBoxSelectLevel ),
					m_WindowNameSelectLevel );
			m_ComboBoxSelectLevel->SetBackgroundImage( m_TextureNameSelectLevel );
			m_ComboBoxSelectLevel->SetItemBackgroundImage( m_TextureNameSelectLevel );
			g_Script.Register( m_ScriptClickLevelChange.c_str(), [this](IScriptEngine*){ SetLevel(m_ComboBoxSelectLevel->GetTextDefinitionRef().Text); return 0; } );
			m_ComboBoxSelectLevel->SetClickScript( m_ScriptClickLevelChange + "()" );

			m_SizeLevelWindow = std::min(slSize.x, slSize.y - m_HeightComboBoxSelectLevel);
			m_WindowLevel = g_GUI.AddWindow( m_WindowNameLevel, GUI::Rectangle( 
						(slSize.x / 2) - (m_SizeLevelWindow / 2), 0, m_SizeLevelWindow, m_SizeLevelWindow ),
					m_WindowNameSelectLevel );
			m_WindowLevel->ToggleOpen( );
			m_SpriteLevelSelected = g_GUI.AddSprite( "", GUI::SpriteDefinition(
						"",
						0,
						0,
						m_SizeLevelWindow,
						m_SizeLevelWindow),
					m_WindowNameLevel );

			CreateSpawnPointButtons( );
			LoadLevels( );
		}

		{ // Gameplay settings

			int gpsX = sizeLobby.x / 2;
			int gpsY = sizeLobby.y / 2;

			int gpsW = sizeLobby.x / 2;
			int gpsH = sizeLobby.y / 2;

			int gpsDiff = m_HeightComboBoxSelectLevel;

			int gpsStartCountW = gpsW - gpsDiff * 4 - gpsDiff * 3;

			m_WindowGamePlaySettings = g_GUI.AddWindow(m_WindowNameGamePlaySettings, GUI::Rectangle(gpsX, gpsY, gpsW, gpsH -m_HeightButton), m_WindowNameLobby, true);
			m_WindowGamePlaySettings->ToggleOpen();

			// | Starting units count | 
			m_BtnSpawnCount = g_GUI.AddButton("GamePlaySettingStartCount", GUI::Rectangle(gpsDiff, gpsDiff, gpsStartCountW, gpsDiff), m_WindowNameGamePlaySettings);
			m_BtnSpawnCount->GetBackgroundRef().Texture = m_TextureNameButtons;
			m_BtnSpawnCount->SetText("Starting units count");
			m_BtnSpawnCount->SetEnabled(false);

			// | Starting units count | - |
			m_BtnSpawnCountDec = g_GUI.AddButton("GamePlaySettingStartCountModDec", GUI::Rectangle(gpsStartCountW + gpsDiff, gpsDiff, gpsDiff, gpsDiff), m_WindowNameGamePlaySettings);
			m_BtnSpawnCountDec->GetBackgroundRef().Texture = m_TextureNameButtons;
			m_BtnSpawnCountDec->SetText("-");
			m_BtnSpawnCountDec->SetClickScript(m_ScriptChangeGameSetting + "(-1)");
			

			// | Starting units count | - | X |
			m_BtnSpawnCountVal = g_GUI.AddButton("GamePlaySettingStartCountVal", GUI::Rectangle(gpsStartCountW + gpsDiff * 2, gpsDiff, gpsDiff, gpsDiff), m_WindowNameGamePlaySettings);
			m_BtnSpawnCountVal->GetBackgroundRef().Texture = m_TextureNameButtons;
			m_BtnSpawnCountVal->SetText("X");
			m_BtnSpawnCountVal->SetEnabled(false);

			// | Starting units count | - | X | + |
			m_BtnSpawnCountInc = g_GUI.AddButton("GamePlaySettingStartCountModInc", GUI::Rectangle(gpsStartCountW + gpsDiff * 3, gpsDiff, gpsDiff, gpsDiff), m_WindowNameGamePlaySettings);
			m_BtnSpawnCountInc->GetBackgroundRef().Texture = m_TextureNameButtons;
			m_BtnSpawnCountInc->SetText("+");
			m_BtnSpawnCountInc->SetClickScript(m_ScriptChangeGameSetting + "(1)");
			
		}
	}
}

void SSGameLobby::Shutdown()
{
	m_ConnectionState = ConnectionState::EnteringIP;
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		if ( m_ConnectedPlayerEntries[i].IsAI )
			ChangeReserveAI( i, false );
	}
	g_GUI.DeleteObject( m_WindowNameLobby );	
	g_GUI.DeleteObject( m_WindowNameEnterIP );
	g_NetworkEngine.UnRegisterCallback( m_NetworkCallbackHandle );

	for ( auto& levelTextures : m_LevelTextures )
	{
		tDelete( levelTextures.second );
	}

	m_LevelTextures.clear( );
	m_ResearchStations.clear( );
	m_ResourceSpawns.clear( );

	g_SSChat.SetChatState( SSChat::CHATSTATE_INGAME );
	g_SubsystemManager.RestartSubsystem( "Chat" );
	m_ColourEntries.clear();
	g_SSMail.UnregisterSubscriber( this );
}

void SSGameLobby::UpdateUserLayer( const float deltaTime )
{
	if ( m_ConnectionState == ConnectionState::EnteringIP )
	{
		if ( m_TextBoxEnterIP->GetText() == "" )
			m_ButtonConnect->SetEnabled( false );
		else
			m_ButtonConnect->SetEnabled( true );
	}

	if ( m_ConnectionState == ConnectionState::WaitingToConnect )
	{
		if ( g_SSNetworkController.DoesPlayerExist( g_NetworkInfo.GetHostID() ) )
		{
			m_WindowEnterIP->ToggleOpen();
			m_WindowLobby->ToggleOpen();
			m_ConnectionState = ConnectionState::Connected;
			g_GameData.ResetSpawnPoints( );
			UpdateSpawnPointButtons( );
			m_TextError->GetTextDefinitionRef().Text = "";
			g_SSChat.InitializeChatWindow( 0, m_WindowLobby->GetSize().y / 2, m_WindowLobby->GetSize().x / 2, (m_WindowLobby->GetSize().y / 2) - m_HeightButton, 70, SSChat::CHATSTATE_LOBBY );
			g_GUI.OpenWindow( "Chat" );
			g_Alliances.Initialize( m_MaxNrOfTeams );
			m_ButtonStartGame->SetVisible( false );
			m_ButtonStartGame->SetEnabled( false  );
		}
		
		if ( m_ConnectionAttemptFailed )
		{
			m_ConnectionState = ConnectionState::EnteringIP;
			m_ButtonConnect->SetEnabled( true );
			m_ButtonHost->SetEnabled( true );
			m_TextError->GetTextDefinitionRef().Text = "Failed to connect to game"; // TODODB: Better error message plox
			m_ConnectionAttemptFailed = false;
		}
	}
	if ( m_ConnectionState == ConnectionState::Connected || m_ConnectionState == ConnectionState::Hosting )
	{
		// Read messages
		for ( auto& mail : m_UserMailbox )
		{
			if ( mail->CreatedFromPacket && mail->Type == MessageTypes::COLOUR_CHANGE )
			{
				const ColourChangeMessage* colourChange = static_cast<const ColourChangeMessage*>( mail );
				TryChangeColour( colourChange->SelectedColourIndex, colourChange->PlayerID, colourChange->Force );
			}
			if ( mail->CreatedFromPacket && mail->Type == MessageTypes::TEAM_CHANGE )
			{
				const TeamChangeMessage* teamChange = static_cast<const TeamChangeMessage*>( mail );
				TryChangeTeam( teamChange->SelectedTeamIndex, teamChange->PlayerID, teamChange->Force );
			}
			if ( mail->CreatedFromPacket && mail->Type == MessageTypes::SPAWN_POINT_CHANGE )
			{
				const SpawnPointChangeMessage* spawnChange = static_cast<const SpawnPointChangeMessage*>( mail );
				TryChangeSpawnPoint( spawnChange->SelectedSpawnPoint, spawnChange->PlayerID, spawnChange->Force );
			}
			if (mail->CreatedFromPacket && mail->Type == MessageTypes::CHANGE_SPAWN_COUNT)
			{
				const SpawnCountChangeMessage* countChange = static_cast<const SpawnCountChangeMessage*>(mail);
				ChangeSpawnCount( countChange->Count );
			}
			if (mail->CreatedFromPacket && mail->Type == MessageTypes::RESERVE_AI_PLAYER)
			{
				const ReserveAIMessage* reserveAI = static_cast<const ReserveAIMessage*>(mail);
				ChangeReserveAI( reserveAI->PlayerID, reserveAI->On );
			}
		}
		unsigned int nrOfReadyPlayers = 0;
		for ( auto& playerEntry : m_ConnectedPlayerEntries )
		{
			if ( !playerEntry.IsAI )
			{
				playerEntry.Name->SetVisible( false );
				playerEntry.Colour->SetVisible( false );
				playerEntry.Ready->SetVisible( false );
				playerEntry.Team->SetVisible( false );
				playerEntry.Kick->SetVisible( false );
				playerEntry.Kick->SetEnabled( false );
			}
			else
			{
				playerEntry.Name->SetVisible( true );
				playerEntry.Colour->SetVisible( true );
				playerEntry.Ready->SetVisible( true );
				playerEntry.Team->SetVisible( true );
				playerEntry.Kick->SetVisible( g_NetworkInfo.AmIHost() );

				playerEntry.Colour->SetEnabled( g_NetworkInfo.AmIHost() );
				playerEntry.Team->SetEnabled( g_NetworkInfo.AmIHost() );
				playerEntry.Kick->SetEnabled( g_NetworkInfo.AmIHost() );
			}
			playerEntry.ReserveForAIBtn->SetVisible( g_NetworkInfo.AmIHost() );
			playerEntry.ReserveForAIBtn->SetEnabled( g_NetworkInfo.AmIHost() );
		}
		unsigned int nrOfActivePlayers = 0; // Includes AI
		for ( auto& player : g_SSNetworkController.GetNetworkedPlayers() )
		{
			// TODOJM/TODODB: Handle spectators
			assert( player.second.PlayerID < MAX_PLAYERS ); // TODODB: Remove this assert when this can no longer happen
			ConnectedPlayer connectedPlayer = m_ConnectedPlayerEntries[player.second.PlayerID];
			connectedPlayer.Name->GetTextDefinitionRef().Text = player.second.Name;
			connectedPlayer.Colour->GetTextDefinitionRef().Text = "Colour";
			if ( m_PlayersReady[player.second.PlayerID] )
			{
				++nrOfReadyPlayers;
			}
			ConnectedPlayer& playerEntry = m_ConnectedPlayerEntries[player.second.PlayerID];
			playerEntry.IsActive = true;
			// Enable entry changing only when it is the local player (Some reduntant stuff for possible future use)
			playerEntry.Name->SetEnabled( g_NetworkInfo.GetNetworkID( ) == player.second.PlayerID );
			playerEntry.Colour->SetEnabled( g_NetworkInfo.GetNetworkID( ) == player.second.PlayerID );
			playerEntry.Ready->SetEnabled( g_NetworkInfo.GetNetworkID( ) == player.second.PlayerID );
			playerEntry.Team->SetEnabled( g_NetworkInfo.GetNetworkID( ) == player.second.PlayerID );
			playerEntry.Name->SetVisible( true );
			playerEntry.Colour->SetVisible( true );
			playerEntry.Ready->SetVisible( m_PlayersReady[player.second.PlayerID] );
			playerEntry.Team->SetVisible( true );
			playerEntry.Kick->SetVisible( g_NetworkInfo.AmIHost( ) && player.second.PlayerID != g_NetworkInfo.GetNetworkID() );
			playerEntry.Kick->SetEnabled( g_NetworkInfo.AmIHost( ) && player.second.PlayerID != g_NetworkInfo.GetNetworkID() );
			playerEntry.ReserveForAIBtn->SetEnabled( false );
			playerEntry.ReserveForAIBtn->SetVisible( false );
			playerEntry.IsAI = false;
			nrOfActivePlayers++;
		}
		// Disable or enable start game button depending on number of players ready
		m_ButtonStartGame->SetEnabled( 
				nrOfActivePlayers <= m_CurrentLevelNrOfSpawns // Don't start if there is not enough spawn points
				&& nrOfReadyPlayers == g_SSNetworkController.GetNetworkedPlayers().size() // Only start if all players are ready
				&& g_SSNetworkController.GetNetworkedPlayers().size() > 1 // Don't start alone
				&& ArePlayersOnDifferentTeams( ) ); // Do not allow players to play on same team as it will result in instant victory
		m_ComboBoxSelectLevel->SetEnabled( m_ConnectionState == ConnectionState::Hosting ); // Only host can start

		// Update spawn point colours
		for ( auto& spawn : m_ButtonsSpawnPoints )
		{
			if ( spawn.PlayerID == -1 ) // Neutral
			{
				spawn.Button->SetColour( m_ColourNeutralSpawnPoint );
			}
			else // Player owns it
			{
				assert( spawn.PlayerID < MAX_PLAYERS );
				int colourIndex = m_ConnectedPlayerEntries[ spawn.PlayerID ].SelectedColourIndex;
				assert( colourIndex >= 0 && colourIndex < m_ColourEntries.size() );
				spawn.Button->SetColour( m_ColourEntries.at( colourIndex ).Colour );
			}
		}

		// Only host can change initial spawn count, can not be in startup because you can not be host there
		m_BtnSpawnCountDec->SetEnabled(g_NetworkInfo.AmIHost());
		m_BtnSpawnCountInc->SetEnabled(g_NetworkInfo.AmIHost());

		m_BtnSpawnCountVal->SetText(rToString(m_NumberOfSpawns));
	}
	if ( g_SSKeyBinding.ConsumeFromPressStack( ACTION::ACTION_GUI_ABORT ) )
	{
		if ( m_WindowEnterIP->IsOpen( ) )
		{
			// TODODB: make this work? :D
			//if ( g_NetworkInfo.IsTryingToConnect() )
				//AbortConnection();
			//else
			BackFromEnterIP( );
		}
		else if ( m_WindowLobby->IsOpen( ) )
		{
			BackFromLobby( );
		}
	}
}

void SSGameLobby::Reset()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
		m_PlayersReady[i] = false;
}

void SSGameLobby::StartGame()
{
	if ( g_NetworkInfo.AmIHost() )
	{
		if ( AreAnyClientsConnected( ) )
			g_PacketPump.SendToAll( UserSignalMessage( UserSignalType::START_GAME, g_NetworkInfo.GetNetworkID() ) );
		SwitchToMultiplayer( );
	}
	else
		Logger::Log( "Attempted to start the game without being host", "SSGameLobby", LogSeverity::WARNING_MSG );
}

void SSGameLobby::SwitchToMultiplayer( )
{
	g_GameData.ClearPlayers( );
	for ( auto& player : m_ConnectedPlayerEntries )
	{
		if ( player.IsActive )
		{
			g_GameData.AddPlayer( player.PlayerID, player.IsAI );
		}
	}
	g_GameModeSelector.SwitchToGameMode( GameModeType::Multiplayer );
}

void SSGameLobby::ToggleReadyStatus( short playerID )
{
	if ( playerID != PLAYER_ID_INVALID )
	{
		m_PlayersReady[playerID] = !m_PlayersReady[playerID]; // TODODB: Make sure only valid player ID:s can be used here (not higher than MAX_PLAYERS for the selected map)
		Logger::Log( "Player " + rToString( playerID ) + " ready status = " + rToString( m_PlayersReady[playerID] ), "SSGameLobby", LogSeverity::DEBUG_MSG );
	}
}

bool SSGameLobby::IsPlayerReady( short playerID )
{
	if ( playerID >= 0 && playerID < MAX_PLAYERS )
		return m_PlayersReady[playerID];
	
	Logger::Log( "Attempted to get ready status of invalid playerID (ID  = " + rToString( playerID ) + ")", "SSGameLobby", LogSeverity::WARNING_MSG );
	return false;
}

void SSGameLobby::SetLevel( const rString& levelName )
{
	if ( m_ComboBoxSelectLevel )
	{
		if ( g_NetworkInfo.AmIHost( ) )
		{
			g_SSSceneLoader.SetSceneToBeLoaded( levelName );
            m_ComboBoxSelectLevel->SetText( levelName );
			if ( AreAnyClientsConnected( ) ) // Only send message if there is anyone to send it to
			{
				g_PacketPump.SendToAll( LevelSelectionChangeMessage( levelName ) );
			}
		}
		else
		{
			m_ComboBoxSelectLevel->SetText( levelName );
			g_SSSceneLoader.SetSceneToBeLoaded( levelName );
		}
		UpdateSpawnPointButtons( );
		auto levelTexture = m_LevelTextures.find( levelName );
		if ( levelTexture != m_LevelTextures.end( ) )
		{
			m_SpriteLevelSelected->GetSpriteDefinitionRef().TextureHandle = levelTexture->second->GetHandle( );
			g_SSMiniMap.SetMiniMapTexture( levelTexture->second->GetFilename() );
		}
		else
		{
			Logger::Log( "Failed to set texture sprite for level: " + levelName, "SSGameLobby", LogSeverity::ERROR_MSG );
		}
	}
}

const rString& SSGameLobby::GetSelectedLevel( ) const
{
	return m_ComboBoxSelectLevel->GetTextDefinitionRef().Text;
}

void SSGameLobby::OnNewClientConnect( short playerID )
{
	if ( AreAnyClientsConnected( ) )
		g_PacketPump.Send( LevelSelectionChangeMessage( GetSelectedLevel() ), playerID );

	TryChangeColour( 0, playerID, false );
	TryChangeTeam( playerID, playerID, false ); // Run just in case it is decided to run limitation logic on teams
	ChangeSpawnCount(m_NumberOfSpawns);

	if ( !m_ConnectedPlayerEntries[ playerID ].IsAI ) // No need to send to AI players
	{
		int i = 0;
		for ( auto& player : m_ConnectedPlayerEntries )
		{
			if ( player.IsActive && AreAnyClientsConnected( ) )
			{
				g_PacketPump.Send( ColourChangeMessage( i, player.SelectedColourIndex, true), playerID );
				g_PacketPump.Send( TeamChangeMessage( i, player.SelectedTeamIndex, true), playerID );
				if ( player.IsAI )
				{
					g_PacketPump.Send( ReserveAIMessage( i, true ), playerID );
				}
				for ( unsigned int j = 0; j < m_ButtonsSpawnPoints.size(); ++j )
				{
					if ( m_ButtonsSpawnPoints.at( j ).PlayerID == i )
					{
						g_PacketPump.Send( SpawnPointChangeMessage( i, j, true ), playerID );
						break;
					}
				}
			}
			++i;
		}
	}
}

void SSGameLobby::ConnectToGame( const rString& input )
{
	if ( !g_NetworkInfo.AmIHost() )
	{
		rString IPAddress = "";
		unsigned short port = INVALID_PORT;
		NetworkUtility::GetIPAndPortFromString( input, IPAddress, port );

		if ( IPAddress == "" )
			IPAddress = LOCALHOST_IP;
		if ( port == INVALID_PORT )
			port = g_SSNetworkController.GetDefaultListeningPort();

		if ( g_SSNetworkController.ConnectToGame( IPAddress, port ) )
		{
			Logger::Log( "Connection request added", "SSGameLobby", LogSeverity::DEBUG_MSG );
			m_ConnectionState = ConnectionState::WaitingToConnect;
			m_ButtonBackEnterIP->SetEnabled( false );
			m_ButtonConnect->SetEnabled( false );
			m_ButtonConnect->SetEnabled( false );
			m_ButtonHost->SetEnabled( false );
			m_TextError->GetTextDefinitionRef().Text = "Connecting...";
		}
		else
		{
			m_TextError->GetTextDefinitionRef().Text = "Failed to add connection request";
			Logger::Log( "Failed to add connection request", "SSGameLobby", LogSeverity::ERROR_MSG );
		}
	}
	else
	{
		m_TextError->GetTextDefinitionRef().Text = "Invalid port supplied"; // TODODB: Is this correct error message?
		Logger::Log( "Invalid port supplied", "SSGameLobby", LogSeverity::WARNING_MSG );
	}
}

void SSGameLobby::HostGame()
{
	// INVALID_PORT means use default port
	if ( g_SSNetworkController.MakeHost( INVALID_PORT ) )
	{
		m_ConnectionState = ConnectionState::Hosting;
		TryChangeColour( 0, 0, false );
		g_Alliances.Initialize( m_MaxNrOfTeams );
		g_GameData.ResetSpawnPoints( );
		TryChangeTeam( 0, 0, false );
		m_ButtonConnect->SetEnabled( false );
		m_ButtonHost->SetEnabled( false );
		m_WindowEnterIP->ToggleOpen( );
		m_WindowLobby->ToggleOpen( );
		m_ButtonStartGame->SetVisible( true );
		UpdateSpawnPointButtons( );

		g_SSChat.InitializeChatWindow( 0, m_WindowLobby->GetSize().y / 2, m_WindowLobby->GetSize().x / 2, (m_WindowLobby->GetSize().y / 2) - m_HeightButton, 70, SSChat::CHATSTATE_LOBBY );
	}
	else
	{
		// TODODB: Better error message plox
		m_TextError->GetTextDefinitionRef().Text = "Failed to host game";
	}
}

void SSGameLobby::OnReadyButtonClicked( )
{
	if ( g_NetworkInfo.AmIHost() )
		ToggleReadyStatus( g_NetworkInfo.GetNetworkID() );

	if ( AreAnyClientsConnected( ) )
	{
		UserSignalMessage signalPacket = UserSignalMessage( UserSignalType::READY_TOGGLE, g_NetworkInfo.GetNetworkID() );
		g_NetworkInfo.AmIHost() ? g_PacketPump.SendToAll( signalPacket ) : g_PacketPump.Send( signalPacket );
	}
}

void SSGameLobby::OnDisconnection( const void* const disconnectingPlayerID )
{
	short disconnectingPlayer = *reinterpret_cast<const short*>( disconnectingPlayerID );
	if ( !g_NetworkInfo.AmIHost() && disconnectingPlayer == g_NetworkInfo.GetHostID() )
		BackFromLobby();
	if ( disconnectingPlayer != -1 )
	{
		if ( m_PlayersReady[disconnectingPlayer] )
			m_PlayersReady[disconnectingPlayer] = false;
		ResetColour( disconnectingPlayer );
		ResetTeam( disconnectingPlayer );
		ResetSpawnPoint( disconnectingPlayer );
		m_ConnectedPlayerEntries[ disconnectingPlayer ].IsActive = false;
	}
}

void SSGameLobby::OnConnectionSuccess( const void* const connectionData )
{
	const ConnectionData* const connection = reinterpret_cast<const ConnectionData* const>(connectionData);

	CallbackConfig* networkConfig = g_ConfigManager.GetConfig( "network.cfg" );
	networkConfig->SetString( "SavedIP", connection->IPAddress.GetPrintableAdress() );
	networkConfig->SaveFile( "network.cfg" );

	m_TextError->GetTextDefinitionRef().Text = "";

	m_ButtonBackEnterIP->SetEnabled( true );
	m_ButtonConnect->SetEnabled( true );
	m_ButtonHost->SetEnabled( true );
}
void SSGameLobby::OnConnectionFail( const void* const connectionData )
{
	m_ConnectionAttemptFailed = true;

	m_TextError->GetTextDefinitionRef().Text = "";

	m_ButtonBackEnterIP->SetEnabled( true );
	m_ButtonConnect->SetEnabled( true );
	m_ButtonHost->SetEnabled( true );
}

void SSGameLobby::BackFromEnterIP( )
{
	g_GameModeSelector.SwitchToGameMode( GameModeType::MainMenu );
}

void SSGameLobby::BackFromLobby( )
{
	// Reset GUI
	m_WindowLobby->ToggleOpen();
	m_WindowEnterIP->ToggleOpen();
	m_ButtonConnect->SetEnabled( true );
	m_ButtonHost->SetEnabled( true );

	// Reset systems
	if ( g_NetworkInfo.AmIHost() )
		g_NetworkEngine.StopListening();

	g_NetworkInfo.Reset();
	g_NetworkEngine.RequestDisconnection( -1 );
	g_PlayerData.Reset();
	g_SSNetworkController.Reset();
	Reset();
	
	for ( short i = 0; i < MAX_PLAYERS; ++i )
	{
		ResetColour( i );
		ResetTeam( i );
		ResetSpawnPoint( i );
		if ( m_ConnectedPlayerEntries[i].IsAI )
			ChangeReserveAI( i, false );
	}
	m_ConnectionState = ConnectionState::EnteringIP;
	g_GUI.CloseWindow( "Chat" );
}

void SSGameLobby::LoadLevels( )
{
	if ( !m_ComboBoxSelectLevel )
	{
		Logger::Log( "The level select combo box is not initialized", "SSGameLobby", LogSeverity::ERROR_MSG );
		return;
	}

	rVector<rString> directoryContent;
	FileUtility::GetListOfContentInDirectory( m_MapsFolder.c_str(), directoryContent );

	for ( auto& entry : directoryContent )
	{
		m_ComboBoxSelectLevel->AddItem( entry );
	}

	LoadLevelPreviews( directoryContent );
	if ( directoryContent.size( ) > 0 )
	{
		SetLevel( directoryContent.at( 0 ) );
	}
}

void SSGameLobby::LoadLevelPreviews( const rVector<rString> levelNames )
{
	m_SpawnPoints.clear();
	for ( auto& level : levelNames )
	{
		gfx::Texture* texture = tNew( gfx::Texture );
		rString levelPath = m_MapsFolder + level + "/" + level + m_LevelPreviewFormat;
		if ( texture->Init( levelPath.c_str(), gfx::TextureType::TEXTURE_2D ) )
			m_LevelTextures.emplace( level, texture );
		else
		{
			if ( texture->Init( m_DefaultLevelTexturePath.c_str(), gfx::TextureType::TEXTURE_2D ) )
				m_LevelTextures.emplace( level, texture );
			else
				Logger::Log( "Failed to load default level texture: " + m_DefaultLevelTexturePath, "SSGameLobby", LogSeverity::ERROR_MSG );
			Logger::Log( "Failed to load level texture: " + levelPath, "SSGameLobby", LogSeverity::ERROR_MSG );
		}

		// Load the scene file
		gfx::ObjectImporter importer;
		bool status = importer.LoadScene( level );

		// Could not load scene file
		if( status == false )
		{
			Logger::Log( "Failed to load level file: " + levelPath, "SSGameLobby", LogSeverity::ERROR_MSG );
		}

		// Get spawn points
		gfx::RoboScene scene;
		status = importer.GetSceneByName( scene, level );
		rVector <gfx::EntityObj> spawnPoints;
		spawnPoints = importer.GetSpawnPointsInScene( 0 );
		rVector<glm::vec2> spawnPointProcessed;
		for( auto& spawnPoint : spawnPoints )
		{
			spawnPointProcessed.push_back( glm::vec2( spawnPoint.Translation[0] / scene.Width, spawnPoint.Translation[2] / scene.Height ) );
		}
		m_SpawnPoints.emplace( level, spawnPointProcessed );

		rVector<gfx::EntityObj> resourceDrops;
		resourceDrops = importer.GetResourcesInScene( 0 );
		rVector<ResourceSpawn> resourceSpawnsProcessed;
		for ( auto& resource : resourceDrops )
		{
			if ( resource.Radius > 0.0f )
			{
				resourceSpawnsProcessed.push_back( ResourceSpawn{ 
					glm::vec2( resource.Translation[0] / scene.Width, resource.Translation[2] / scene.Height ),
					resource.GetDropRate( )} );
			}
		}
		m_ResourceSpawns.emplace( level, resourceSpawnsProcessed );	

		rVector<gfx::EntityObj> researchStations = importer.GetControlPointsInScene( 0 );
		rVector<glm::vec2> researchStationsProcessed;
		for ( auto& research : researchStations )
		{
			researchStationsProcessed.push_back( glm::vec2( research.Translation[0] / scene.Width, research.Translation[2] / scene.Height ) );
		}
		m_ResearchStations.emplace( level, researchStationsProcessed );
	}
}

void SSGameLobby::TryChangeColour( int index, short playerID, bool force ) 
{
	auto canChange = [this, &index, &playerID]( ) -> bool
	{
		int i = 0;
		for ( auto& playerEntry : m_ConnectedPlayerEntries )
		{
			if ( playerEntry.SelectedColourIndex == index && i != playerID )
				return false;
			++i;
		}
		return true;
	};
	if ( g_NetworkInfo.AmIHost( ) )
	{
		if ( canChange() )
		{
			SetColour( index, playerID );
			DEV( 
				if ( playerID == g_NetworkInfo.GetNetworkID() )
				{
					Logger::Log( "Broadcasting colour change to all clients. Player ID: " + rToString( playerID )
						+ ". Colour index: " + rToString(index), "SSGameLobby", LogSeverity::DEBUG_MSG );
				}
				else
				{
					Logger::Log( "Received colour change request from client. Broadcasting colour change to all clients. Player ID: " 
						+ rToString( playerID ) + ". Colour index: " + rToString(index), "SSGameLobby", LogSeverity::DEBUG_MSG );
				}
			);
			// Broadcast colour change to clients
			if ( AreAnyClientsConnected( ) )
				g_PacketPump.SendToAll( ColourChangeMessage( playerID, index, true ) );
		}
		else
		{
			index = GetUnusedColourIndex( playerID );
			DEV( 
				if ( playerID == g_NetworkInfo.GetNetworkID() )
				{
					Logger::Log( "You tried to set used colour as host. Giving you an unused one. Colour index: " + rToString( index ), "SSGameLobby", LogSeverity::DEBUG_MSG );
				}
				else
				{
					Logger::Log( "Received invalid colour change request from client. Broadcasting an unused one to all clients. Player ID: " 
						+ rToString( playerID ) + ". Colour index: " + rToString(index), "SSGameLobby", LogSeverity::DEBUG_MSG );
				}
			);
			SetColour( index, playerID );
			if ( AreAnyClientsConnected( ) )
				g_PacketPump.SendToAll( ColourChangeMessage( playerID, index, true ) );
		}
	}
	else
	{
		if ( force )
		{
			DEV( Logger::Log( "Received a forced colour change. Player ID: " + rToString( playerID ) 
						+ ". Colour index: " + rToString( index), "SSGameLobby", LogSeverity::DEBUG_MSG ) );
			SetColour( index, playerID );
		}
		else
		{
			DEV( Logger::Log( "Sending colour change request to server. Colour index: " + rToString( index ), "SSGameLobby", LogSeverity::DEBUG_MSG ) );
			g_PacketPump.Send( ColourChangeMessage( playerID, index, false ) );
		}
	}
}

void SSGameLobby::SetColour( int index, short playerID )
{
	m_ConnectedPlayerEntries[ playerID ].Colour->SetSelectedIndex( index == -1 ? 0 : index );
	m_ConnectedPlayerEntries[ playerID ].SelectedColourIndex = index;
	for ( auto& colourEntry : m_ColourEntries )
	{
		colourEntry.PlayerID = colourEntry.PlayerID == playerID ? -1 : colourEntry.PlayerID;
	}
	if ( index != -1 )
	{
		m_ColourEntries[ index ].PlayerID = playerID;
		g_GameData.SetPlayerColour( playerID, m_ColourEntries[ index ].Colour );
	}
	else
	{
		g_GameData.SetPlayerColour( playerID, glm::vec4(1.0f) );
	}
}

void SSGameLobby::ResetColour( short playerID )
{
	assert( playerID >= 0 && playerID < MAX_PLAYERS );
	m_ConnectedPlayerEntries[ playerID ].Colour->SetSelectedIndex( 0 );
	m_ConnectedPlayerEntries[ playerID ].SelectedColourIndex = -1;
	for ( auto& colours : m_ColourEntries )
	{
		if ( colours.PlayerID == playerID )
			colours.PlayerID = -1;
	}
}

int SSGameLobby::GetUnusedColourIndex( short playerID )
{
	int toReturn = -1;
	for ( unsigned int i = 0; i < m_ColourEntries.size(); ++i )
	{
		if ( m_ColourEntries.at( i ).PlayerID == -1 && toReturn == -1 )
		{
			toReturn = static_cast<int>( i );
		}
		// Prioritize using same index if possible
		if ( playerID == m_ColourEntries.at( i ).PlayerID )
		{
			toReturn = static_cast<int>( i );
			break;
		}
	}
	return toReturn;
}

void SSGameLobby::TryChangeTeam( int index, short playerID, bool force )
{
	if ( g_NetworkInfo.AmIHost( ) )
	{
		if ( AreAnyClientsConnected( ) )
			g_PacketPump.SendToAll( TeamChangeMessage( playerID, index, true ) );
		SetTeam( index, playerID );
	}
	else
	{
		if ( force )
			SetTeam( index, playerID );
		else
		{
			g_PacketPump.Send( TeamChangeMessage( playerID, index, false ));
		}
	}
}

void SSGameLobby::SetTeam( int index, short playerID )
{
	assert( playerID >= 0 && playerID < MAX_PLAYERS );
	g_Alliances.RemoveFromAlliance( playerID );
	m_ConnectedPlayerEntries[playerID].SelectedTeamIndex = index;
	if ( index != -1 )
	{
		g_Alliances.AddToAlliance( index, playerID );
		m_ConnectedPlayerEntries[playerID].Team->SetSelectedIndex( index );
		DEV( Logger::Log( "Setting team for player " + rToString( playerID ) + " to Team " + rToString( index + 1 ), "SSGameLobby", LogSeverity::DEBUG_MSG ) );
	}
}

void SSGameLobby::ResetTeam( short playerID )
{
	assert( playerID >= 0 && playerID < MAX_PLAYERS );
	m_ConnectedPlayerEntries[playerID].SelectedTeamIndex = -1;
	m_ConnectedPlayerEntries[playerID].Team->SetSelectedIndex( 0 );
	g_Alliances.RemoveFromAlliance( playerID );
}

void SSGameLobby::CreateSpawnPointButtons( )
{
	g_Script.Register( m_ScriptSpawnPointSelect.c_str(), [this](IScriptEngine* script) -> int
			{
				int buttonID = script->PopInt( );
				short playerID = g_NetworkInfo.GetNetworkID( );
				TryChangeSpawnPoint( buttonID, playerID, false );
				return 0;
			} );
	m_ButtonsSpawnPoints.clear( );
	SpawnPointButton spawnPointButton;
	for ( int i = 0; i < m_MaxNrOfSpawnPoints; ++i )
	{
		spawnPointButton.Button = g_GUI.AddButton( "", GUI::Rectangle(0, 0, m_SizeSpawnPointButton, m_SizeSpawnPointButton ), m_WindowNameLevel );
		spawnPointButton.Button->SetClickScript( m_ScriptSpawnPointSelect + "(" + rToString( i ) + ")" );
		spawnPointButton.Button->GetBackgroundRef().Texture = m_TextureNameSpawnPointButton;
		spawnPointButton.PlayerID = -1;
		m_ButtonsSpawnPoints.push_back( spawnPointButton );
	}
}

void SSGameLobby::UpdateSpawnPointButtons( )
{
	rString levelName =  m_ComboBoxSelectLevel->GetTextDefinitionRef().Text;
	auto spawnPointIt = m_SpawnPoints.find( levelName );
	if ( spawnPointIt == m_SpawnPoints.end( ) )
	{
		Logger::Log( "Failed to find spawnpoints for level: " + levelName, "SSGameLobby", LogSeverity::ERROR_MSG );
		return;
	}
	rVector<glm::vec2>& spawnPoints = spawnPointIt->second;
	int i = 0;
	assert( m_ButtonsSpawnPoints.size() >= spawnPoints.size( ) );
	for ( auto& button : m_ButtonsSpawnPoints )
	{
		button.Button->SetVisible( false );
	}
	m_CurrentLevelNrOfSpawns = spawnPoints.size( );
	for ( auto& spawnPoint : spawnPoints )
	{
		m_ButtonsSpawnPoints.at( i ).Button->SetPosition(
				static_cast<int>( spawnPoint.x * m_SizeLevelWindow ) - m_SizeSpawnPointButton / 2, static_cast<int>( spawnPoint.y * m_SizeLevelWindow ) - m_SizeSpawnPointButton / 2 );
		m_ButtonsSpawnPoints.at( i ).Button->SetVisible( true );
		++i;
	}
	UpdateLevelElements( levelName );
}

void SSGameLobby::TryChangeSpawnPoint( int index, short playerID, bool force )
{
	if ( g_NetworkInfo.AmIHost( ) )
	{
		if ( static_cast<unsigned int>( index ) >= m_ButtonsSpawnPoints.size() )
		{
			Logger::Log( "Tried to select invalid spawn point, syncronisation can not be guaranteed", "SSGameLobby", LogSeverity::ERROR_MSG );
			ResetSpawnPoint( playerID );
			if ( AreAnyClientsConnected( ) )
				g_PacketPump.SendToAll( SpawnPointChangeMessage( playerID, -1, true ) );
		}
		else
		{
			SpawnPointButton& spawnButton = m_ButtonsSpawnPoints.at( index );
			if ( spawnButton.PlayerID == -1 ) // -1 means it is free for the taking
			{
				short newPlayerID = playerID;
				if ( playerID == g_NetworkInfo.GetHostID( ) )
				{
					newPlayerID = GetNextSpawnToSetAsHost( );
				}
				ResetSpawnPoint( newPlayerID );
				SetSpawnPoint( index, newPlayerID );
				if ( AreAnyClientsConnected( ) )
					g_PacketPump.SendToAll( SpawnPointChangeMessage( newPlayerID, index, true ) ); // Let player have the spawnpoint
				DEV( Logger::Log( "Broadcasting spawn point: " + rToString( index ) + " for player: " + rToString( newPlayerID ), "SSGameLobby", LogSeverity::DEBUG_MSG ) );
			}
			else if ( spawnButton.PlayerID == playerID ) // Same player already on this spawn point
			{
				if ( playerID == g_NetworkInfo.GetNetworkID( ) ) // Host spawn reset, make sure we try setting that one next
					m_SpawnPointToSetAsHost = spawnButton.PlayerID;
				// Reset to no selection
				ResetSpawnPoint( playerID );
				if ( AreAnyClientsConnected( ) )
					g_PacketPump.SendToAll( SpawnPointChangeMessage( -1, index, true ) ); // Turn the players spawnpoint off
				DEV( Logger::Log( "Resetting spawn point for player: " + rToString( playerID ), "SSGameLobby", LogSeverity::DEBUG_MSG ) );
			}
			else
			{
				// If player is host, reset AI position of the occupied spawn is owned by an AI player
				if ( m_ConnectedPlayerEntries[spawnButton.PlayerID].IsAI && playerID == g_NetworkInfo.GetHostID( ) )
				{
					m_SpawnPointToSetAsHost = spawnButton.PlayerID;
					ResetSpawnPoint( spawnButton.PlayerID );
					if ( AreAnyClientsConnected( ) )
						g_PacketPump.SendToAll( SpawnPointChangeMessage( -1, index, true ) ); // Turn the players spawnpoint off
					DEV( Logger::Log( "Resetting spawn point for player: " + rToString( playerID ), "SSGameLobby", LogSeverity::DEBUG_MSG ) );
				}
				else
				{
					DEV( Logger::Log( "Player: " + rToString( playerID ) + " tried to select spawnpoint: " + rToString( index ) + " but it was already taken by"
							+ rToString( spawnButton.PlayerID ), "SSGameLobby", LogSeverity::DEBUG_MSG ) );
				}
			}
		}
	}
	else
	{
		if ( force )
		{
			ResetSpawnPoint( playerID );
			SetSpawnPoint( index, playerID );
			DEV( Logger::Log( "Recieved forced update of spawnpoint for player: " + rToString( playerID ) + " to spawnpoint: " + rToString( index ),
						"SSGameLobby", LogSeverity::DEBUG_MSG) );
		}
		else
		{
			g_PacketPump.Send( SpawnPointChangeMessage( playerID, index, false ) ); // Ask to change spawnpoint
			DEV( Logger::Log( "Sending request to change your spawnpoint to: " + rToString( index ), "SSGameLobby", LogSeverity::DEBUG_MSG) );
		}
	}
}

void SSGameLobby::SetSpawnPoint( int index, short playerID )
{
	assert( index >= 0 && index < m_ButtonsSpawnPoints.size( ) );
	m_ButtonsSpawnPoints.at( index ).PlayerID = playerID;
	g_GameData.SetPlayerSpawnPoint( index, playerID );
}

void SSGameLobby::ResetSpawnPoint( short playerID )
{
	for ( auto& spawn : m_ButtonsSpawnPoints )
	{
		spawn.PlayerID = spawn.PlayerID == playerID ? -1 : spawn.PlayerID;
	}
	g_GameData.SetPlayerSpawnPoint( -1, playerID );
}

void SSGameLobby::ChangeSpawnCount( int count )
{
	m_NumberOfSpawns = count;
	if (g_NetworkInfo.AmIHost())
	{
		if ( AreAnyClientsConnected( ) )
			g_PacketPump.SendToAll(SpawnCountChangeMessage(m_NumberOfSpawns));
	}
	g_SSAI.SetSpawnCount(m_NumberOfSpawns);
}

void SSGameLobby::ChangeReserveAI( short playerID, bool on )
{
	if ( on )
	{
		Logger::Log( "Reserving slot: " + rToString( playerID ) + " for AI", "SSGameLobby", LogSeverity::DEBUG_MSG );
		g_NetworkEngine.ReserveSlot( playerID );
		m_ConnectedPlayerEntries[playerID].Name->GetTextDefinitionRef().Text = "AI " + rToString( playerID );
		m_ConnectedPlayerEntries[playerID].IsAI = true;
		m_ConnectedPlayerEntries[playerID].IsActive = true;
	}
	else
	{
		Logger::Log( "Unreserving slot: " + rToString( playerID ) + " for AI", "SSGameLobby", LogSeverity::DEBUG_MSG );
		g_NetworkEngine.UnreserveSlot( playerID );
		m_ConnectedPlayerEntries[ playerID ].IsAI = false;
		m_ConnectedPlayerEntries[ playerID ].IsActive = false;
	}
	if ( g_NetworkInfo.AmIHost( ) && m_ConnectionState == ConnectionState::Hosting )
	{
		if ( AreAnyClientsConnected( ) )
			g_PacketPump.SendToAll( ReserveAIMessage( playerID, on ) );
	}
}

short SSGameLobby::GetNextSpawnToSetAsHost( )
{
	// Loop through each ai or host and find someone with no spawn point
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		int index = ( m_SpawnPointToSetAsHost + i ) % MAX_PLAYERS;
		ConnectedPlayer& player = m_ConnectedPlayerEntries[index];
		if ( ( player.IsAI && player.IsActive ) || player.PlayerID == g_NetworkInfo.GetHostID( ) )
		{
			bool found = false;
			for ( auto& spawnButton : m_ButtonsSpawnPoints )
			{
				if ( spawnButton.PlayerID == player.PlayerID )
				{
					found = true;
					break;
				}
			}
			if ( !found )
			{
				return index;
			}
		}
	}

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		int index = ( m_SpawnPointToSetAsHost + i ) % MAX_PLAYERS;
		if ( ( m_ConnectedPlayerEntries[index].IsAI && m_ConnectedPlayerEntries[index].IsActive ) || index == g_NetworkInfo.GetNetworkID( ) )
		{
			m_SpawnPointToSetAsHost = index + 1;
			return index;
		}
	}
	return -1;
}

bool SSGameLobby::ArePlayersOnDifferentTeams( ) const
{
	int team = m_ConnectedPlayerEntries[0].SelectedTeamIndex;
	for ( auto& player : m_ConnectedPlayerEntries )
	{
		if ( player.IsActive && player.SelectedTeamIndex != team )
			return true;
	}
	return false;
}

bool SSGameLobby::AreAnyClientsConnected( ) const
{
	return g_NetworkInfo.GetConnectedPlayerCount( ) > 0;
}

void SSGameLobby::UpdateLevelElements( const rString& level )
{
	g_GUI.DeleteObject( m_NameWindowLevelElements );
	m_WindowLevelElements = g_GUI.AddWindow( m_NameWindowLevelElements,
			GUI::Rectangle( 0, 0,
				m_WindowLevel->GetSize( ).x, m_WindowLobby->GetSize( ).y ),
			m_WindowNameLevel );
	m_WindowLevelElements->ToggleOpen( );

	auto researchStations = m_ResearchStations.find( level );
	if ( researchStations != m_ResearchStations.end( ) )
	{
		GUI::Sprite* sprite = nullptr;
		for ( auto& station : researchStations->second )
		{
			sprite = g_GUI.AddSprite( "", GUI::SpriteDefinition( m_NameTextureResearchStations,
					static_cast<int>( station.x * m_SizeLevelWindow ) - CONTROL_POINT_SPRITE_SIZE / 2,
					static_cast<int>( station.y * m_SizeLevelWindow ) - CONTROL_POINT_SPRITE_SIZE / 2,
				   	CONTROL_POINT_SPRITE_SIZE, CONTROL_POINT_SPRITE_SIZE ), m_NameWindowLevelElements );
			sprite->GetSpriteDefinitionRef().Colour *= m_FactorResearchStationDarkening;
		}
	}

	auto resourceDrops = m_ResourceSpawns.find( level );
	if ( resourceDrops != m_ResourceSpawns.end( ) )
	{
		GUI::Sprite* sprite = nullptr;
		for ( auto& drop : resourceDrops->second )
		{
			sprite = g_GUI.AddSprite( "", GUI::SpriteDefinition( m_NameTextureResourceDrops,
					static_cast<int>( drop.Position.x * m_SizeLevelWindow ) - RESOURCE_SPAWN_SPRITE_SIZE / 2,
					static_cast<int>( drop.Position.y * m_SizeLevelWindow ) - RESOURCE_SPAWN_SPRITE_SIZE / 2,
				   	RESOURCE_SPAWN_SPRITE_SIZE, RESOURCE_SPAWN_SPRITE_SIZE ), m_NameWindowLevelElements );
			sprite->GetSpriteDefinitionRef().Colour = g_SSResourceManager.GetResourceSpawnTypeColour( drop.Type ) * m_FactorResourceDarkening;
		}
	}
}
