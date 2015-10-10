/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory/Alloc.h>

#include <utility/SerializationUtility.h>
#include <Messaging/Message.h>

namespace MessageTypes
{
	enum MessageType : MESSAGE_TYPE_ENUM_UNDELYING_TYPE 
	{
		// 0 Is allocated by network engine
		BAD									= 1ULL << 1,
		HEST								= 1ULL << 2,
		ORDER_UNITS							= 1ULL << 3,
		ORDER_INVOKE						= 1ULL << 4,
		GAME_OVER							= 1ULL << 5,
		AI_MSG								= 1ULL << 6,
		RESEARCH							= 1ULL << 7,
		CONTROL_POINT						= 1ULL << 8,
		UPGRADE								= 1ULL << 9,
		RANDOM_SEED							= 1ULL << 10,
		USER_SIGNAL							= 1ULL << 11,
		STEP								= 1ULL << 12,
		STEP_RESPONSE						= 1ULL << 13,
		NAME_UPDATE							= 1ULL << 14,
		WRITE_FILE							= 1ULL << 15,
		CHAT								= 1ULL << 16,
		CONNECTION_STATUS					= 1ULL << 17,
		LEVEL_SELECTION_CHANGE				= 1ULL << 18,
		COLOUR_CHANGE						= 1ULL << 19,
		TEAM_CHANGE							= 1ULL << 20,
		SPAWN_POINT_CHANGE					= 1ULL << 21,
		CHANGE_SPAWN_COUNT					= 1ULL << 22,
		RESERVE_AI_PLAYER					= 1ULL << 23,
		UPGRADE_COMPLETE					= 1ULL << 24,
		SFX_PING_MESSAGE					= 1ULL << 25,
		USER_PING_MESSAGE					= 1ULL << 26,
		SFX_BUTTON_CLICK					= 1ULL << 27,
		PLAYER_TYPE							= 1ULL << 28,
		PLACE_PROP							= 1ULL << 29,
		PLACE_RESOUCE						= 1ULL << 30,
		PLACE_CONTROL_POINT					= 1ULL << 31,
		MOVE_OBJECT							= 1ULL << 32,
		EDITOR_SFXEMITTER					= 1ULL << 33,
		UPDATE_GHOST_ENTITY_POS				= 1ULL << 34,
		UPDATE_GHOST_ENTITY_VISIBLILITY		= 1ULL << 35,
		UPDATE_GHOST_ENTITY_MODEL			= 1ULL << 36,
		EDITOR_CAMERA_PATHS					= 1ULL << 37,
		SELECT_ENTITY						= 1ULL << 38,
		EDITOR_PARTICLE_EMITTER				= 1ULL << 39,
		EDITOR_TERRAIN_BRUSH				= 1ULL << 40,
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
		SET_EDITOR_LOBBY,
		SET_MULTIPLAYER_LOBBY,
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

struct HestMessage : public Message
{
public:
	HestMessage();
	HestMessage( float testFloat, int testInt );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	float			HoofDensity;
	int				TailsOnFaceCount;
};

struct OrderUnitsMessage : public Message
{
public:
	OrderUnitsMessage();
	OrderUnitsMessage( unsigned int executionFrame, int missionType, int teamID, const rVector<int>& squads, const glm::vec3& targetLocation, unsigned int targetEntity, bool overridingCommand );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

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
	OrderInvokeMessage();
	OrderInvokeMessage( unsigned int executionFrame, const rVector<int>& squads, int command, const int team, int genericValue );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	int				GenericValue;
	int				TeamID;
	int				EnumCommand;
	rVector<int>	Squads;
};

struct GameOverMessage : public Message
{
public:
	GameOverMessage();
	GameOverMessage( int gameOverState );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	int	State;
};

struct AIMessage : public Message
{
public:
	AIMessage();
	AIMessage( short aiMsg, int team, int squadID, unsigned int entityID );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short	AIMsg;
	int		TeamID;
	int		SquadID;
	int		EntityID;
};

struct ResearchMessage : public Message
{
public:
	ResearchMessage();
	ResearchMessage( short playerID, int upgradeID, int statusChange );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short	PlayerID;
	int		UpgradeID;
	int		StatusChange;
};

struct ControlPointMessage : public Message
{
public:
	ControlPointMessage();
	ControlPointMessage( unsigned int entityID, short oldOwnerID, short newOwnerID, int statusChange );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	EntityID;
	short			OldOwnerID;
	short			NewOwnerID;
	int				StatusChange;
};

struct UpgradeMessage : public Message
{
public:
	UpgradeMessage();
	UpgradeMessage( unsigned int executionFrame, short playerID, const rVector<int>& squads, int upgradeType );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	short			PlayerID;
	int				UpgradeType;
	rVector<int>	Squads;
};

struct RandomSeedMessage : public Message
{
public:
	RandomSeedMessage();
	RandomSeedMessage( unsigned int seed );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	Seed;
};

struct UserSignalMessage : public Message
{
public:
	UserSignalMessage();
	UserSignalMessage( UserSignalType::SINGAL_TYPE type, unsigned short senderID );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	UserSignalType::SINGAL_TYPE SignalType;
	unsigned short SenderID;
};

struct StepMessage : public Message
{
public:
	StepMessage();
	StepMessage( unsigned int frame, unsigned int hash, unsigned int randomCount );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int Frame;
	unsigned int Hash;
	unsigned int RandomCount;
};

struct StepResponseMessage : public Message
{
public:
	StepResponseMessage();
	StepResponseMessage( short playerID, unsigned int frame );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short			PlayerID;
	unsigned int	Frame;
};

struct NameUpdateMessage : public Message
{
public:
	NameUpdateMessage();
	NameUpdateMessage( const rVector<rString>& names, const rVector<short>& playerIDs );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	rVector<short>		PlayerIDs;
	rVector<rString>	Names;
};

struct WriteFileMessage : public Message
{
public:
	WriteFileMessage();
	WriteFileMessage( const rString& text, const rString& filePath );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	rString FilePathStr;
	rString FileTextStr;
};

struct ChatMessage : public Message
{
public:
	ChatMessage();
	ChatMessage( const rString& message, short senderID, short targetID = -1 );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	rString	MessageText;
	short	SenderID; // PlayerID
	short	TargetID; // PlayerID
};

struct ConnectionStatusUpdateMessage : public Message
{
public:
	ConnectionStatusUpdateMessage();
	ConnectionStatusUpdateMessage( short networkID, ConnectionStatusUpdates::ConnectionStatusUpdate newStatus );
	
