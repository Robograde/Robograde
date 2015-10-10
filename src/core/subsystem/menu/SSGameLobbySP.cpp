/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#include "SSGameLobbySP.h"
#include "../../CompileFlags.h"
#include <utility/Colours.h>
#include <utility/Logger.h>
#include <utility/ConfigManager.h>
#include <gfx/ObjectImporter.h>
#include "../input/SSKeyBinding.h"
#include "../../utility/GameModeSelector.h"
#include "../../utility/Alliances.h"
#include "../../utility/GameData.h"
#include "../gamelogic/SSSceneLoader.h"
#include "../gamelogic/SSSceneLoader.h"
#include "../gamelogic/SSAI.h"
#include "../gamelogic/SSResourceManager.h"
#include "../gui/SSMiniMap.h"

SSGameLobbySP& SSGameLobbySP::GetInstance( )
{
	static SSGameLobbySP instance;
	return instance;
}

void SSGameLobbySP::Startup( )
{
	DEV( g_Script.Register( m_ScriptAIOnly.c_str( ), [this]( IScriptEngine* ) -> int
	{
		m_IsAIOnly = !m_IsAIOnly;
		Logger::Log( rString("Toggled AI only: ") + rString(m_IsAIOnly ? "On" : "Off"), "SSGameLobby", LogSeverity::DEBUG_MSG );
		return 0;
	} ) );
	CreateMainWindow( );
	CreateLevelSelection( );
	CreateGameSettings( );
	CreatePlayerSlots( );
	g_GameData.ResetSpawnPoints( );
}

void SSGameLobbySP::Shutdown( )
{
	m_CurrentNumberOfPlayers = m_MinimumPlayerSlots;
	m_InitialSquadSize = m_InitialInitalSquadSize;
	g_GUI.DeleteObject( m_NameWindowLobby );
	m_PlayerSlots.clear( );

	for ( auto& levelTextures : m_LevelTextures )
	{
		tDelete( levelTextures.second );
	}

	m_LevelTextures.clear( );
	m_ColourEntries.clear( );
	DEV( m_IsAIOnly = false; );
}

void SSGameLobbySP::UpdateUserLayer( const float deltaTime )
{
	if( g_SSKeyBinding.ConsumeFromPressStack( ACTION_GUI_ABORT ) )
	{
		g_GameModeSelector.SwitchToGameMode( GameModeType::MainMenu );
	}

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
			int colourIndex = m_PlayerSlots.at( spawn.PlayerID ).SelectedColourIndex;
			assert( colourIndex >= 0 && colourIndex < m_ColourEntries.size() );
			spawn.Button->SetColour( m_ColourEntries.at( colourIndex ).Colour );
		}
	}
	m_ButtonStartGame->SetEnabled( static_cast<size_t>( m_CurrentNumberOfPlayers ) <= m_CurrentLevelNrOfSpawns
			&& ArePlayersOnDifferentTeams( )); // Don't allow all players to be on same team
}

void SSGameLobbySP::CreateMainWindow( )
{
	glm::ivec2 parentSize = g_GUI.GetWindow( m_NameWindowParent )->GetSize( );
	g_GUI.UseFont( m_Font );
	m_WindowLobby = g_GUI.AddWindow( m_NameWindowLobby, GUI::Rectangle( 0, 0, parentSize.x, parentSize.y ), m_NameWindowParent );
	m_WindowLobby->ToggleOpen( );
	glm::ivec2 lobbySize = m_WindowLobby->GetSize( );
	g_Script.Register( m_ScriptStartGame.c_str( ), [this](IScriptEngine*) -> int
												   {
		StartGame( );
		return 0;
	} );
	m_ButtonStartGame = g_GUI.AddButton( "", GUI::Rectangle( lobbySize.x - m_ButtonWidth, lobbySize.y - m_ButtonHeight, m_ButtonWidth, m_ButtonHeight ), m_NameWindowLobby );
	m_ButtonStartGame->GetTextDefinitionRef( ).Text = "Start";
	m_ButtonStartGame->GetBackgroundRef( ).Texture = m_TextureNameButtons;
	m_ButtonStartGame->SetClickScript( m_ScriptStartGame + "()" );
	m_ButtonBack = g_GUI.AddButton( "", GUI::Rectangle( 0, lobbySize.y - m_ButtonHeight, m_ButtonWidth, m_ButtonHeight ), m_NameWindowLobby );
	m_ButtonBack->GetTextDefinitionRef( ).Text = "Back";
	m_ButtonBack->SetClickScript( "SwitchGameMode( 'mainmenu' )" );
	m_ButtonBack->GetBackgroundRef( ).Texture = m_TextureNameButtons;
}

