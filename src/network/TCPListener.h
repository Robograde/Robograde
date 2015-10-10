/**************************************************
Zlib Copyright <2015> <Daniel "MonzUn" Bengtsson>
***************************************************/

#pragma once
#include <utility/LocklessQueue.h>
#include <thread>
#include "TCPSocket.h"

class TCPListener
{
#if PLATFORM == PLATFORM_WINDOWS
	typedef int socklen_t;
#endif

public:
	TCPListener( );
	~TCPListener( );

	bool						StartListening( const unsigned short port );
	void						StopListening();

	LocklessQueue<TCPSocket*>&	GetConnectionQueue();

	bool						SetTimeout( unsigned int seconds );

	bool						IsListening() const;

private:
	void						ListenForConnection(const unsigned short port );

	bool						m_IsListening = false;
	Socket						m_ListeningSocket;
	std::atomic_bool			m_IsRunning;
	std::thread					m_ListenerThread;
	LocklessQueue<TCPSocket*>	m_ConnectionQueue;
};