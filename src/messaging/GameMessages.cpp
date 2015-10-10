/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "GameMessages.h"
#include <utility/DataSizes.h>

using namespace SerializationUtility;
using namespace DataSizes;

//**************************Hest**************************
HestMessage::HestMessage() : Message( MessageTypes::HEST )
{}

HestMessage::HestMessage( float testFloat, int testInt ) : Message( MessageTypes::HEST )
{
	HoofDensity			= testFloat;
	TailsOnFaceCount	= testInt;
}

Message* HestMessage::Clone() const
{
	return tNew( HestMessage, *this );
}

unsigned int HestMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize( ) + FLOAT_SIZE + INT_SIZE;
}

void HestMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &HoofDensity,		FLOAT_SIZE );
	CopyAndIncrementDestination( buffer, &TailsOnFaceCount, INT_SIZE );
}

void HestMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &HoofDensity, buffer,		FLOAT_SIZE );
	CopyAndIncrementSource( &TailsOnFaceCount, buffer,	INT_SIZE );
}
  
//**************************OrderUnits**************************
OrderUnitsMessage::OrderUnitsMessage() : Message( MessageTypes::ORDER_UNITS )
{}

OrderUnitsMessage::OrderUnitsMessage( unsigned int executionFrame, int missionType, int teamID, const rVector<int>& squads, const glm::vec3& targetLocation, unsigned int targetEntity, bool overridingCommand )
	: Message( MessageTypes::ORDER_UNITS )
{
	ExecutionFrame		= executionFrame;
	MissionType			= missionType;
	TeamID				= teamID;
	Squads				= squads;
	TargetLocation		= targetLocation;
	TargetEntity		= targetEntity;
	OverridingCommand	= overridingCommand;
}

Message* OrderUnitsMessage::Clone() const
{
	return tNew( OrderUnitsMessage, *this );
}

unsigned int OrderUnitsMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize( ) + UNSIGNED_INT_SIZE + INT_SIZE + INT_SIZE + (FLOAT_SIZE * 3) + UNSIGNED_INT_SIZE + BOOL_SIZE + UNSIGNED_INT_SIZE + (INT_SIZE * static_cast<unsigned int>( Squads.size() ));
}

void OrderUnitsMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &ExecutionFrame,		UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, &MissionType,			INT_SIZE );
	CopyAndIncrementDestination( buffer, &TeamID,				INT_SIZE );
	CopyAndIncrementDestination( buffer, &TargetLocation,		FLOAT_SIZE * 3 );
	CopyAndIncrementDestination( buffer, &TargetEntity,			UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, &OverridingCommand,	BOOL_SIZE );

	unsigned int squadCount = static_cast<unsigned int>( Squads.size() );
	CopyAndIncrementDestination( buffer, &squadCount,	UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, Squads.data(), INT_SIZE * squadCount );
}

void OrderUnitsMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &ExecutionFrame, buffer,	UNSIGNED_INT_SIZE );
	CopyAndIncrementSource( &MissionType, buffer,		INT_SIZE );
	CopyAndIncrementSource( &TeamID, buffer,			INT_SIZE );
	CopyAndIncrementSource( &TargetLocation, buffer,	FLOAT_SIZE * 3 );
	CopyAndIncrementSource( &TargetEntity, buffer,		UNSIGNED_INT_SIZE );
	CopyAndIncrementSource( &OverridingCommand, buffer, BOOL_SIZE );

	unsigned int squadCount;
	CopyAndIncrementSource( &squadCount, buffer, UNSIGNED_INT_SIZE );
	Squads.reserve( squadCount );
	for ( int i = 0; i < static_cast<int>( squadCount ); ++i ) // TODODB: use insert here instead
	{
		int squadID;
		CopyAndIncrementSource( &squadID, buffer, INT_SIZE );
		Squads.push_back( squadID );
	}
}

//**************************OrderInvoke**************************
OrderInvokeMessage::OrderInvokeMessage() : Message( MessageTypes::ORDER_INVOKE )
{}

OrderInvokeMessage::OrderInvokeMessage( unsigned int exectuionFrame, const rVector<int>& squads, int command, const int team, int genericValue ) : Message( MessageTypes::ORDER_INVOKE )
{
	ExecutionFrame	= exectuionFrame;
	Squads			= squads;
	EnumCommand		= command;
	GenericValue	= genericValue;
	TeamID			= team;
}

Message* OrderInvokeMessage::Clone() const
{
	return tNew( OrderInvokeMessage, *this );
}

