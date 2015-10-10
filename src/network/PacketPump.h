/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include <utility/LocklessQueue.h>
#include <messaging/Message.h>
#include <mutex>
#include "NetworkInfo.h"
#include "NetworkLibraryDefine.h"

#define g_PacketPump PacketPump::GetInstance()

class PacketPump
{
public:
	NETWORK_API static PacketPump& GetInstance();

	NETWORK_API void					Send					( const Message& packet, const short receiverID = g_NetworkInfo.GetHostID() );
	NETWORK_API void					SendToAll				( const Message& packet );
				const Message* const	GetPacketFromQueue		( const short queueID );
				void					AddQueue				( const short queueID );
				void					RemoveQueue				( const short queueID );

private:
	// No external instancing allowed
	PacketPump( );
	PacketPump( const PacketPump& rhs );
	~PacketPump( );
	PacketPump& operator=(const PacketPump& rhs);
	
	rMap<const short, LocklessQueue<const Message*>*>			m_SendQueues;
	std::mutex													m_Lock;
};