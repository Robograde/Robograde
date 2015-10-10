/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include <initializer_list>
#include <memory/Alloc.h>
#include <script/ScriptEngine.h>
#include "../subsystem/Subsystem.h"

#define g_GameModeSelector GameModeSelector::GetInstance()

enum class GameModeType
{
	Unknown,
	ConsoleOnly,
	Singleplayer,
	SingleplayerAI,
	AIOnly,
	NetworkLobby,
	SinglePlayerLobby,
	Multiplayer,
	Replay,
	DedicatedServer,
	MainMenu,
};

struct GameMode
{
	GameMode( GameModeType type, const rString& name, bool isSimulation, bool isNetworked, bool isMenu )
		: Type( type ), Name( name ), IsSimulation( isSimulation ), IsNetworked( isNetworked ), IsMenu( isMenu ) {}

	GameModeType			Type;
	rString					Name;
	rVector<Subsystem*>		Subsystems; 
	bool					IsSimulation;
	bool					IsNetworked;
	bool					IsMenu;

	bool operator==( const GameMode& rhs ) const { return this->Type == rhs.Type; }
	bool operator!=( const GameMode& rhs ) const { return this->Type != rhs.Type; }
};

class GameModeSelector // TODODB: Document the shit out of this class // TODODB: Fix function order
{
public:
	static GameModeSelector&	GetInstance();
	void						Initialize();

	void						Update();
	void						AddSubsystemToGameModes( Subsystem* subsystem, std::initializer_list<GameModeType> gameModes = {} );

	void						SwitchToGameMode( GameModeType gameMode );
	bool						SwitchToGameModebyString( const rString& gameModeName );

	const GameMode&				GetCurrentGameMode() const;

	const GameMode&				GetGameModeFromType( GameModeType gameModeType ) const;
	const GameMode&				GetGameModeFromString( const rString& name ) const;
	GameModeType				GetGameModeTypeFromString( const rString& gameModeName ) const;	

private:
	// No external instancing allowed
	GameModeSelector() {}
	GameModeSelector( const GameModeSelector& rhs );
	~GameModeSelector() {}
	GameModeSelector& operator=( const GameModeSelector& rhs );

	void						StopCurrentMode();
	void						StartMode( const GameMode& gameMode );
	int							ScriptRestart( IScriptEngine* scriptEngine );
	int							ScriptPrintGameModeSubsystems( IScriptEngine* scriptEngine );
	int							ScriptSwitchGameMode( IScriptEngine* scriptEngine );

	GameMode*					GetEditableGameModeFromType( GameModeType gameModeType );

	GameMode					m_CurrentGameMode		= INVALID_GAME_MODE;
	GameMode					m_NextGameMode			= INVALID_GAME_MODE;	// TODODB: Set this to console if config fails to read
	rVector<GameMode>			m_GameModes;
	rMap<rString, GameModeType> m_GameModeNames;

	const GameMode INVALID_GAME_MODE = GameMode( GameModeType::Unknown, "unknown", false, false, false );
};