	Message* Clone() const override;
	unsigned int GetSerializationSize() const override;
	void Serialize( char*& buffer ) const override;
	void Deserialize( const char*& buffer ) override;

	short NetworkID; //TODODB: Rename this to playerID
	ConnectionStatusUpdates::ConnectionStatusUpdate	ConnectionStatus;
};

struct LevelSelectionChangeMessage : public Message
{
public:
	LevelSelectionChangeMessage();
	LevelSelectionChangeMessage( const rString& levelName );
	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	rString LevelName;
};

struct ColourChangeMessage : public Message
{
public:
	ColourChangeMessage();
	ColourChangeMessage( short playerID, int selectedColourIndex, bool force );
	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short PlayerID;
	int SelectedColourIndex;
	bool Force;
};

struct TeamChangeMessage : public Message
{
public:
	TeamChangeMessage();
	TeamChangeMessage( short playerID, int selectedTeamIndex, bool force );
	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short PlayerID;
	int SelectedTeamIndex;
	bool Force;
};

struct SpawnPointChangeMessage : public Message
{
public:
	SpawnPointChangeMessage();
	SpawnPointChangeMessage( short playerID, int selectedTeamIndex, bool force );
	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short PlayerID;
	int SelectedSpawnPoint;
	bool Force;
};

struct SpawnCountChangeMessage : public Message
{
public:
 	SpawnCountChangeMessage();
 	SpawnCountChangeMessage( short count );
	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize(char*& buffer) const override;
	void			Deserialize(const char*& buffer) override;

	short Count;
};

struct ReserveAIMessage : public Message
{
public:
 	ReserveAIMessage();
 	ReserveAIMessage( short playerID, bool on );
	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize(char*& buffer) const override;
	void			Deserialize(const char*& buffer) override;