unsigned int OrderInvokeMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + UNSIGNED_INT_SIZE + INT_SIZE + INT_SIZE + INT_SIZE + UNSIGNED_INT_SIZE + (INT_SIZE * static_cast<unsigned int>( Squads.size() ));
}

void OrderInvokeMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &ExecutionFrame,	UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, &GenericValue,		INT_SIZE );
	CopyAndIncrementDestination( buffer, &TeamID,			INT_SIZE );
	CopyAndIncrementDestination( buffer, &EnumCommand,		INT_SIZE );

	unsigned int squadCount = static_cast<unsigned int>( Squads.size() );
	CopyAndIncrementDestination( buffer, &squadCount,	UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, Squads.data(), INT_SIZE * squadCount );
}

void OrderInvokeMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &ExecutionFrame, buffer,	UNSIGNED_INT_SIZE );
	CopyAndIncrementSource( &GenericValue, buffer,		INT_SIZE );
	CopyAndIncrementSource( &TeamID, buffer,			INT_SIZE );
	CopyAndIncrementSource( &EnumCommand, buffer,		INT_SIZE );

	unsigned int squadCount;
	CopyAndIncrementSource( &squadCount, buffer,		UNSIGNED_INT_SIZE );
	Squads.reserve( squadCount );
	for ( unsigned int i = 0; i < squadCount; ++i ) // TODODB: Use insert here instead
	{
		int squadID;
		CopyAndIncrementSource( &squadID, buffer, INT_SIZE );
		Squads.push_back( squadID );
	}
}

//**************************GameOver**************************
GameOverMessage::GameOverMessage() : Message( MessageTypes::GAME_OVER )
{}

GameOverMessage::GameOverMessage( int gameOverState ) : Message( MessageTypes::GAME_OVER )
{
	State = gameOverState;
}

Message* GameOverMessage::Clone() const
{
	return tNew( GameOverMessage, *this );
}

unsigned int GameOverMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + INT_SIZE;
}

void GameOverMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &State, INT_SIZE );
}

void GameOverMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &State, buffer, INT_SIZE );
}

//**************************AIMsg**************************
AIMessage::AIMessage() : Message( MessageTypes::AI_MSG )
{}

AIMessage::AIMessage( short aiMsg, int team, int squadID, unsigned int entityID ) : Message( MessageTypes::AI_MSG )
{
	AIMsg		= aiMsg;
	TeamID		= team;
	SquadID		= squadID;
	EntityID	= entityID;
}

Message* AIMessage::Clone() const
{
	return tNew( AIMessage, *this );
}

unsigned int AIMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + SHORT_SIZE + INT_SIZE + INT_SIZE + UNSIGNED_INT_SIZE;
}

void AIMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &AIMsg,	SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &TeamID,	INT_SIZE );
	CopyAndIncrementDestination( buffer, &SquadID,	INT_SIZE );
	CopyAndIncrementDestination( buffer, &EntityID, UNSIGNED_INT_SIZE );
}

void AIMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &AIMsg, buffer,		SHORT_SIZE );
	CopyAndIncrementSource( &TeamID, buffer,	INT_SIZE );
	CopyAndIncrementSource( &SquadID, buffer,	INT_SIZE );
	CopyAndIncrementSource( &EntityID, buffer,	UNSIGNED_INT_SIZE );
}

//**************************ResearchMessage**************************
ResearchMessage::ResearchMessage() : Message( MessageTypes::RESEARCH )
{}

ResearchMessage::ResearchMessage( short playerID, int upgradeID, int statusChange ) : Message( MessageTypes::RESEARCH )
{
	PlayerID		= playerID;
	UpgradeID		= upgradeID;
	StatusChange	= statusChange;
}

Message* ResearchMessage::Clone() const
{
	return tNew( ResearchMessage, *this );
}

unsigned int ResearchMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + SHORT_SIZE + INT_SIZE + INT_SIZE;
}

void ResearchMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &PlayerID,		SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &UpgradeID,	INT_SIZE );
	CopyAndIncrementDestination( buffer, &StatusChange, INT_SIZE );
}

void ResearchMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &PlayerID, buffer,		SHORT_SIZE );
	CopyAndIncrementSource( &UpgradeID, buffer,		INT_SIZE );
	CopyAndIncrementSource( &StatusChange, buffer,	INT_SIZE );
}

