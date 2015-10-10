/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include <glm/glm.hpp>
#include <memory/Alloc.h>
#include <utility/SerializationUtility.h>
#include "MessageLibraryDefine.h"
#include "Message.h"

namespace MessageTypes
{
	enum MessageType : MESSAGE_TYPE_ENUM_UNDELYING_TYPE 
	{
		// 0 Is allocated by network engine
		BAD							= 1 << 1,
		HEST						= 1 << 2,
		ORDER_UNITS					= 1 << 3,
		ORDER_INVOKE				= 1 << 4,
		GAME_OVER					= 1 << 5,
		AI_MSG						= 1 << 6,
		RESEARCH					= 1 << 7,
		CONTROL_POINT				= 1 << 8,
		UPGRADE						= 1 << 9,
		RANDOM_SEED					= 1 << 10,
		USER_SIGNAL					= 1 << 11,
		STEP						= 1 << 12,
		STEP_RESPONSE				= 1 << 13,
		NAME_UPDATE					= 1 << 14,
		WRITE_FILE					= 1 << 15,
		CHAT						= 1 << 16,
		CONNECTION_STATUS			= 1 << 17,
		LEVEL_SELECTION_CHANGE		= 1 << 18,
		COLOUR_CHANGE				= 1 << 19,
		TEAM_CHANGE					= 1 << 20,
		SPAWN_POINT_CHANGE			= 1 << 21,
		CHANGE_SPAWN_COUNT			= 1 << 22,
		RESERVE_AI_PLAYER			= 1 << 23,
		UPGRADE_COMPLETE			= 1 << 24,
		SFX_PING_MESSAGE			= 1 << 25,
		USER_PING_MESSAGE			= 1 << 26,
		SFX_BUTTON_CLICK			= 1 << 27,
	};
}

namespace UserSignalType // TODODB: Rename this when it doesn't conflict with packets anymore
{
	enum SINGAL_TYPE
	{
		READY_TOGGLE,
		START_GAME,
		PAUSE,
		FINISHED_LOADING,
	};
}

namespace ConnectionStatusUpdates
{
	enum ConnectionStatusUpdate : char
	{
		CONNECTED,
		DISCONNECTED,
	};
}

struct HestMessage : public Message // TOOODB: Make these structs if possible
{
public:
	MESSAGING_API	HestMessage();
	MESSAGING_API	HestMessage( float testFloat, int testInt );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	float			HoofDensity;
	int				TailsOnFaceCount;
};

struct OrderUnitsMessage : public Message
{
public:
	MESSAGING_API	OrderUnitsMessage();
	MESSAGING_API	OrderUnitsMessage( unsigned int executionFrame, int missionType, int teamID, const rVector<int>& squads, const glm::vec3& targetLocation, unsigned int targetEntity, bool overridingCommand );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	int				MissionType;
	int				TeamID;
	glm::vec3		TargetLocation;
	unsigned int	TargetEntity;
	bool			OverridingCommand;
	rVector<int>	Squads;
};

struct OrderInvokeMessage : public Message
{
public:
	MESSAGING_API	OrderInvokeMessage();
	MESSAGING_API	OrderInvokeMessage( unsigned int executionFrame, const rVector<int>& squads, int command, const int team, int genericValue );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	int				GenericValue;
	int				TeamID;
	int				EnumCommand;
	rVector<int>	Squads;
};

struct GameOverMessage : public Message
{
public:
	MESSAGING_API	GameOverMessage();
	MESSAGING_API	GameOverMessage( int gameOverState );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	int	State;
};

struct AIMessage : public Message
{
public:
	MESSAGING_API	AIMessage();
	MESSAGING_API	AIMessage( short aiMsg, int team, int squadID, unsigned int entityID );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	short	AIMsg;
	int		TeamID;
	int		SquadID;
	int		EntityID;
};

struct ResearchMessage : public Message
{
public:
	MESSAGING_API	ResearchMessage();
	MESSAGING_API	ResearchMessage( short playerID, int upgradeID, int statusChange );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	short	PlayerID;
	int		UpgradeID;
	int		StatusChange;
};

struct ControlPointMessage : public Message
{
public:
	MESSAGING_API	ControlPointMessage();
	MESSAGING_API	ControlPointMessage( unsigned int entityID, short oldOwnerID, short newOwnerID, int statusChange );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	unsigned int	EntityID;
	short			OldOwnerID;
	short			NewOwnerID;
	int				StatusChange;
};

