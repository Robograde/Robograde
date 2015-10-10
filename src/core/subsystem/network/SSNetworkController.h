/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <network/NetworkInfo.h>
#include "../../utility/PlayerData.h"

// Conveniency access function
#define g_SSNetworkController SSNetworkController::GetInstance()
#define PLAYER_TYPE_UNDERLYING_TYPE short

enum class PlayerType : PLAYER_TYPE_UNDERLYING_TYPE
{
	Player,
	Observer,
	Editor,
};

struct NetworkPlayer
{
	NetworkPlayer( short playerID = PLAYER_ID_INVALID, short networkID = -1, PlayerType playerType = PlayerType::Observer, const rString& name = "ANON", unsigned int frameCounter = 0 )
	{
		PlayerID		= playerID;
		NetworkID		= networkID;
		Type			= PlayerType::Observer;
		Name			= name;
		FrameCounter	= frameCounter;
	}

	short			PlayerID				= PLAYER_ID_INVALID;
	short			NetworkID				= -1;
	PlayerType		Type					= PlayerType::Observer;
	rString			Name					= "ANON";
	unsigned int	FrameCounter			= 0;
	bool			HasFinishedLoading		= false;
};

class SSNetworkController : public Subsystem
{
public:
	static SSNetworkController& GetInstance( );

	void						Startup() override;
	void						Shutdown() override;
	void						UpdateUserLayer( const float deltaTime ) override;
	void						UpdateSimLayer( const float timeStep ) override;
	void						Reset();

	bool						MakeHost( unsigned short port = INVALID_PORT ); // Invalid port results in default port being used
	bool						ConnectToGame( const rString& address, unsigned short port );
	void						AddPlayer( const NetworkPlayer& newPlayer );
	void						RemovePlayer( short playerID );
	bool						ShareHostFile( const rString& filePath );

	rMap<short, NetworkPlayer>& GetNetworkedPlayers();
	const rString&				GetPlayerName( short playerID ) const;
	unsigned short				GetDefaultListeningPort() const;
	unsigned int				GetHostStep() const;

	void						SetPlayerName( short playerID, const rString& newPlayerName );
	void						SetPlayerFinishedLoading( short playerID );
	void						SetHostStep( unsigned int hostStep );

	bool						DoesPlayerExist( short playerID ) const;
	bool						AllFinishedLoading() const;

	void						IncrementFramesToRun();
	bool						DecrementFramesToRun();
	void						PushHostHash( unsigned int hash );
	unsigned int				PopHostHash();
	void						PushHostRandomCount( unsigned int randomCount );
	unsigned int				PopHostRandomCount();

private:
	// No external instancing allowed
	SSNetworkController( ) : Subsystem( "NetworkController" ) { }
	SSNetworkController( const SSNetworkController& rhs );
	~SSNetworkController( ) { };
	SSNetworkController& operator=( const SSNetworkController& rhs );

	void						CheckCallbacks();

	bool						ReadConfig();
	bool						EnsureEngineReady();

	void						KickPlayerByName( const rString& playerName );
	void						KickPlayerByID( short playerID );

	void						EnqueueIncomingConnection( const void* const networkID );	// Holds NetworkIDs to newly connected players
	void						EnqueueOutgoingConnection( const void* const networkID );	// Holds NetworkID to newly connected host
	void						EnqueueDisconnection( const void* const networkID );		// Holds NetworkIDs to recently disconnected players

	int							FindUnusedPlayerID() const;

	void						RegisterMessages();

	rMap<short, NetworkPlayer>	m_NetworkPlayers;
	LocklessQueue<short>		m_IncomingConnectionQueue;
	LocklessQueue<short>		m_OutgoingConnectionQueue;
	LocklessQueue<short>		m_DisconnectionQueue;
	rVector<int>				m_NetworkCallbackHandles;
	unsigned short				m_DefaultListeningPort	= INVALID_PORT;
	bool						m_HasReadConfig			= false;

	const rString				INVALID_PLAYER_NAME		= "INVALID_PLAYER_NAME";
	const rString				BESTWEIGHTS_FILE_PATH	= "../../../asset/ai/bestWeights.txt";
	const rString				GENES0_FILE_PATH		= "../../../asset/ai/genes0.txt";
	const rString				GENES1_FILE_PATH		= "../../../asset/ai/genes1.txt";
	const rString				GENES2_FILE_PATH		= "../../../asset/ai/genes2.txt";
	const rString				GENES3_FILE_PATH		= "../../../asset/ai/genes3.txt";

	unsigned int				m_HostStep				= 0;
	unsigned int				m_FramesToRun			= 0;
	std::queue<unsigned int>	m_HostHashQueue;
	std::queue<unsigned int>	m_HostRandomCountQueue;
};