//**************************ControlPointMessage**************************
ControlPointMessage::ControlPointMessage() : Message( MessageTypes::CONTROL_POINT )
{}

ControlPointMessage::ControlPointMessage( unsigned int entityID, short oldOwnerID, short newOwnerID, int statusChange ) : Message( MessageTypes::CONTROL_POINT )
{
	EntityID			= entityID;
	OldOwnerID			= oldOwnerID;
	NewOwnerID			= newOwnerID;
	StatusChange		= statusChange;
}

Message* ControlPointMessage::Clone() const
{
	return tNew( ControlPointMessage, *this );
}

unsigned int ControlPointMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + UNSIGNED_INT_SIZE + SHORT_SIZE + SHORT_SIZE + INT_SIZE;
}

void ControlPointMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &EntityID,		UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, &OldOwnerID,	SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &NewOwnerID,	SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &StatusChange, INT_SIZE );
}

void ControlPointMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &EntityID, buffer,		UNSIGNED_INT_SIZE );
	CopyAndIncrementSource( &OldOwnerID, buffer,	SHORT_SIZE );
	CopyAndIncrementSource( &NewOwnerID, buffer,	SHORT_SIZE );
	CopyAndIncrementSource( &StatusChange, buffer,	INT_SIZE );
}

//**************************UpgradeMessage**************************
UpgradeMessage::UpgradeMessage() : Message( MessageTypes::UPGRADE )
{}

UpgradeMessage::UpgradeMessage( unsigned int executionFrame, short playerID, const rVector<int>& squads, int upgradeType ) : Message( MessageTypes::UPGRADE )
{
	ExecutionFrame	= executionFrame;
	PlayerID		= playerID;
	UpgradeType		= upgradeType;
	Squads			= squads;
}

Message* UpgradeMessage::Clone() const
{
	return tNew( UpgradeMessage, *this );
}

unsigned int UpgradeMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + UNSIGNED_INT_SIZE + SHORT_SIZE + INT_SIZE + UNSIGNED_INT_SIZE + (INT_SIZE * static_cast<unsigned int>( Squads.size() ));
}

void UpgradeMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &ExecutionFrame,	UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, &PlayerID,			SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &UpgradeType,		INT_SIZE );

	unsigned int squadCount = static_cast<unsigned int>( Squads.size() );
	CopyAndIncrementDestination( buffer, &squadCount,	UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, Squads.data(), INT_SIZE * squadCount );
}

void UpgradeMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &ExecutionFrame, buffer,	UNSIGNED_INT_SIZE );
	CopyAndIncrementSource( &PlayerID, buffer,			SHORT_SIZE );
	CopyAndIncrementSource( &UpgradeType, buffer,		INT_SIZE );

	unsigned int squadCount;
	CopyAndIncrementSource( &squadCount, buffer, UNSIGNED_INT_SIZE );
	Squads.reserve( squadCount );
	for ( unsigned int i = 0; i < squadCount; ++i )
	{
		int squadID;
		CopyAndIncrementSource( &squadID, buffer, INT_SIZE );
		Squads.push_back( squadID );
	}
}

//**************************RandomSeed**************************
RandomSeedMessage::RandomSeedMessage()
{}

RandomSeedMessage::RandomSeedMessage( unsigned int seed ) : Message( MessageTypes::RANDOM_SEED, false )
{
	Seed = seed;
}

Message* RandomSeedMessage::Clone() const
{
	return tNew( RandomSeedMessage, *this );
}

unsigned int RandomSeedMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + UNSIGNED_INT_SIZE;
}

void RandomSeedMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &Seed, UNSIGNED_INT_SIZE );
}

void RandomSeedMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &Seed, buffer, UNSIGNED_INT_SIZE );
}

//**************************UserSignal**************************
UserSignalMessage::UserSignalMessage()
{}

UserSignalMessage::UserSignalMessage( UserSignalType::SINGAL_TYPE type, unsigned short senderID ) : Message( MessageTypes::USER_SIGNAL, false )
{
	SignalType	= type;
	SenderID	= senderID;
}

Message* UserSignalMessage::Clone() const
{
	return tNew( UserSignalMessage, *this );
}

unsigned int UserSignalMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + sizeof( UserSignalType::SINGAL_TYPE ) + UNSIGNED_SHORT_SIZE; // Fix unsinged short -> short and sizeof(Signaltype)
}

void UserSignalMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &SignalType, sizeof( UserSignalType::SINGAL_TYPE ) );
	CopyAndIncrementDestination( buffer, &SenderID, UNSIGNED_SHORT_SIZE );
}

void UserSignalMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &SignalType, buffer, sizeof( UserSignalType::SINGAL_TYPE ) );
	CopyAndIncrementSource( &SenderID, buffer, UNSIGNED_SHORT_SIZE );
}

//**************************StepMessage**************************
StepMessage::StepMessage()
{}

StepMessage::StepMessage( unsigned int frame, unsigned int hash, unsigned int randomCount ) : Message( MessageTypes::STEP, false )
{
	Frame		= frame;
	Hash		= hash;
	RandomCount = randomCount;
}

Message* StepMessage::Clone() const
{
	return tNew( StepMessage, *this );
}

unsigned int StepMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + UNSIGNED_INT_SIZE + UNSIGNED_INT_SIZE + UNSIGNED_INT_SIZE;
}

void StepMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &Frame,		UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, &Hash,			UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, &RandomCount,	UNSIGNED_INT_SIZE );
}

void StepMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &Frame, buffer,			UNSIGNED_INT_SIZE );
	CopyAndIncrementSource( &Hash, buffer,			UNSIGNED_INT_SIZE );
	CopyAndIncrementSource( &RandomCount, buffer,	UNSIGNED_INT_SIZE );
}

//**************************StepResponseMessage**************************
StepResponseMessage::StepResponseMessage()
{}

StepResponseMessage::StepResponseMessage( short playerID, unsigned int frame ) : Message( MessageTypes::STEP_RESPONSE, false )
{
	PlayerID	= playerID;
	Frame		= frame;
}

Message* StepResponseMessage::Clone() const
{
	return tNew( StepResponseMessage, *this );
}

unsigned int StepResponseMessage::GetSerializationSize() const
{
	return static_cast<unsigned int>( Message::GetSerializationSize() + SHORT_SIZE + UNSIGNED_INT_SIZE );
}

void StepResponseMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &PlayerID, SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &Frame,	UNSIGNED_INT_SIZE );
}

void StepResponseMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &PlayerID, buffer,	SHORT_SIZE );
	CopyAndIncrementSource( &Frame, buffer,		UNSIGNED_INT_SIZE );
}

//**************************NameUpdateMessage**************************
NameUpdateMessage::NameUpdateMessage()
{}

NameUpdateMessage::NameUpdateMessage( const rVector<rString>& names, const rVector<short>& playerIDs ) : Message( MessageTypes::NAME_UPDATE, false )
{
	Names		= names;
	PlayerIDs	= playerIDs;
}

Message* NameUpdateMessage::Clone() const
{
	return tNew( NameUpdateMessage, *this );
}

unsigned int NameUpdateMessage::GetSerializationSize() const
{
	unsigned int contentSize = 0;
	contentSize += UNSIGNED_INT_SIZE;
	contentSize += static_cast<unsigned int>( Names.size() * UNSIGNED_INT_SIZE );
	for ( int i = 0; i < Names.size(); ++i )
		contentSize += static_cast<unsigned int>( Names[i].size() * CHAR_SIZE );

	contentSize += UNSIGNED_INT_SIZE;
	contentSize += static_cast<unsigned int>( PlayerIDs.size() ) * SHORT_SIZE;

	return Message::GetSerializationSize() + contentSize;
}

void NameUpdateMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	unsigned int vectorSize = static_cast<unsigned int>( Names.size() );
	CopyAndIncrementDestination( buffer, &vectorSize, UNSIGNED_INT_SIZE );
	for ( int i = 0; i < Names.size(); ++i )
	{
		unsigned int stringLength = static_cast<unsigned int>( Names[i].size() );
		CopyAndIncrementDestination( buffer, &stringLength,		UNSIGNED_INT_SIZE );
		CopyAndIncrementDestination( buffer, Names[i].data(),	stringLength * CHAR_SIZE );
	}

	vectorSize = static_cast<unsigned int>( PlayerIDs.size() );
	CopyAndIncrementDestination( buffer, &vectorSize, UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, PlayerIDs.data(), vectorSize * SHORT_SIZE );
}

void NameUpdateMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	unsigned int vectorSize;
	CopyAndIncrementSource( &vectorSize, buffer, UNSIGNED_INT_SIZE );
	Names.reserve( vectorSize );

	unsigned int stringLength;
	for ( unsigned int i = 0; i < vectorSize; ++i )
	{
		CopyAndIncrementSource( &stringLength, buffer, UNSIGNED_INT_SIZE );
		char* str = static_cast<char*>( tMalloc( ( stringLength * CHAR_SIZE ) + CHAR_SIZE ) );
		CopyAndIncrementSource( str, buffer, stringLength *	CHAR_SIZE );
		str[stringLength] = '\0';
		Names.push_back( rString( str ) );
		tFree( str );
	}

	CopyAndIncrementSource( &vectorSize, buffer, UNSIGNED_INT_SIZE );
	PlayerIDs.reserve( vectorSize );
 	PlayerIDs.insert( PlayerIDs.begin(), reinterpret_cast<const short*>( buffer ), reinterpret_cast<const short*>( buffer ) + vectorSize );

	buffer += SHORT_SIZE * vectorSize;
}


//**************************WriteFileMessage**************************

WriteFileMessage::WriteFileMessage()
{}

WriteFileMessage::WriteFileMessage(const rString& text, const rString& filePath) : Message(MessageTypes::WRITE_FILE, false)
{
	FileTextStr = text;
	FilePathStr = filePath;
}

Message* WriteFileMessage::Clone() const
{
	return tNew(WriteFileMessage, *this);
}

unsigned int WriteFileMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + static_cast<unsigned int>( UNSIGNED_INT_SIZE + FileTextStr.size() * CHAR_SIZE) + UNSIGNED_INT_SIZE + static_cast<unsigned int>(FilePathStr.size() * CHAR_SIZE);
}

void WriteFileMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );

	unsigned int textSize = static_cast<unsigned int>( FileTextStr.size() );
	CopyAndIncrementDestination( buffer, &textSize, UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, FileTextStr.data(), textSize * CHAR_SIZE );

	unsigned int pathSize = static_cast< unsigned int >( FilePathStr.size() );
	CopyAndIncrementDestination( buffer, &pathSize, UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, FilePathStr.data(), pathSize * CHAR_SIZE );
}

void WriteFileMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	{
		unsigned int textLength;
		CopyAndIncrementSource( &textLength, buffer, UNSIGNED_INT_SIZE );
		char* str = static_cast<char*>( tMalloc( ( textLength * CHAR_SIZE ) + CHAR_SIZE ) );
		CopyAndIncrementSource( str, buffer, textLength * CHAR_SIZE );
		str[textLength] = '\0';
		FileTextStr = rString( str );
		tFree( str );
	}
	{
		unsigned int pathLength;
		CopyAndIncrementSource( &pathLength, buffer, UNSIGNED_INT_SIZE );
		char* path = static_cast<char*>( tMalloc( ( pathLength * CHAR_SIZE ) + CHAR_SIZE ) );
		CopyAndIncrementSource( path, buffer, pathLength * CHAR_SIZE );
		path[pathLength] = '\0';
		FilePathStr = rString( path );
		tFree( path );
	}
}

//**************************ChatMessage**************************

ChatMessage::ChatMessage()
{}

ChatMessage::ChatMessage( const rString& message, short senderID, short targetID ) : Message( MessageTypes::CHAT, false )
{
	MessageText	= message;
	SenderID	= senderID;
	TargetID	= targetID;
}

Message* ChatMessage::Clone() const
{
	return tNew( ChatMessage, *this );
}

unsigned int ChatMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + static_cast<unsigned int> ( UNSIGNED_INT_SIZE + MessageText.size() * CHAR_SIZE + SHORT_SIZE + SHORT_SIZE );
}

void ChatMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	
	unsigned int stringLength = static_cast<unsigned int>( MessageText.size() );
	CopyAndIncrementDestination( buffer, &stringLength, UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, MessageText.data(), stringLength * CHAR_SIZE );
	CopyAndIncrementDestination( buffer, &SenderID, SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &TargetID, SHORT_SIZE );
}

void ChatMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	unsigned int stringLength;
	CopyAndIncrementSource( &stringLength, buffer, UNSIGNED_INT_SIZE );
	char* string = static_cast<char*>( tMalloc( stringLength + 1 ) );
	CopyAndIncrementSource( string, buffer, stringLength * CHAR_SIZE );
	string[stringLength] = '\0';
	MessageText = rString( string );
	tFree( string );
	CopyAndIncrementSource( &SenderID, buffer, SHORT_SIZE );
	CopyAndIncrementSource( &TargetID, buffer, SHORT_SIZE );
}

//**************************ConnectionStatusMessage**************************