	short 	PlayerID;
	bool 	On; // True when reserving, false when unreserving
};

struct UpgradeCompleteMessage : public Message
{
public:
	UpgradeCompleteMessage();
	UpgradeCompleteMessage(short playerID, int upgradeType);

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short			PlayerID;
	int				UpgradeType;
};

struct SFXPingMessage : public Message
{
public:
	SFXPingMessage();
	SFXPingMessage(short playerID, short pingType);

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short			PlayerID;
	short			PingType;
};

struct UserPingMessage : public Message
{
public:
	UserPingMessage();
	UserPingMessage( short playerID, glm::vec3 position );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize( ) const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	// TODODP: Come up with variables
	short			PingingPlayerID;
	glm::vec3		WorldPosition;
};

struct SFXButtonClickMessage : public Message
{
public:
	SFXButtonClickMessage();
	SFXButtonClickMessage( const rString& buttonName );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	rString ButtonName;
};

struct PlayerTypeMessage : public Message
{
public:
	PlayerTypeMessage();
	PlayerTypeMessage( short playerType, short playerID, short networkID );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short	PlayerType;
	short	PlayerID;
	short	NetworkID;
};

struct PlacePropMessage : public Message
{
public:
	PlacePropMessage();
	PlacePropMessage( unsigned int executionFrame, float posX, float posZ, const glm::vec3& scale, const glm::quat& orientation, bool blockpath, const rString modelPath, int radius, bool isTree );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	float			PosX;
	float			PosZ;
	glm::vec3		Scale;
	glm::quat		Orientation;
	bool			BlockPath;
	rString			ModelPath;
	int				Radius;
	bool			IsTree;
};

struct PlaceResourceMessage : public Message
{
public:
	PlaceResourceMessage();
	PlaceResourceMessage( unsigned int executionFrame, float posX, float posZ, const glm::vec3& scale, const glm::quat& orientation, const rString& modelFileName );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	float			PosX;
	float			PosZ;
	glm::vec3		Scale;
	glm::quat		Orientation;
	rString			ModelFileName;
};

struct PlaceControlPointMessage : public Message
{
public:
	PlaceControlPointMessage();
	PlaceControlPointMessage( unsigned int executionFrame, float posX, float posZ, const glm::vec3& scale, const glm::quat& orientation, const rString& modelFileName );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	float			PosX;
	float			PosZ;
	glm::vec3		Scale;
	glm::quat		Orientation;
	rString			ModelFileName;
};

struct MoveObjectMessage : public Message
{
public:
	MoveObjectMessage();
	MoveObjectMessage( unsigned int executionFrame, unsigned int entityToMove, float newPosX, float newPosZ );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	unsigned int	EntityToMove;
	float			NewPosX;
	float			NewPosZ;
};

struct EditorSFXEmitterMessage : public Message
{
public:
	EditorSFXEmitterMessage();
	EditorSFXEmitterMessage(unsigned int executionFrame, unsigned int entityID, short playerId, short type, glm::vec3 pos, rString name, rString path, float timeInterval, float distanceMin, float distanceMax);

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	unsigned int	EntityID;
	short			PlayerId;
	short			Type; //0 == create, 1 == delete, 2 == modified
	glm::vec3		Pos;

	rString			Name;
	rString			Path;

	float			TimeInterval;
	float			DistanceMin;
	float			DistanceMax;
};

struct UpdateGhostEntityPositionMessage : public Message
{
	UpdateGhostEntityPositionMessage();
	UpdateGhostEntityPositionMessage( short ownerPlayerID, glm::vec3 newPosition );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short			OwnerPlayerID;
	glm::vec3		NewPosition;
};

struct UpdateGhostEntityVisibilityMessage : public Message
{
	UpdateGhostEntityVisibilityMessage();
	UpdateGhostEntityVisibilityMessage( unsigned int executionFrame, short ownerPlayerID, bool isVisible );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	short			OwnerPlayerID;
	bool			Isvisible;
};

struct UpdateGhostEntityModelMessage : public Message
{
	UpdateGhostEntityModelMessage();
	UpdateGhostEntityModelMessage( unsigned int executionFrame, short ownerPlayerID, const rString& modelName );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	Executionframe;
	short			OwnerPlayerID;
	rString			ModelName;
};

struct EditorCameraPathsMessage : public Message
{
	enum class EventType : uint64_t
	{
		NewSpline,
		DeleteSpline,
		RenameSpline,
		