void SSGameLobbySP::CreateGameSettings( )
{
	g_Script.Register( m_ScriptChangeInitialSquadSize.c_str( ), [this]( IScriptEngine* script ) -> int
																{
		m_InitialSquadSize += script->PopInt( );
		m_InitialSquadSize = std::min( m_MaximumInitialSquadSize, std::max( m_InitialSquadSize, m_MinimumInitialSquadSize ) ); // Clamp initial squad size
		m_BtnSpawnCountVal->SetText( rToString( m_InitialSquadSize ) );
		return 0;
	} );

	glm::ivec2 sizeLobby = m_WindowLobby->GetSize( );
	int gpsX = sizeLobby.x / 2;
	int gpsY = sizeLobby.y / 2;

	int gpsW = sizeLobby.x / 2;
	int gpsH = sizeLobby.y / 2;

	int gpsDiff = m_HeightComboBoxSelectLevel;

	int gpsStartCountW = gpsW - gpsDiff * 4 - gpsDiff * 3;

	m_WindowGamePlaySettings = g_GUI.AddWindow(m_WindowNameGamePlaySettings, GUI::Rectangle(gpsX, gpsY, gpsW, gpsH - m_ButtonHeight), m_NameWindowLobby, true);
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
	m_BtnSpawnCountDec->SetClickScript(m_ScriptChangeInitialSquadSize + "(-1)");

	// | Starting units count | - | X |
	m_BtnSpawnCountVal = g_GUI.AddButton("GamePlaySettingStartCountVal", GUI::Rectangle(gpsStartCountW + gpsDiff * 2, gpsDiff, gpsDiff, gpsDiff), m_WindowNameGamePlaySettings);
	m_BtnSpawnCountVal->GetBackgroundRef().Texture = m_TextureNameButtons;
	m_BtnSpawnCountVal->SetText("X");
	m_BtnSpawnCountVal->SetEnabled(false);
	m_BtnSpawnCountVal->SetText( rToString( m_InitialSquadSize ) );

	// | Starting units count | - | X | + |
	m_BtnSpawnCountInc = g_GUI.AddButton("GamePlaySettingStartCountModInc", GUI::Rectangle(gpsStartCountW + gpsDiff * 3, gpsDiff, gpsDiff, gpsDiff), m_WindowNameGamePlaySettings);
	m_BtnSpawnCountInc->GetBackgroundRef().Texture = m_TextureNameButtons;
	m_BtnSpawnCountInc->SetText("+");
	m_BtnSpawnCountInc->SetClickScript(m_ScriptChangeInitialSquadSize + "(1)");
}