struct UpgradeMessage : public Message
{
public:
	MESSAGING_API	UpgradeMessage();
	MESSAGING_API	UpgradeMessage( unsigned int executionFrame, short playerID, const rVector<int>& squads, int upgradeType );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	short			PlayerID;
	int				UpgradeType;
	rVector<int>	Squads;
};

struct RandomSeedMessage : public Message
{
public:
	MESSAGING_API	RandomSeedMessage();
	MESSAGING_API	RandomSeedMessage( unsigned int seed );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	unsigned int	Seed;
};

struct UserSignalMessage : public Message
{
public:
	MESSAGING_API	UserSignalMessage();
	MESSAGING_API	UserSignalMessage( UserSignalType::SINGAL_TYPE type, unsigned short senderID );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	UserSignalType::SINGAL_TYPE SignalType;
	unsigned short SenderID;
};

struct StepMessage : public Message
{
public:
	MESSAGING_API	StepMessage();
	MESSAGING_API	StepMessage( unsigned int frame, unsigned int hash, unsigned int randomCount );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	unsigned int Frame;
	unsigned int Hash;
	unsigned int RandomCount;
};

struct StepResponseMessage : public Message
{
public:
	MESSAGING_API	StepResponseMessage();
	MESSAGING_API	StepResponseMessage( short playerID, unsigned int frame );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	short			PlayerID;
	unsigned int	Frame;
};

struct NameUpdateMessage : public Message
{
public:
	MESSAGING_API		NameUpdateMessage();
	MESSAGING_API		NameUpdateMessage( const rVector<rString>& names, const rVector<short>& playerIDs );

	MESSAGING_API		Message*		Clone() const override;
	MESSAGING_API		unsigned int	GetSerializationSize() const override;
	MESSAGING_API		void			Serialize( char*& buffer ) const override;
	MESSAGING_API		void			Deserialize( const char*& buffer ) override;

	rVector<short>		PlayerIDs;
	rVector<rString>	Names;
};

struct WriteFileMessage : public Message
{
public:
	MESSAGING_API	WriteFileMessage();
	MESSAGING_API	WriteFileMessage( const rString& text, const rString& filePath );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	rString FilePathStr;
	rString FileTextStr;
};

struct ChatMessage : public Message
{
public:
	MESSAGING_API	ChatMessage();
	MESSAGING_API	ChatMessage( const rString& message, short senderID, short targetID = -1 );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	rString	MessageText;
	short	SenderID; // PlayerID
	short	TargetID; // PlayerID
};

struct ConnectionStatusUpdateMessage : public Message
{
public:
	MESSAGING_API ConnectionStatusUpdateMessage();
	MESSAGING_API ConnectionStatusUpdateMessage( short networkID, ConnectionStatusUpdates::ConnectionStatusUpdate newStatus );

	MESSAGING_API Message* Clone() const override;
	MESSAGING_API unsigned int GetSerializationSize() const override;
	MESSAGING_API void Serialize( char*& buffer ) const override;
	MESSAGING_API void Deserialize( const char*& buffer ) override;

	short NetworkID; //TODODB: Rename this to playerID
	ConnectionStatusUpdates::ConnectionStatusUpdate	ConnectionStatus;
};

struct LevelSelectionChangeMessage : public Message
{
public:
	MESSAGING_API 	LevelSelectionChangeMessage( );
	MESSAGING_API 	LevelSelectionChangeMessage( const rString& levelName );
	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	rString LevelName;
};

struct ColourChangeMessage : public Message
{
public:
	MESSAGING_API 	ColourChangeMessage( );
	MESSAGING_API 	ColourChangeMessage( short playerID, int selectedColourIndex, bool force );
	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	short PlayerID;
	int SelectedColourIndex;
	bool Force;
};

struct TeamChangeMessage : public Message
{
public:
	MESSAGING_API 	TeamChangeMessage( );
	MESSAGING_API 	TeamChangeMessage( short playerID, int selectedTeamIndex, bool force );
	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	short PlayerID;
	int SelectedTeamIndex;
	bool Force;
};

struct SpawnPointChangeMessage : public Message
{
public:
	MESSAGING_API 	SpawnPointChangeMessage( );
	MESSAGING_API 	SpawnPointChangeMessage( short playerID, int selectedTeamIndex, bool force );
	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	short PlayerID;
	int SelectedSpawnPoint;
	bool Force;
};

struct SpawnCountChangeMessage : public Message
{
public:
	MESSAGING_API 	SpawnCountChangeMessage();
	MESSAGING_API 	SpawnCountChangeMessage( short count );
	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize(char*& buffer) const override;
	MESSAGING_API	void			Deserialize(const char*& buffer) override;