ConnectionStatusUpdateMessage::ConnectionStatusUpdateMessage()
{}

ConnectionStatusUpdateMessage::ConnectionStatusUpdateMessage( short networkID, ConnectionStatusUpdates::ConnectionStatusUpdate newStatus ) : Message( MessageTypes::CONNECTION_STATUS, false )
{
	NetworkID			= networkID;
	ConnectionStatus	= newStatus;
}

Message* ConnectionStatusUpdateMessage::Clone() const
{
	return tNew( ConnectionStatusUpdateMessage, *this );
}

unsigned int ConnectionStatusUpdateMessage::GetSerializationSize() const
{
	return ( SHORT_SIZE + sizeof( ConnectionStatusUpdates::ConnectionStatusUpdate ) + Message::GetSerializationSize() );
}

void ConnectionStatusUpdateMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &NetworkID, SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &ConnectionStatus, sizeof( ConnectionStatusUpdates::ConnectionStatusUpdate ) );
}

void ConnectionStatusUpdateMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &NetworkID, buffer, SHORT_SIZE );
	CopyAndIncrementSource( &ConnectionStatus, buffer, sizeof( ConnectionStatusUpdates::ConnectionStatusUpdate ) );
}

//**************************LevelSelectionChangeMessage**************************

LevelSelectionChangeMessage::LevelSelectionChangeMessage( ) : Message( MessageTypes::LEVEL_SELECTION_CHANGE, false )
{}

LevelSelectionChangeMessage::LevelSelectionChangeMessage( const rString& levelName ) : Message( MessageTypes::LEVEL_SELECTION_CHANGE, false )
{
	LevelName = levelName;
}

Message* LevelSelectionChangeMessage::Clone() const
{
	return tNew( LevelSelectionChangeMessage, *this );
}

unsigned int LevelSelectionChangeMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + static_cast<unsigned int> ( UNSIGNED_INT_SIZE + LevelName.size() * CHAR_SIZE );
}

void LevelSelectionChangeMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	
	unsigned int stringLength = static_cast<unsigned int>( LevelName.size() );
	CopyAndIncrementDestination( buffer, &stringLength, UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, LevelName.data(), stringLength * CHAR_SIZE );
}

void LevelSelectionChangeMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	unsigned int stringLength;
	CopyAndIncrementSource( &stringLength, buffer, UNSIGNED_INT_SIZE );
	char* string = static_cast<char*>( tMalloc( stringLength + 1 ) );
	CopyAndIncrementSource( string, buffer, stringLength * CHAR_SIZE );
	string[stringLength] = '\0';
	LevelName = rString( string );
	tFree( string );
}

//**************************ColourChangeMessage**************************
ColourChangeMessage::ColourChangeMessage( ) : Message( MessageTypes::COLOUR_CHANGE, false )
{}

ColourChangeMessage::ColourChangeMessage( short playerID, int selectedColourIndex, bool force ) : Message( MessageTypes::COLOUR_CHANGE, false )
{
	PlayerID = playerID;
	SelectedColourIndex = selectedColourIndex;
	Force = force;
}

Message* ColourChangeMessage::Clone() const
{
	return tNew( ColourChangeMessage, *this );
}

unsigned int ColourChangeMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + static_cast<unsigned int> ( SHORT_SIZE + UNSIGNED_INT_SIZE + BOOL_SIZE );
}

void ColourChangeMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	
	CopyAndIncrementDestination( buffer, &PlayerID, SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &SelectedColourIndex, INT_SIZE );
	CopyAndIncrementDestination( buffer, &Force, BOOL_SIZE );
}

void ColourChangeMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );

	CopyAndIncrementSource( &PlayerID, buffer, SHORT_SIZE );
	CopyAndIncrementSource( &SelectedColourIndex, buffer, INT_SIZE );
	CopyAndIncrementSource( &Force, buffer, BOOL_SIZE );
}

//**************************TeamChangeMessage**************************
TeamChangeMessage::TeamChangeMessage( ) : Message( MessageTypes::TEAM_CHANGE, false )
{}

TeamChangeMessage::TeamChangeMessage( short playerID, int selectedTeamIndex, bool force ) : Message( MessageTypes::TEAM_CHANGE, false )
{
	PlayerID = playerID;
	SelectedTeamIndex = selectedTeamIndex;
	Force = force;
}

Message* TeamChangeMessage::Clone() const
{
	return tNew( TeamChangeMessage, *this );
}