void SSGameLobbySP::CreatePlayerSlots( )
{
	// Script callbacks
	g_Script.Register( m_ScriptNameColourChange.c_str(), [this](IScriptEngine* script) -> int
			{
				int playerID = script->PopInt( );
				int selectedIndex = m_PlayerSlots.at( playerID ).Colour->GetSelectedIndex( );
				TryChangeColour( selectedIndex, playerID );
				return 0;
			} );

	// Player colours
	m_ColourEntries.push_back( ColourEntry	{ "Green",	Colours::KINDA_GREEN, Colours::KINDA_GREEN	, -1 } );
	m_ColourEntries.push_back( ColourEntry	{ "Red", 	Colours::KINDA_RED 	, Colours::KINDA_RED	, -1 } );
	m_ColourEntries.push_back( ColourEntry	{ "Blue", 	Colours::KINDA_BLUE	, Colours::KINDA_BLUE	, -1 } );
	m_ColourEntries.push_back( ColourEntry	{ "Pink",	Colours::KINDA_PINK	, Colours::KINDA_PINK	, -1 } );
	m_ColourEntries.push_back( ColourEntry	{ "Yellow", Colours::VERY_YELLOW, Colours::VERY_YELLOW	, -1 } );
	m_ColourEntries.push_back( ColourEntry	{ "LtBlue", Colours::LTBLUE		, Colours::LTBLUE		, -1 } );
	// Text colour modified
	m_ColourEntries.push_back( ColourEntry	{ "Black",	Colours::BLACK		, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), -1 } );

	glm::ivec2 parentSize = m_WindowLobby->GetSize( );
	m_WindowPlayerSlots = g_GUI.AddWindow( m_NameWindowPlayerSlots, GUI::Rectangle( 0, 0, parentSize.x / 2, parentSize.y / 2 ), m_NameWindowLobby );
	m_WindowPlayerSlots->ToggleOpen( );
	glm::ivec2 windowSize = m_WindowPlayerSlots->GetSize( );

	// Create slot entries
	m_PlayerSlots.clear( );
	m_PlayerSlots.resize( m_MaximumPlayerSlots );
	int entryY = m_HeightPlayerEntry * (m_MaximumPlayerSlots - 1) + m_TopPadding;
	for ( int i = m_MaximumPlayerSlots -1 ; i >= 0; --i )
	{
		PlayerSlot& slot = m_PlayerSlots.at( i );
		int x = windowSize.x - m_WidthTeam;
		// Name
		slot.Name = g_GUI.AddText( "", GUI::TextDefinition( "", m_LeftPadding, entryY, m_WidthName, m_HeightPlayerEntry ), m_NameWindowPlayerSlots );
		slot.Name->GetTextDefinitionRef().Text = "AI Player " + rToString( i );
		slot.Name->SetVisible( false );
		// Team
		slot.Team = g_GUI.AddComboBox( "", GUI::Rectangle( x, entryY, m_WidthTeam, m_HeightPlayerEntry ), m_NameWindowPlayerSlots );
		for ( int j = 0; j < m_MaxNrOfTeams; ++j )
		{
			slot.Team->AddItem( "Team " + rToString( j + 1 ) );
		}
		slot.Team->GetBackgroundRef( ).Texture = m_TextureNameComboBoxes;
		slot.Team->SetItemBackgroundImage( m_TextureNameComboBoxes );
		slot.Team->SetVisible( false );
		slot.Team->SetEnabled( false );
		slot.Team->SetSelectedIndex( i );
		// Colour
		x -= m_WidthColour;
		slot.Colour = g_GUI.AddComboBox( "", GUI::Rectangle( x, entryY, m_WidthColour, m_HeightPlayerEntry ), m_NameWindowPlayerSlots );
		for( auto& colourEntry : m_ColourEntries )
		{
			slot.Colour->AddItem( colourEntry.Text );
			slot.Colour->GetItem( slot.Colour->GetNumItems() - 1 ).Text.Colour = colourEntry.TextColour;
		}
		slot.Colour->SetClickScript( m_ScriptNameColourChange + "(" + rToString( i ) + ")" );
		slot.Colour->GetBackgroundRef( ).Texture = m_TextureNameComboBoxes;
		slot.Colour->SetItemBackgroundImage( m_TextureNameComboBoxes );
		slot.Colour->SetVisible( false );
		slot.Colour->SetEnabled( false );
		// Default to be AI
		slot.IsAI = true;

		entryY -= m_HeightPlayerEntry;
	}
	for ( int i = 0; i < m_CurrentNumberOfPlayers; ++i )
	{
		TryChangeColour( 0, i ); // Assign a colour
	}

	CallbackConfig* cfg = g_ConfigManager.GetConfig( m_PathConfigName );
	rString name = cfg->GetString( "PlayerName", "ANON", "The players ingame nickname" );
	// Default first slot to be player, show all entry settings
	m_PlayerSlots.at( 0 ).IsAI = false;
	m_PlayerSlots.at( 0 ).Name->GetTextDefinitionRef().Text = name;
	m_PlayerSlots.at( 0 ).Name->SetVisible( true );
	m_PlayerSlots.at( 0 ).Colour->SetVisible( true );
	m_PlayerSlots.at( 0 ).Team->SetVisible( true );
	m_PlayerSlots.at( 0 ).Colour->SetEnabled( true );
	m_PlayerSlots.at( 0 ).Team->SetEnabled( true );
	m_PlayerSlots.at( 0 ).Colour->SetSelectedIndex( 0 );

	// AI Buttons
	g_Script.Register( m_ScriptNameAddAI.c_str(), [this] (IScriptEngine* ) -> int
			{
				AddAI( );
				return 0;
			} );
	g_Script.Register( m_ScriptNameRemoveAI.c_str(), [this] (IScriptEngine* ) -> int
			{
				RemoveAI( );
				return 0;
			} );
	m_ButtonAddAI = g_GUI.AddButton( "", GUI::Rectangle( windowSize.x - m_ButtonWidth, windowSize.y - m_ButtonHeight, m_ButtonWidth, m_ButtonHeight ), m_NameWindowPlayerSlots );
	m_ButtonAddAI->GetTextDefinitionRef().Text = "Add";
	m_ButtonAddAI->SetClickScript( m_ScriptNameAddAI + "()" );
	m_ButtonAddAI->SetBackgroundImage( m_TextureNameButtons );
	m_ButtonRemoveAI = g_GUI.AddButton( "", GUI::Rectangle( windowSize.x - m_ButtonWidth * 2, windowSize.y - m_ButtonHeight, m_ButtonWidth, m_ButtonHeight ), m_NameWindowPlayerSlots );
	m_ButtonRemoveAI->GetTextDefinitionRef().Text = "Remove";
	m_ButtonRemoveAI->SetClickScript( m_ScriptNameRemoveAI + "()" );
	m_ButtonRemoveAI->SetBackgroundImage( m_TextureNameButtons );
}

