/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>
#include "../../utility/GlobalDefinitions.h"
#include "../../utility/GameModeSelector.h"
#include "../../CompileFlags.h"

#define g_SSGameLobbySP SSGameLobbySP::GetInstance()

class SSGameLobbySP : public Subsystem
{
public:
	static SSGameLobbySP& GetInstance();

	void	Startup() override;
	void	Shutdown() override;
	void	UpdateUserLayer( const float deltaTime ) override;

private:
	// No external instancing allowed
	SSGameLobbySP() : Subsystem( "GameLobbySP" ) {}
	SSGameLobbySP( const SSGameLobbySP& rhs );
	~SSGameLobbySP() {};
	SSGameLobbySP& operator=( const SSGameLobbySP& rhs );

	void CreateMainWindow( );
	void CreateGameSettings( );
	void CreatePlayerSlots( );
	void CreateLevelSelection( );
	void StartGame( );

	// Level selection
	void LoadLevels( );
	void LoadLevelPreviews( const rVector<rString> levelNames );
	void SetLevel( const rString& level );

	// Spawn point
	void CreateSpawnPointButtons( );
	void UpdateSpawnPointButtons( );
	void TryChangeSpawnPoint( int index );
	void SetSpawnPoint( int index, short playerID );
	void ResetSpawnPoint( short playerID );

	// Colour
	void TryChangeColour( int index, short playerID );
	void SetColour( int index, short playerID );
	void ResetColour( short playerID );
	int  GetUnusedColourIndex( short playerID );

	void AddAI( );
	void RemoveAI( );

	bool ArePlayersOnDifferentTeams( ) const;
	void UpdateLevelElements( const rString& level );

#ifdef ROBOGRADE_DEV
	const GameModeType m_GameModeToChangeTo = GameModeType::SingleplayerAI;
	const rString m_ScriptAIOnly			= "GE_SP_ToggleAIOnly";
	bool m_IsAIOnly							= false;
#else
	const GameModeType m_GameModeToChangeTo = GameModeType::SingleplayerAI;
#endif

	// Main window
	const rString 	m_NameWindowParent		= "RootWindow";
	const rString 	m_NameWindowLobby		= "SinglePlayerLobby";
	const rString 	m_TextureNameButtons	= "Button_64x32.png";
	const rString 	m_TextureNameComboBoxes = "Button_64x32.png";
	const rString 	m_ScriptStartGame		= "GE_SP_StartGame";
	FONT_ID 		m_Font					= FONT_ID_LEKTON_16;
	const int 		m_ButtonWidth			= 96;
	const int 		m_ButtonHeight			= 48;
	GUI::Window* 	m_WindowLobby			= nullptr;
	GUI::Button* 	m_ButtonStartGame		= nullptr;
	GUI::Button* 	m_ButtonBack			= nullptr;
	const int 		m_TopPadding			= 15;
	const int 		m_BottomPadding			= 15;
	const int 		m_LeftPadding			= 10;

	// Teams
	const int 		m_MaxNrOfTeams				= 4;

	// Game settings
	const rString 	m_WindowNameGamePlaySettings	= "SP_GamePlaySettingsWindow";
	const rString	m_ScriptChangeInitialSquadSize	= "GE_SP_ChangeInitialSquadSize";
	GUI::Button* 	m_BtnSpawnCountVal				= nullptr;
	GUI::Button*	m_BtnSpawnCountDec				= nullptr;
	GUI::Button*	m_BtnSpawnCountInc				= nullptr;
	GUI::Button* 	m_BtnSpawnCount 				= nullptr;
	const int		m_MinimumInitialSquadSize		= 1;
	const int		m_MaximumInitialSquadSize		= 24;
	int				m_InitialInitalSquadSize		= 8;
	int				m_InitialSquadSize				= m_InitialInitalSquadSize;

	// Colours
	struct ColourEntry
	{
		rString		Text;
		glm::vec4	Colour;
		glm::vec4 	TextColour;
		int 		SlotID;
	};
	const rString 		m_ScriptNameColourChange	= "GE_SP_ColourChange";
	rVector<ColourEntry>m_ColourEntries;

