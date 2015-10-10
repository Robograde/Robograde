/**************************************************
2015 Robograde Development Team
***************************************************/

#pragma once
#include <atomic>
#include <glm/glm.hpp>
#include <memory/Alloc.h>
#include <utility/FileUtility.h>
#include "GlobalDefinitions.h"	// Used for setting initial player count.
#include "../datadriven/ComponentTypes.h"
#include "../subsystem/replay/SSReplayPlayer.h"

#if PLATFORM == PLATFORM_LINUX
#include <limits.h>
#endif

#define g_GameData	GameData::GetInstance()

class GameData
{
public:
	struct Player;
	static GameData&			GetInstance();

	void						Reset();
	bool						ReadGameSettingsConfig();
	bool						ReadGraphicsConfig();
	void						AddSpawnPoint( glm::vec3 spawn );
	void						AddPlayer( short playerID, bool isAI );
	void						RemovePlayer( short playerID );
	void						ClearPlayers();
	void						IncrementFrameCount();
	void 						ResetSpawnPoints( );

	unsigned int				GetFrameCount()							const;
	int							GetWindowHeight()						const;
	int							GetWindowWidth()						const;
	int							GetFieldHeight()						const;
	int							GetFieldWidth()							const;
	const rVector<glm::vec3>&	GetSpawnPoints()						const;
	const rString&				GetDefaultGameModeName()				const;
	const rString&				GetCurrentMapName()						const;
	const glm::vec4& 			GetPlayerColour( short playerID )		const;
	int 						GetPlayerSpawnPoint( short playerID );	
	const rVector<Player>&		GetPlayers()							const;

	void						SetWindowSize( int width, int height );
	void						SetFieldHeight( const int val );
	void						SetFieldWidth( const int val );
	void						SetCurrentMapName( const rString& currentMap );
	void 						SetPlayerColour( short playerID, const glm::vec4& colour );
	void 						SetPlayerSpawnPoint( int spawnPointIndex, short playerID );
	
	bool						IsDedicatedServer() const;
	

	// TODO FIX THIS UGLY PIECE OF CODE
	bool 					m_Vsync = false;
	bool 					m_Fullscreen = false;
	int 					m_MSAA = 1;

	struct Player
	{
		short PlayerID;
		bool IsAI;
	};
private:
							// No external instancing allowed
							GameData()			{ }
							GameData			( const GameData& rhs );
							~GameData()			{ }
	GameData&				operator=			( const GameData& rhs );

	std::atomic_uint		m_FrameCount;
	
	int						m_FieldWidth;
	int						m_FieldHeight;
	rVector<glm::vec3>		m_SpawnPoints;

	int 					m_WindowWidth;
	int 					m_WindowHeight;
	rString					m_DefaultGameModeName;
	rString					m_AIMapName;
	rString					m_CurrentMap;
	rVector<glm::vec4> 		m_PlayerColours;
	const glm::vec4 		m_DefaultColour = glm::vec4( 1.0f );
	rVector<short> 			m_PlayerSpawnPoints;
	rVector<Player>			m_Players;
};