void SSGameLobbySP::CreateLevelSelection( )
{
	// Script callbacks
	g_Script.Register( m_ScriptClickLevelChange.c_str( ), [this]( IScriptEngine* )
					   {
		SetLevel( m_ComboBoxSelectLevel->GetTextDefinitionRef( ).Text );
		return 0;
	} );

	glm::ivec2 sizeLobby = m_WindowLobby->GetSize( );
	m_WindowSelectLevel = g_GUI.AddWindow( m_NameWindowSelectLevel, GUI::Rectangle( sizeLobby.x / 2, 0, sizeLobby.x / 2, sizeLobby.y / 2 ),
										   m_NameWindowLobby, true );
	glm::ivec2 slSize = m_WindowSelectLevel->GetSize( );
	m_WindowSelectLevel->ToggleOpen( );
	m_ComboBoxSelectLevel = g_GUI.AddComboBox( "", GUI::Rectangle( 0, m_WindowSelectLevel->GetSize( ).y - m_HeightComboBoxSelectLevel,
																   m_WindowSelectLevel->GetSize( ).x, m_HeightComboBoxSelectLevel ),
											   m_NameWindowSelectLevel );
	m_ComboBoxSelectLevel->SetBackgroundImage( m_TextureNameComboBoxes );
	m_ComboBoxSelectLevel->SetItemBackgroundImage( m_TextureNameComboBoxes );
	m_ComboBoxSelectLevel->SetClickScript( m_ScriptClickLevelChange + "()" );

	m_SizeLevelWindow = std::min( slSize.x, slSize.y - m_HeightComboBoxSelectLevel );
	m_WindowLevel = g_GUI.AddWindow( m_NameWindowLevel,
									 GUI::Rectangle( ( slSize.x / 2 ) - ( m_SizeLevelWindow / 2 ), 0, m_SizeLevelWindow, m_SizeLevelWindow ),
									 m_NameWindowSelectLevel );
	m_WindowLevel->ToggleOpen( );
	m_SpriteLevelSelected = g_GUI.AddSprite( "", GUI::SpriteDefinition( "", 0, 0, m_SizeLevelWindow, m_SizeLevelWindow ), m_NameWindowLevel );

	CreateSpawnPointButtons( );
	LoadLevels( );
}