	short Count;
};

struct ReserveAIMessage : public Message
{
public:
	MESSAGING_API 	ReserveAIMessage();
	MESSAGING_API 	ReserveAIMessage( short playerID, bool on );
	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize(char*& buffer) const override;
	MESSAGING_API	void			Deserialize(const char*& buffer) override;

	short 	PlayerID;
	bool 	On; // True when reserving, false when unreserving
};

struct UpgradeCompleteMessage : public Message
{
public:
	MESSAGING_API	UpgradeCompleteMessage();
	MESSAGING_API	UpgradeCompleteMessage(short playerID, int upgradeType);

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	short			PlayerID;
	int				UpgradeType;
};

struct SFXPingMessage : public Message
{
public:
	MESSAGING_API	SFXPingMessage();
	MESSAGING_API	SFXPingMessage(short playerID, short pingType);

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	short			PlayerID;
	short			PingType;
};

struct UserPingMessage : public Message
{
public:
	MESSAGING_API	UserPingMessage( );
	MESSAGING_API	UserPingMessage( short playerID, glm::vec3 position );

	MESSAGING_API	Message*		Clone( ) const override;
	MESSAGING_API	unsigned int	GetSerializationSize( ) const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	// TODODP: Come up with variables
	short			PingingPlayerID;
	glm::vec3		WorldPosition;
};

struct SFXButtonClickMessage : public Message
{
public:
	MESSAGING_API	SFXButtonClickMessage();
	MESSAGING_API	SFXButtonClickMessage( const rString& buttonName );

	MESSAGING_API	Message*		Clone() const override;
	MESSAGING_API	unsigned int	GetSerializationSize() const override;
	MESSAGING_API	void			Serialize( char*& buffer ) const override;
	MESSAGING_API	void			Deserialize( const char*& buffer ) override;

	rString ButtonName;
};

namespace Messages
{
	static Message* GetDefaultMessage( const MessageTypes::MessageType type )
	{
		switch ( type )
		{
			case MessageTypes::BAD:
				return nullptr;
			case MessageTypes::HEST:
				return tNew( HestMessage );
			case MessageTypes::ORDER_UNITS:
				return tNew( OrderUnitsMessage );
			case MessageTypes::ORDER_INVOKE:
				return tNew( OrderInvokeMessage );
			case MessageTypes::GAME_OVER:
				return tNew( GameOverMessage );
			case MessageTypes::AI_MSG:
				return tNew( AIMessage );
			case MessageTypes::RESEARCH:
				return tNew( ResearchMessage );
			case MessageTypes::CONTROL_POINT:
				return tNew( ControlPointMessage );
			case MessageTypes::UPGRADE:
				return tNew( UpgradeMessage );
			case MessageTypes::RANDOM_SEED:
				return tNew( RandomSeedMessage );
			case MessageTypes::USER_SIGNAL:
				return tNew( UserSignalMessage );
			case MessageTypes::STEP:
				return tNew( StepMessage );
			case MessageTypes::STEP_RESPONSE:
				return tNew( StepResponseMessage );
			case MessageTypes::NAME_UPDATE:
				return tNew( NameUpdateMessage );
			case MessageTypes::WRITE_FILE:
				return tNew( WriteFileMessage );
			case MessageTypes::CHAT:
				return tNew( ChatMessage );
			case MessageTypes::CONNECTION_STATUS:
				return tNew( ConnectionStatusUpdateMessage );
			case MessageTypes::LEVEL_SELECTION_CHANGE:
				return tNew( LevelSelectionChangeMessage );
			case MessageTypes::COLOUR_CHANGE:
				return tNew( ColourChangeMessage );
			case MessageTypes::TEAM_CHANGE:
				return tNew( TeamChangeMessage );
			case MessageTypes::SPAWN_POINT_CHANGE:
				return tNew( SpawnPointChangeMessage );
			case MessageTypes::CHANGE_SPAWN_COUNT:
				return tNew( SpawnCountChangeMessage );
			case MessageTypes::RESERVE_AI_PLAYER:
				return tNew( ReserveAIMessage );
			case MessageTypes::UPGRADE_COMPLETE:
				return tNew( UpgradeCompleteMessage );
			case MessageTypes::SFX_PING_MESSAGE:
				return tNew( SFXPingMessage );
			case MessageTypes::USER_PING_MESSAGE:
				return tNew( UserPingMessage );
			case MessageTypes::SFX_BUTTON_CLICK:
				return tNew( SFXButtonClickMessage );
			default:
				return nullptr;
				break;
		}
	}
}