	struct PlayerSlot
	{
		GUI::Text* 		Name	= nullptr;
		GUI::ComboBox*	Colour	= nullptr;
		GUI::ComboBox* 	Team	= nullptr;

		int 			SelectedColourIndex = -1;
		bool 			IsAI;
	};

	// Player slots window
	const rString 		m_PathConfigName 			= "network.cfg";
	const rString 		m_NameWindowPlayerSlots		= "SP_PlayerSlots";
	GUI::Window*		m_WindowPlayerSlots			= nullptr;
	const int 			m_MaximumPlayerSlots		= MAX_PLAYERS; // TODOJM: Can be more than MAX_PLAYERS?
	const int 			m_MinimumPlayerSlots		= 1;
	const int 			m_HeightPlayerEntry			= 32;
	const int 			m_WidthName					= 100;
	const int 			m_WidthColour				= 100;
	const int 			m_WidthTeam					= 100;
	rVector<PlayerSlot>	m_PlayerSlots;
	int 				m_CurrentNumberOfPlayers	= m_MinimumPlayerSlots;
	GUI::Button*		m_ButtonAddAI				= nullptr;
	GUI::Button*		m_ButtonRemoveAI			= nullptr;
	const rString 		m_ScriptNameAddAI 			= "GE_SP_AddAI";
	const rString 		m_ScriptNameRemoveAI		= "GE_SP_RemoveAI";

	// Level selection
	const rString 	m_LevelPreviewFormat		= ".png";
	const rString 	m_MapsFolder				= "../../../asset/maps/";
	const rString 	m_DefaultLevelTexturePath	= "../../../asset/gui/sprites/NoPreviewAvaliable.png";
	const rString 	m_NameWindowSelectLevel		= "SP_SelectLevelWindow";
	const rString 	m_NameWindowLevel			= "SP_LevelWindow";
	const rString 	m_ScriptClickLevelChange	= "GE_SP_LevelChange";
	GUI::Window* 	m_WindowSelectLevel			= nullptr;
	GUI::Window* 	m_WindowLevel				= nullptr;
	GUI::Window* 	m_WindowGamePlaySettings	= nullptr;
	const int 		m_HeightComboBoxSelectLevel = 32;
	GUI::ComboBox* 	m_ComboBoxSelectLevel		= nullptr;
	GUI::Sprite* 	m_SpriteLevelSelected		= nullptr;
	size_t 			m_CurrentLevelNrOfSpawns	= 0;
	struct ResourceSpawn
	{
		glm::vec2 Position;
		int Type;
	};
	GUI::Window*	m_WindowLevelElements			= nullptr;
	const rString	m_NameWindowLevelElements		= "LevelElementsWindow";
	const rString	m_NameTextureResearchStations	= "minimap/controlPoint.png";
	const rString	m_NameTextureResourceDrops		= "minimap/Resource.png";
	const float		m_FactorResourceDarkening		= 0.8f;
	const float		m_FactorResearchStationDarkening= 0.8f;

	rMap<rString, rVector<ResourceSpawn>> m_ResourceSpawns;
	rMap<rString, rVector<glm::vec2>> m_ResearchStations;
	rMap<rString, gfx::Texture*> m_LevelTextures;
	
	// Spawn points
	struct SpawnPointButton
	{
		GUI::Button* Button;
		short PlayerID;
	};
	rVector<SpawnPointButton> 	m_ButtonsSpawnPoints;
	rMap<rString, rVector<glm::vec2>> m_SpawnPoints;
	const rString 	m_ScriptSpawnPointSelect		= "GE_SpawnPointButtonClick";
	const rString 	m_TextureNameSpawnPointButton	= "minimap/SpawnPoint.png";
	const int 		m_SizeSpawnPointButton			= 32;
	const int 		m_MaxNrOfSpawnPoints			= MAX_PLAYERS;
	int 			m_SizeLevelWindow				= 0;
	const glm::vec4 m_ColourNeutralSpawnPoint		= glm::vec4( 0.5f, 0.5f, 0.5f, 1.0f );
	short 			m_CurrentSpawnPointToSet 		= 0;
};