void SSGameLobbySP::StartGame( )
{
	g_GameModeSelector.SwitchToGameMode( m_GameModeToChangeTo );

	g_Alliances.Initialize( m_MaxNrOfTeams );
	for ( int i = 0; i < m_CurrentNumberOfPlayers; ++i )
	{
		g_Alliances.AddToAlliance( m_PlayerSlots.at( i ).Team->GetSelectedIndex( ), i );
		DEV(
			g_GameData.AddPlayer( i, m_PlayerSlots.at( i ).IsAI || m_IsAIOnly );
		);
		NON_DEV( g_GameData.AddPlayer( i, m_PlayerSlots.at( i ).IsAI ); );
	}
	g_SSMiniMap.SetMiniMapTexture( m_MapsFolder + m_ComboBoxSelectLevel->GetTextDefinitionRef( ).Text + "/" +
								   m_ComboBoxSelectLevel->GetTextDefinitionRef( ).Text + m_LevelPreviewFormat );
	g_SSAI.SetSpawnCount( m_InitialSquadSize );
	g_SSSceneLoader.SetSceneToBeLoaded( m_ComboBoxSelectLevel->GetTextDefinitionRef().Text );
}

void SSGameLobbySP::LoadLevels( )
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

void SSGameLobbySP::LoadLevelPreviews( const rVector<rString> levelNames )
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
				resource.GetDropRate( ) } );
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

void SSGameLobbySP::SetLevel( const rString& level )
{
	m_ComboBoxSelectLevel->GetTextDefinitionRef().Text = level;
	UpdateSpawnPointButtons( );
	
	auto levelTexture = m_LevelTextures.find( level );
	if ( levelTexture != m_LevelTextures.end( ) )
	{
		m_SpriteLevelSelected->GetSpriteDefinitionRef().TextureHandle = levelTexture->second->GetHandle( );
		g_SSMiniMap.SetMiniMapTexture( levelTexture->second->GetFilename() );
	}
	else
	{
		Logger::Log( "Failed to set texture sprite for level: " + level, "SSGameLobby", LogSeverity::ERROR_MSG );
	}	
}

void SSGameLobbySP::CreateSpawnPointButtons( )
{
	g_Script.Register( m_ScriptSpawnPointSelect.c_str(), [this](IScriptEngine* script) -> int
		{
			int buttonID = script->PopInt( );

			TryChangeSpawnPoint( buttonID );
			return 0;
		} );
	m_ButtonsSpawnPoints.clear( );
	SpawnPointButton spawnPointButton;
	for ( int i = 0; i < m_MaxNrOfSpawnPoints; ++i )
	{
		spawnPointButton.Button = g_GUI.AddButton( "", GUI::Rectangle(0, 0, m_SizeSpawnPointButton, m_SizeSpawnPointButton ), m_NameWindowLevel );
		spawnPointButton.Button->SetClickScript( m_ScriptSpawnPointSelect + "(" + rToString( i ) + ")" );
		spawnPointButton.Button->GetBackgroundRef().Texture = m_TextureNameSpawnPointButton;
		spawnPointButton.PlayerID = -1;
		m_ButtonsSpawnPoints.push_back( spawnPointButton );
	}
}

