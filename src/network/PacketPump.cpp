/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#include "PacketPump.h"

PacketPump&	PacketPump::GetInstance( )
{
	static PacketPump instance;
	return instance;
}

PacketPump::PacketPump( )
{}

PacketPump::~PacketPump( )
{
	const Message* p;
	for ( auto& sendQueuePair : m_SendQueues )
	{
		while ( sendQueuePair.second->Consume( p ) )	// Delete all enqueued packets
			tDelete( p );
		pDelete( sendQueuePair.second );				// Delete the queue
	}

}

void PacketPump::Send( const Message& packet, const short receiverID )
{
	// If the requested receiver exists
	if ( m_SendQueues.find( receiverID ) != m_SendQueues.end( ) )
		m_SendQueues.at( receiverID )->Produce( packet.Clone() );
#ifdef _DEBUG // TODODB: Use some kind of DEV flag instead
	else
		Logger::Log( "Attempted to send packet to nonexistant receiver (ID = " + rToString( receiverID ) + " )", "PacketPump", LogSeverity::WARNING_MSG );
#endif
}

void PacketPump::SendToAll( const Message& packet )
{
	if ( m_SendQueues.size( ) != 0 )
	{
		for ( auto& sendQueuePair : m_SendQueues )
			sendQueuePair.second->Produce( packet.Clone() );
	}
#ifdef _DEBUG
	else
		Logger::Log( "Attempted to send packet to all clients although no clients are connected", "PacketPump", LogSeverity::WARNING_MSG );
#endif
}

const Message* const PacketPump::GetPacketFromQueue( const short queueID )
{
	// If the requested queue exists
	if ( m_SendQueues.find( queueID ) != m_SendQueues.end() )
	{
		const Message* packet;
		if( m_SendQueues.at( queueID )->Consume( packet ) )
			return packet;
	}
	return nullptr;
}

void PacketPump::AddQueue( const short queueID )
{
	if ( m_SendQueues.find( queueID ) == m_SendQueues.end( ) )
		m_SendQueues.emplace( std::pair<short, LocklessQueue<const Message*>*>( queueID, pNew( LocklessQueue<const Message*> ) ) );
	else
		Logger::Log( "Attempted to add already existing SendQueue", "PacketPump", LogSeverity::WARNING_MSG );
}

void PacketPump::RemoveQueue( const short queueID )
{
	auto itemToRemove = m_SendQueues.find( queueID );
	if ( itemToRemove != m_SendQueues.end( ) )
	{
		LocklessQueue<const Message*>* queueToRemove = itemToRemove->second;
		m_SendQueues.erase( itemToRemove );		// Remove the queue from the list of available queues so that no more packets gets sent to it
		const Message* p;
		while ( queueToRemove->Consume( p ) )	// Delete all packets waiting to be sent
			tDelete( p );
		pDelete( queueToRemove );				// Delete the queue itself
	}
	else
		Logger::Log( "Attempted to remove nonixistent SendQueue", "PacketPump", LogSeverity::WARNING_MSG );
}