/**************************************************
2015 Johan Melin
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <atomic>
#include <gui/GUIEngine.h>
#include "../../utility/GlobalDefinitions.h"
#include "../../utility/PlayerData.h"
#include <messaging/Subscriber.h>

#define g_SSGameLobby SSGameLobby::GetInstance()

class SSGameLobby : public Subsystem, public Subscriber
{
public:
	static SSGameLobby& GetInstance();

	void			Startup() override;
	void			Shutdown() override;
	void			UpdateUserLayer( const float deltaTime ) override;
	void			Reset();
	
	void			StartGame();
	void 			SwitchToMultiplayer( );
	void			ToggleReadyStatus( short playerID = g_PlayerData.GetPlayerID() );

	bool			IsPlayerReady( short playerID );
	void 			SetLevel( const rString& levelName );
	void			SetEditorMode( bool editorMode );

	const rString& 	GetSelectedLevel( ) const;
	void 			OnNewClientConnect( short playerID );

	bool			IsEditorLobby() const;

private:
	enum class ConnectionState
	{
		EnteringIP,
		WaitingToConnect,
		Connected,
		Hosting
	};
	// No external instancing allowed
	SSGameLobby() : Subsystem( "GameLobby" ), Subscriber( "GameLobby" ) {}
	SSGameLobby( const SSGameLobby& rhs );
	~SSGameLobby() {};
	SSGameLobby& operator=( const SSGameLobby& rhs );

	void ConnectToGame( const rString& inputString );
	void HostGame( );
	void OnReadyButtonClicked();
	void OnConnectionSuccess(const void* const connectionData );
	void OnConnectionFail( const void* const connectionData );
	void OnDisconnection( const void* const disconnectingPlayerID );
	void BackFromEnterIP( );
	void BackFromLobby( ); 
	void LoadLevels( );
	void LoadLevelPreviews( const rVector<rString> levelNames );
	void TryChangeColour( int index, short playerID, bool force );
	void SetColour( int index, short playerID );
	void ResetColour( short playerID );
	int  GetUnusedColourIndex( short playerID );
	void TryChangeTeam( int index, short playerID, bool force );
	void SetTeam( int index, short playerID );
	void ResetTeam( short playerID );
	void CreateSpawnPointButtons( );
	void UpdateSpawnPointButtons( );
	void TryChangeSpawnPoint( int index, short playerID, bool force );
	void SetSpawnPoint( int index, short playerID );
	void ResetSpawnPoint( short playerID );
	void ChangeSpawnCount( int count );
	void ChangeReserveAI( short playerID, bool on );
	short  GetNextSpawnToSetAsHost( );
	bool ArePlayersOnDifferentTeams( ) const;
	bool AreAnyClientsConnected( ) const;
	void UpdateLevelElements( const rString& level );

	bool				m_PlayersReady[MAX_PLAYERS];

	int					m_NetworkCallbackHandle			= -1;

	const int 			m_MaxNrOfTeams 					= 4;

	const rString 		m_WindowNameEnterIP				= "EnterIP";
	const rString 		m_ScriptNameConnect				= "GE_Connect";
	const rString 		m_ScriptNameHost				= "GE_Host";
	const rString 		m_ScriptNameBackEnterIP 	 	= "GE_BackEnterIP";
	const int 			m_WidthEnterIP					= 700;
	const int 			m_HeightEnterIP					= 200;
	const int 			m_WidthButton					= 96;
	const int 			m_HeightButton					= 48;
	const int 			m_WidthTextBox					= 412;
	const int 			m_HeightTextBox 				= 48;
	const int 			m_HeightInfoText 				= 30;
	const int 			m_HeightErrorText 				= 30;
	const int 			m_TopPadding 					= 15;
	const int 			m_BottomPadding					= 15;
	const int 			m_LeftPadding 					= 10;
	GUI::Window* 		m_WindowEnterIP 				= nullptr;
	GUI::TextBox* 		m_TextBoxEnterIP 				= nullptr;
	GUI::Text* 			m_TextError 					= nullptr;
	GUI::Button* 		m_ButtonConnect 				= nullptr;
	GUI::Button* 		m_ButtonHost					= nullptr;
	GUI::Button* 		m_ButtonBackEnterIP				= nullptr;
	GUI::Button* 		m_BtnSpawnCountVal				= nullptr;
	GUI::Button*		m_BtnSpawnCount					= nullptr;
	GUI::Button*		m_BtnSpawnCountDec				= nullptr;
	GUI::Button*		m_BtnSpawnCountInc				= nullptr;

	const rString 		m_WindowNameParent				= "RootWindow";
	const rString 		m_WindowNameLobby				= "GameLobby";
	const rString 		m_ScriptNameStartGame			= "GE_StartGame";
	const rString 		m_ScriptNameReady				= "GE_Ready";
	const rString 		m_ScriptNameBackLobby			= "GE_BackLobby";
	const rString		m_ScriptChangeGameSetting		= "GE_DecreaseGameSetting";
	const rString		m_ScriptIncGameSetting			= "GE_DecreaseGameSetting";

	GUI::Window* 		m_WindowLobby 					= nullptr;
	GUI::Button* 		m_ButtonStartGame 				= nullptr;
	GUI::Button* 		m_ButtonReady 					= nullptr;
	GUI::Button* 		m_ButtonBackLobby 				= nullptr;
	
	struct ConnectedPlayer
	{
		GUI::Text* 		Name 	= nullptr;
		GUI::ComboBox*	Colour 	= nullptr;
		GUI::Sprite*	Ready 	= nullptr;
		GUI::ComboBox* 	Team 	= nullptr;
		GUI::Button* 	Kick 	= nullptr;
		GUI::Button* 	ReserveForAIBtn = nullptr;
		int 			SelectedColourIndex = -1;
		int 			SelectedTeamIndex = -1;
		bool 			IsAI = false;
		bool 			IsActive = false;
		short 			PlayerID = -1;
	};

	struct ColourEntry
	{
		rString Text;
		glm::vec4 Colour;
		glm::vec4 TextColour;
		short PlayerID; // The player that is using this colour at the moment.
	};

	const rString 		m_WindowNameConnectedClients 	= "ConnectedClientsWindow";
	const rString 		m_ScriptNameColourChange 		= "GE_ColourChange";
	const rString 		m_ScriptNameTeamChange  		= "GE_TeamChange";
	const rString 		m_ScriptNameReserveForAI  		= "GE_ReserveAI";
	const rString 		m_ScriptNameKick		  		= "GE_Kick";
	const int 			m_HeightPlayerEntry 			= 32;
	const int 			m_WidthName						= 100;
	const int 			m_WidthColour					= 128;
	const int 			m_WidthTeam						= 128;
	const int 			m_WidthReady					= 32;
	const int 			m_WidthKick						= 32;
	const int 			m_WidthReserveForAI				= 32;
	const rString 		m_TextureNameButtons 			= "Button_64x32.png";
	const rString 		m_TextureNameReadySprite		= "Button_Ready.png";
	const rString 		m_TextureNameKickSprite			= "Button_Kick.png";
	const rString 		m_TextureNameComboBox			= "Button_64x32.png";
	const rString 		m_TextureNameComboBoxItem		= "Button_64x32.png";
	ConnectedPlayer 	m_ConnectedPlayerEntries[MAX_PLAYERS];
	rVector<ColourEntry>m_ColourEntries;
	GUI::Window*		m_WindowConnectedClients		= nullptr;

	const rString 		m_WindowNameSelectLevel 		= "SelectLevelWindow";
	const rString 		m_WindowNameLevel 				= "LevelWindow";
	const rString 		m_WindowNameGamePlaySettings	= "GamePlaySettingsWindow";
	const rString 		m_ScriptClickLevelChange 		= "GE_LevelChange";
	GUI::Window* 		m_WindowSelectLevel 			= nullptr;
	GUI::Window* 		m_WindowLevel 					= nullptr;
	GUI::Window* 		m_WindowGamePlaySettings		= nullptr;
	const int 			m_HeightComboBoxSelectLevel		= 32;

	const rString 		m_TextureNameSelectLevel		= "Button_128x32.png";
	GUI::ComboBox* 		m_ComboBoxSelectLevel 			= nullptr;
	GUI::Sprite* 		m_SpriteLevelSelected			= nullptr;
	const rString 		m_MapsFolder 					= "../../../asset/maps/";
	const rString 		m_LevelPreviewFormat 			= ".png";
	const rString 		m_DefaultLevelTexturePath		= "../../../asset/gui/sprites/NoPreviewAvaliable.png";
	size_t 				m_CurrentLevelNrOfSpawns 		= 0;

	const rString 		m_ScriptSpawnPointSelect 		= "GE_SpawnPointButtonClick";
	const rString 		m_TextureNameSpawnPointButton	= "minimap/SpawnPoint.png";
	const int 			m_SizeSpawnPointButton			= 32;
	const int 			m_MaxNrOfSpawnPoints			= MAX_PLAYERS;
	int 				m_SizeLevelWindow				= 0;

	int					m_NumberOfSpawns				= 8;

	bool				m_IsEditorLobby					= false;

	struct SpawnPointButton
	{
		GUI::Button* Button;
		short PlayerID;
	};
	rVector<SpawnPointButton> 	m_ButtonsSpawnPoints;
	const glm::vec4 			m_ColourNeutralSpawnPoint = glm::vec4( 0.5f, 0.5f, 0.5f, 1.0f );
	short 						m_SpawnPointToSetAsHost = 0;

	rMap<rString, rVector<glm::vec2>> m_SpawnPoints;

	struct ResourceSpawn
	{
		glm::vec2 Position;
		int Type;
	};
	GUI::Window* m_WindowLevelElements;
	const rString m_NameWindowLevelElements = "LevelElementsWindow";
	const rString m_NameTextureResearchStations = "minimap/controlPoint.png";
	const rString m_NameTextureResourceDrops = "minimap/Resource.png";
	rMap<rString, rVector<ResourceSpawn>> m_ResourceSpawns;
	rMap<rString, rVector<glm::vec2>> m_ResearchStations;
	rMap<rString, gfx::Texture*> m_LevelTextures;
	const float m_FactorResourceDarkening = 0.8f;
	const float m_FactorResearchStationDarkening = 0.8f;

	ConnectionState		m_ConnectionState				= ConnectionState::EnteringIP;
	std::atomic_bool	m_ConnectionAttemptFailed;
};