unsigned int TeamChangeMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + static_cast<unsigned int> ( SHORT_SIZE + UNSIGNED_INT_SIZE + BOOL_SIZE );
}

void TeamChangeMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	
	CopyAndIncrementDestination( buffer, &PlayerID, SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &SelectedTeamIndex, INT_SIZE );
	CopyAndIncrementDestination( buffer, &Force, BOOL_SIZE );
}

void TeamChangeMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );

	CopyAndIncrementSource( &PlayerID, buffer, SHORT_SIZE );
	CopyAndIncrementSource( &SelectedTeamIndex, buffer, INT_SIZE );
	CopyAndIncrementSource( &Force, buffer, BOOL_SIZE );
}

//**************************SpawnPointChangeMessage**************************
SpawnPointChangeMessage::SpawnPointChangeMessage( ) : Message( MessageTypes::SPAWN_POINT_CHANGE, false )
{}

SpawnPointChangeMessage::SpawnPointChangeMessage( short playerID, int selectedSpawnPoint, bool force ) : Message( MessageTypes::SPAWN_POINT_CHANGE, false )
{
	PlayerID = playerID;
	SelectedSpawnPoint = selectedSpawnPoint;
	Force = force;
}

Message* SpawnPointChangeMessage::Clone() const
{
	return tNew( SpawnPointChangeMessage, *this );
}

unsigned int SpawnPointChangeMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + static_cast<unsigned int> ( SHORT_SIZE + UNSIGNED_INT_SIZE + BOOL_SIZE );
}

void SpawnPointChangeMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	
	CopyAndIncrementDestination( buffer, &PlayerID, SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &SelectedSpawnPoint, INT_SIZE );
	CopyAndIncrementDestination( buffer, &Force, BOOL_SIZE );
}

void SpawnPointChangeMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );

	CopyAndIncrementSource( &PlayerID, buffer, SHORT_SIZE );
	CopyAndIncrementSource( &SelectedSpawnPoint, buffer, INT_SIZE );
	CopyAndIncrementSource( &Force, buffer, BOOL_SIZE );
}

//**************************SpawnCountChangeMessage**************************
SpawnCountChangeMessage::SpawnCountChangeMessage() : Message(MessageTypes::CHANGE_SPAWN_COUNT, false)
{}

SpawnCountChangeMessage::SpawnCountChangeMessage( short count ) : Message(MessageTypes::CHANGE_SPAWN_COUNT, false)
{
	Count = count;
}

Message* SpawnCountChangeMessage::Clone() const
{
	return tNew(SpawnCountChangeMessage, *this);
}

unsigned int SpawnCountChangeMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + static_cast<unsigned int> (SHORT_SIZE); // TODOJM: This was wrong, make sure it is right now/
}

void SpawnCountChangeMessage::Serialize(char*& buffer) const
{
	Message::Serialize(buffer);

	CopyAndIncrementDestination(buffer, &Count, SHORT_SIZE);
}

void SpawnCountChangeMessage::Deserialize(const char*& buffer)
{
	Message::Deserialize(buffer);

	CopyAndIncrementSource(&Count, buffer, SHORT_SIZE);
}

//**************************ReserveAIMessage**************************
ReserveAIMessage::ReserveAIMessage() : Message(MessageTypes::RESERVE_AI_PLAYER, false)
{}

ReserveAIMessage::ReserveAIMessage( short playerID, bool on ) : Message(MessageTypes::RESERVE_AI_PLAYER, false)
{
	PlayerID = playerID;
	On = on;
}

Message* ReserveAIMessage::Clone() const
{
	return tNew(ReserveAIMessage, *this);
}

unsigned int ReserveAIMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + static_cast<unsigned int> (SHORT_SIZE + BOOL_SIZE);
}

void ReserveAIMessage::Serialize(char*& buffer) const
{
	Message::Serialize(buffer);

	CopyAndIncrementDestination(buffer, &PlayerID, SHORT_SIZE);
	CopyAndIncrementDestination(buffer, &On, BOOL_SIZE);
}

void ReserveAIMessage::Deserialize(const char*& buffer)
{
	Message::Deserialize(buffer);

	CopyAndIncrementSource(&PlayerID, buffer, SHORT_SIZE);
	CopyAndIncrementSource(&On, buffer, BOOL_SIZE);
}

//**************************UpgradeCompleteMessage**************************
UpgradeCompleteMessage::UpgradeCompleteMessage() : Message( MessageTypes::UPGRADE_COMPLETE, false)
{}