void SSGameLobbySP::UpdateSpawnPointButtons( )
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
				static_cast<int>( spawnPoint.x * m_SizeLevelWindow - m_SizeSpawnPointButton / 2 ), static_cast<int>( spawnPoint.y * m_SizeLevelWindow - m_SizeSpawnPointButton / 2 ) );
		m_ButtonsSpawnPoints.at( i ).Button->SetVisible( true );
		++i;
	}
	UpdateLevelElements( levelName );
}

void SSGameLobbySP::TryChangeSpawnPoint( int index )
{
	if ( static_cast<unsigned int>( index ) >= m_ButtonsSpawnPoints.size() )
	{
		ResetSpawnPoint( m_CurrentSpawnPointToSet );
	}
	else
	{
		SpawnPointButton& spawnButton = m_ButtonsSpawnPoints.at( index );
		if ( spawnButton.PlayerID == -1 ) // -1 means it is free for the taking
		{
			ResetSpawnPoint( m_CurrentSpawnPointToSet );
			SetSpawnPoint( index, m_CurrentSpawnPointToSet );
			++m_CurrentSpawnPointToSet;
			m_CurrentSpawnPointToSet = m_CurrentSpawnPointToSet < m_CurrentNumberOfPlayers ? m_CurrentSpawnPointToSet : 0;
		}
		else if ( spawnButton.PlayerID == m_CurrentSpawnPointToSet ) // Same player already on this spawn point
		{
			// Reset to no selection
			ResetSpawnPoint( m_CurrentSpawnPointToSet );
		}
		else
		{
			m_CurrentSpawnPointToSet = spawnButton.PlayerID;
			ResetSpawnPoint( spawnButton.PlayerID );
		}
	}
}

void SSGameLobbySP::SetSpawnPoint( int index, short playerID )
{
	assert( index >= 0 && index < m_ButtonsSpawnPoints.size( ) );
	m_ButtonsSpawnPoints.at( index ).PlayerID = playerID;
	g_GameData.SetPlayerSpawnPoint( index, playerID );
}

void SSGameLobbySP::ResetSpawnPoint( short playerID )
{
	for ( auto& spawn : m_ButtonsSpawnPoints )
	{
		spawn.PlayerID = spawn.PlayerID == playerID ? -1 : spawn.PlayerID;
	}
	g_GameData.SetPlayerSpawnPoint( -1, playerID );
}

void SSGameLobbySP::TryChangeColour( int index, short slotIndex )
{
	int i = 0;
	bool canChange = true;
	for ( auto& playerEntry : m_PlayerSlots )
	{
		if ( playerEntry.SelectedColourIndex == index && i != slotIndex )
			canChange = false;
		++i;
	}
	if ( canChange )
	{
		SetColour( index, slotIndex );
	}
	else
	{
		index = GetUnusedColourIndex( slotIndex );
		SetColour( index, slotIndex );
	}
}

void SSGameLobbySP::SetColour( int index, short slotIndex )
{
	m_PlayerSlots.at( slotIndex ).Colour->SetSelectedIndex( index == -1 ? 0 : index );
	m_PlayerSlots.at( slotIndex ).SelectedColourIndex = index;
	for ( auto& colourEntry : m_ColourEntries )
	{
		colourEntry.SlotID = colourEntry.SlotID == slotIndex ? -1 : colourEntry.SlotID;
	}
	if ( index != -1 )
	{
		m_ColourEntries.at( index ).SlotID = slotIndex;
		g_GameData.SetPlayerColour( slotIndex, m_ColourEntries[ index ].Colour );
	}
	else
	{
		g_GameData.SetPlayerColour( slotIndex, glm::vec4(1.0f) );
	}
}

