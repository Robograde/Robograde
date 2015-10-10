/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSMail.h"
#include <queue>
#include <network/NetworkInfo.h>
#include <utility/Logger.h>
#include "../../utility/GameData.h"
#include "../../utility/GameModeSelector.h"
#include "../network/SSNetworkOutput.h"

SSMail& SSMail::GetInstance( )
{
	static SSMail instance;
	return instance;
}

void SSMail::UpdateUserLayer( const float deltaTime )
{
	// Clear all subscribers user mailboxes
	for ( int i = 0; i < m_Subscribers.size( ); ++i )
	{
		m_Subscribers[i]->ClearUserMailbox( );
	}

	// Delete all user messages that were sent last update
	for ( int i = 0; i < m_DeliveredUserMessages.size( ); ++i )
		tDelete( m_DeliveredUserMessages[i] );
	m_DeliveredUserMessages.clear( );

	// Get messages for the user layer
	const Message* message;
	while ( PopUserQueue( message ) )
	{
		bool ShouldPush = false;

		for ( int i = 0; i < m_Subscribers.size(); ++i )
		{
			if ( m_Subscribers[i]->GetUserInterests() & message->Type ) // Check if the subscriber is interested in the message
				m_Subscribers[i]->AddUserMessage( message );	// Give the subscriber a reference to the message
		}
		m_DeliveredUserMessages.push_back( message ); // Save a reference to the message so that it can be deleted when all subscribers are done with it
	}
}

void SSMail::UpdateSimLayer( const float timeStep )
{
	// Clear all subscribers sim mailboxes
	for ( int i = 0; i < m_Subscribers.size( ); ++i )
	{
		m_Subscribers[i]->ClearSimMailbox( );
	}

	// Delete all sim messages that were sent last update
	for ( int i = 0; i < m_DeliveredSimMessages.size( ); ++i )
		tDelete( m_DeliveredSimMessages[i] );
	m_DeliveredSimMessages.clear( );
	
	// Get messages for the current frame
	while ( !m_SimMessagesQueue.empty() && m_SimMessagesQueue.top().first <= g_GameData.GetFrameCount() )
	{
		if ( m_SimMessagesQueue.top().first < g_GameData.GetFrameCount() )
			Logger::Log( "SSMail pushed a late message (Target frame was " + rToString( m_SimMessagesQueue.top().first ) + " and current frame is " + rToString( g_GameData.GetFrameCount() ), "SSMail", LogSeverity::WARNING_MSG );

		const Message* message = m_SimMessagesQueue.top().second;
		m_SimMessagesQueue.pop();

		bool ShouldPush = false;
		if ( g_GameModeSelector.GetCurrentGameMode().IsNetworked )
		{
			if ( g_NetworkInfo.AmIHost() )
				ShouldPush = true;
			else if ( message->CreatedFromPacket )
				ShouldPush = true;
			else if ( g_SSNetworkOutput.GetSimInterests() & message->Type )
				g_SSNetworkOutput.AddSimMessage( message );
		}
		else 
			ShouldPush = true;

		if ( ShouldPush )
		{
			for ( int i = 0; i < m_Subscribers.size(); ++i )
			{
				if ( m_Subscribers[i]->GetSimInterests() & message->Type ) // Check if the subscriber is interested in the message
					m_Subscribers[i]->AddSimMessage( message );	// Give the subscriber a reference to the message
			}
		}

		m_DeliveredSimMessages.push_back( message ); // Save a reference to the message so that it can be deleted when all subscribers are done with it
	}
}

void SSMail::Shutdown( )
{
	// Clear all subscribers sim mailboxes
	for ( int i = 0; i < m_Subscribers.size( ); ++i )
	{
		m_Subscribers[i]->ClearSimMailbox( );
	}

	// Delete all undelivered messages
	while ( !m_SimMessagesQueue.empty() )
	{
		tDelete( m_SimMessagesQueue.top().second );
		m_SimMessagesQueue.pop();
	}

	// Delete the messages that were delivered last frame
	for ( int i = 0; i < m_DeliveredSimMessages.size( ); ++i )
		tDelete( m_DeliveredSimMessages[i] );

	Subsystem::Shutdown();
}

bool SSMail::RegisterSubscriber( Subscriber* subscriberToRegister )
{
	bool result = true;
	for ( int i = 0; i < m_Subscribers.size( ); ++i )
	{
		if ( *m_Subscribers[i] == *subscriberToRegister ) // Check for duplicates
		{
			result = false;
			Logger::Log( "Attempted to register already registered subscriber \"" + subscriberToRegister->GetNameAsSubscriber() + "\"", "SSMail", LogSeverity::WARNING_MSG );
			break;
		}
	}

	if ( result )
	{
		m_Subscribers.push_back( subscriberToRegister );
		CalculateInterests( ); // Recalculate total interest mask
	}

	return result;
}

bool SSMail::UnregisterSubscriber( const Subscriber* const subscriberToUnregister )
{
	bool wasUnregistered = false;
	for ( int i = 0; i < m_Subscribers.size( ); ++i ) // Find the subscriber we want to unregister
	{
		if ( *m_Subscribers[i] == *subscriberToUnregister )
		{
			m_Subscribers.erase( m_Subscribers.begin( ) + i );
			wasUnregistered = true;
			CalculateInterests( ); // Recalculate total interest mask
			break;
		}
	}

	if ( !wasUnregistered )
		Logger::Log( "Attempted to unregister a non registered subscriber \"" + subscriberToUnregister->GetNameAsSubscriber() + "\"", "SSMail", LogSeverity::WARNING_MSG );

	return wasUnregistered;
}

unsigned int SSMail::PushToCurrentFrame( const Message& message )
{
	return PushWithDelay( message, 0 );
}

unsigned int SSMail::PushToNextFrame( const Message& message )
{
	return PushWithDelay( message, 1 );
}

unsigned int SSMail::PushWithDelay( const Message& message, const unsigned int delay )
{
	return PushToFrame( message, g_GameData.GetFrameCount( ) + delay );
}

unsigned int SSMail::PushToFrame( const Message& message, const unsigned int frameToExecute )
{
	if ( m_TotalSimInterests & message.Type ) // Check if any subscriber would like a copy sent to simulation user layer
		m_SimMessagesQueue.push( std::pair<unsigned int, const Message*>( frameToExecute, message.Clone() ) );

	if ( m_TotalUserInterests & message.Type ) // Check if any subscriber would like a copy sent to their user layer
		m_UserMessageQueue.Produce( message.Clone() ); // Push a dynamically allocated copy of the message to the user layer queue

	return frameToExecute;
}

void SSMail::PushToUserLayer( const Message& message )
{
	m_UserMessageQueue.Produce( message.Clone( ) );
}

bool SSMail::PopUserQueue( const Message*& outMessage )
{
	return m_UserMessageQueue.Consume( outMessage );
}

void SSMail::CalculateInterests( )
{
	// Reset interests
	m_TotalSimInterests		= 0;
	m_TotalUserInterests	= 0;

	// Go trough all subscribers and add their interests to the total
	for ( int i = 0; i < m_Subscribers.size( ); ++i )
	{
		m_TotalUserInterests	= m_TotalUserInterests	| m_Subscribers[i]->GetUserInterests();
		m_TotalSimInterests		= m_TotalSimInterests	| m_Subscribers[i]->GetSimInterests();
	}
}