UpgradeCompleteMessage::UpgradeCompleteMessage(short playerID, int upgradeType ) : Message( MessageTypes::UPGRADE_COMPLETE, false)
{
	PlayerID		= playerID;
	UpgradeType		= upgradeType;
}

Message* UpgradeCompleteMessage::Clone() const
{
	return tNew( UpgradeCompleteMessage, *this );
}

unsigned int UpgradeCompleteMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + SHORT_SIZE + INT_SIZE + UNSIGNED_INT_SIZE;
}

void UpgradeCompleteMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &PlayerID,			SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &UpgradeType,		INT_SIZE );
}

void UpgradeCompleteMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &PlayerID, buffer,			SHORT_SIZE );
	CopyAndIncrementSource( &UpgradeType, buffer,		INT_SIZE );
}
//**************************SFXPingMessage**************************
SFXPingMessage::SFXPingMessage() : Message( MessageTypes::SFX_PING_MESSAGE, false)
{}

SFXPingMessage::SFXPingMessage(short playerID, short pingType) : Message( MessageTypes::SFX_PING_MESSAGE, false)
{
	PlayerID		= playerID;
	PingType		= pingType;
}

Message* SFXPingMessage::Clone() const
{
	return tNew(SFXPingMessage, *this );
}

unsigned int SFXPingMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + SHORT_SIZE + SHORT_SIZE;
}

void SFXPingMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &PlayerID, SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &PingType, SHORT_SIZE );
}

void SFXPingMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &PlayerID, buffer, SHORT_SIZE );
	CopyAndIncrementSource( &PingType, buffer, SHORT_SIZE );
}

//**************************UserPingMessage**************************
UserPingMessage::UserPingMessage( ) : Message( MessageTypes::USER_PING_MESSAGE, false )
{
}

UserPingMessage::UserPingMessage( short playerID, glm::vec3 position ) : Message( MessageTypes::USER_PING_MESSAGE, false )
{
	PingingPlayerID	= playerID;
	WorldPosition	= position;
}

Message* UserPingMessage::Clone( ) const
{
	return tNew( UserPingMessage, *this );
}

unsigned int UserPingMessage::GetSerializationSize( ) const
{
	return Message::GetSerializationSize( ) + SHORT_SIZE + FLOAT_SIZE * 3;
}

void UserPingMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );
	CopyAndIncrementDestination( buffer, &PingingPlayerID, SHORT_SIZE );
	CopyAndIncrementDestination( buffer, &WorldPosition, FLOAT_SIZE * 3 );
}

void UserPingMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	CopyAndIncrementSource( &PingingPlayerID, buffer, SHORT_SIZE );
	CopyAndIncrementSource( &WorldPosition, buffer, FLOAT_SIZE * 3 );
}

//**************************SFXPingMessage**************************
SFXButtonClickMessage::SFXButtonClickMessage() : Message( MessageTypes::SFX_BUTTON_CLICK, false)
{}

SFXButtonClickMessage::SFXButtonClickMessage( const rString& buttonName ) : Message( MessageTypes::SFX_BUTTON_CLICK, false)
{
	ButtonName = buttonName;
}

Message* SFXButtonClickMessage::Clone() const
{
	return tNew( SFXButtonClickMessage, *this );
}

unsigned int SFXButtonClickMessage::GetSerializationSize() const
{
	return Message::GetSerializationSize() + static_cast<unsigned int> ( UNSIGNED_INT_SIZE + ButtonName.size() * CHAR_SIZE );
}

void SFXButtonClickMessage::Serialize( char*& buffer ) const
{
	Message::Serialize( buffer );

	unsigned int stringLength = static_cast<unsigned int>( ButtonName.size() );
	CopyAndIncrementDestination( buffer, &stringLength, UNSIGNED_INT_SIZE );
	CopyAndIncrementDestination( buffer, ButtonName.data(), stringLength * CHAR_SIZE );
}

void SFXButtonClickMessage::Deserialize( const char*& buffer )
{
	Message::Deserialize( buffer );
	unsigned int stringLength;
	CopyAndIncrementSource( &stringLength, buffer, UNSIGNED_INT_SIZE );
	char* string = static_cast<char*>( tMalloc( stringLength + 1 ) );
	CopyAndIncrementSource( string, buffer, stringLength * CHAR_SIZE );
	string[stringLength] = '\0';
	ButtonName = rString( string );
	tFree( string );
}