		NewNode,
		DeleteNode,
		ChangeNodeTime,

		None,
	};

	EditorCameraPathsMessage( ) : Message( MessageTypes::EDITOR_CAMERA_PATHS, true ) { Event = EventType::None; }

	Message*		Clone( ) const override;
	unsigned int	GetSerializationSize( ) const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	static EditorCameraPathsMessage		NewSplineMessage( unsigned int executionFrame );
	static EditorCameraPathsMessage		DeleteSplineMessage( unsigned int executionFrame, unsigned int splineID );
	static EditorCameraPathsMessage		RenameSplineMessage( unsigned int executionFrame, unsigned int splineID, const char* newName );

	static EditorCameraPathsMessage		NewNodeMessage( unsigned int executionFrame, unsigned int splineID, glm::vec3 position, glm::quat orientation );
	static EditorCameraPathsMessage		DeleteNodeMessage( unsigned int executionFrame, unsigned int splineID, unsigned int nodeID );
	static EditorCameraPathsMessage		ChangeNodeTimeMessage( unsigned int executionFrame, unsigned int splineID, unsigned int nodeID, float timeValue );

	EventType		Event;
	unsigned int	ExecutionFrame;
	unsigned int	Spline;
	unsigned int	Node;
	rString			SplineName;
	glm::vec3		NodePosition;
	glm::quat		NodeOrientation;
	float			NodeTime;
};

struct SelectEntityMessage : public Message
{
	SelectEntityMessage();
	SelectEntityMessage( short selectingPlayerID, unsigned int entityID );

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	short			SelectingPlayerID;
	unsigned int	EntityID;
};

struct EditorParticleEmitterMessage : public Message
{
public:
	EditorParticleEmitterMessage();
	EditorParticleEmitterMessage(unsigned int executionFrame, unsigned int entityID, short playerId, short type, short particleType, glm::vec3 pos, glm::vec4 col, rString name);

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize( char*& buffer ) const override;
	void			Deserialize( const char*& buffer ) override;

	unsigned int	ExecutionFrame;
	unsigned int	EntityID;
	short			PlayerId;
	short			Type; //0 == create, 1 == delete, 2 == modified
	short			ParticleType;
	glm::vec3		Pos;
	glm::vec4		Col;

	rString			Name;
};

struct EditorTerrainBrushMessage : public Message
{
public:
	EditorTerrainBrushMessage();
	EditorTerrainBrushMessage(unsigned int executionFrame, glm::vec2 center, float strength, float hardness, float radius);

	Message*		Clone() const override;
	unsigned int	GetSerializationSize() const override;
	void			Serialize(char*& buffer) const override;
	void			Deserialize(const char*& buffer) override;

	unsigned int	ExecutionFrame;
	glm::vec2		Center;
	float			Strength;
	float			Hardness;
	float			Radius;
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
			case MessageTypes::PLAYER_TYPE:
				return tNew( PlayerTypeMessage );
			case MessageTypes::PLACE_PROP:
				return tNew( PlacePropMessage );
			case MessageTypes::PLACE_RESOUCE:
				return tNew( PlaceResourceMessage );
			case MessageTypes::PLACE_CONTROL_POINT:
				return tNew( PlaceControlPointMessage );
			case MessageTypes::MOVE_OBJECT:
				return tNew( MoveObjectMessage );
			case MessageTypes::EDITOR_SFXEMITTER:
				return tNew( EditorSFXEmitterMessage );
			case MessageTypes::UPDATE_GHOST_ENTITY_POS:
				return tNew( UpdateGhostEntityPositionMessage );
			case MessageTypes::UPDATE_GHOST_ENTITY_VISIBLILITY:
				return tNew( UpdateGhostEntityVisibilityMessage );
			case MessageTypes::UPDATE_GHOST_ENTITY_MODEL:
				return tNew( UpdateGhostEntityModelMessage );
			case MessageTypes::EDITOR_CAMERA_PATHS:
				return tNew( EditorCameraPathsMessage );
			case MessageTypes::SELECT_ENTITY:
				return tNew( SelectEntityMessage );
			case MessageTypes::EDITOR_PARTICLE_EMITTER:
				return tNew( EditorParticleEmitterMessage );
			default:
				return nullptr;
				break;
		}
	}
}