void SSGameLobbySP::ResetColour( short slotIndex )
{
	assert( slotIndex >= 0 && slotIndex < m_MaximumPlayerSlots );
	m_PlayerSlots.at( slotIndex ).Colour->SetSelectedIndex( 0 );
	m_PlayerSlots.at( slotIndex ).SelectedColourIndex = -1;
	for ( auto& colours : m_ColourEntries )
	{
		if ( colours.SlotID == slotIndex )
			colours.SlotID = -1;
	}
}

int SSGameLobbySP::GetUnusedColourIndex( short slotIndex )
{
	int toReturn = -1;
	for ( unsigned int i = 0; i < m_ColourEntries.size(); ++i )
	{
		if ( m_ColourEntries.at( i ).SlotID == -1 && toReturn == -1 )
		{
			toReturn = static_cast<int>( i );
		}
		// Prioritize using same index if possible
		if ( slotIndex == m_ColourEntries.at( i ).SlotID )
		{
			toReturn = static_cast<int>( i );
			break;
		}
	}
	return toReturn;
}

void SSGameLobbySP::AddAI( )
{
	if ( m_CurrentNumberOfPlayers < m_MaximumPlayerSlots )
	{
		m_PlayerSlots.at( m_CurrentNumberOfPlayers ).Name->SetVisible	( true );
		m_PlayerSlots.at( m_CurrentNumberOfPlayers ).Colour->SetVisible	( true );
		m_PlayerSlots.at( m_CurrentNumberOfPlayers ).Team->SetVisible	( true );

		m_PlayerSlots.at( m_CurrentNumberOfPlayers ).Colour->SetEnabled	( true );
		m_PlayerSlots.at( m_CurrentNumberOfPlayers ).Team->SetEnabled	( true );

		TryChangeColour( 0, m_CurrentNumberOfPlayers );

		++m_CurrentNumberOfPlayers;
		m_CurrentNumberOfPlayers = std::min( m_MaximumPlayerSlots, std::max( m_CurrentNumberOfPlayers, m_MinimumPlayerSlots ) );
	}
}

void SSGameLobbySP::RemoveAI( )
{
	if ( m_CurrentNumberOfPlayers > m_MinimumPlayerSlots )
	{
		--m_CurrentNumberOfPlayers;
		m_CurrentNumberOfPlayers = std::min( m_MaximumPlayerSlots, std::max( m_CurrentNumberOfPlayers, m_MinimumPlayerSlots ) );

		m_PlayerSlots.at( m_CurrentNumberOfPlayers ).Name->SetVisible	( false );
		m_PlayerSlots.at( m_CurrentNumberOfPlayers ).Colour->SetVisible	( false );
		m_PlayerSlots.at( m_CurrentNumberOfPlayers ).Team->SetVisible	( false );

		m_PlayerSlots.at( m_CurrentNumberOfPlayers ).Colour->SetEnabled	( false );
		m_PlayerSlots.at( m_CurrentNumberOfPlayers ).Team->SetEnabled	( false );
	}
}

bool SSGameLobbySP::ArePlayersOnDifferentTeams( ) const
{
	int team = m_PlayerSlots.at( 0 ).Team->GetSelectedIndex( );
	for ( int i = 0; i < m_CurrentNumberOfPlayers; ++i )
	{
		if ( m_PlayerSlots.at( i ).Team->GetSelectedIndex( ) != team )
			return true;
	}
	return false;
}

void SSGameLobbySP::UpdateLevelElements( const rString& level )
{
	g_GUI.DeleteObject( m_NameWindowLevelElements );
	m_WindowLevelElements = g_GUI.AddWindow( m_NameWindowLevelElements,
		GUI::Rectangle( 0, 0,
		m_WindowLevel->GetSize( ).x, m_WindowLobby->GetSize( ).y ),
		m_NameWindowLevel );
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
			sprite->GetSpriteDefinitionRef( ).Colour *= m_FactorResearchStationDarkening;
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
			sprite->GetSpriteDefinitionRef( ).Colour = g_SSResourceManager.GetResourceSpawnTypeColour( drop.Type ) * m_FactorResourceDarkening;
		}
	}
}
