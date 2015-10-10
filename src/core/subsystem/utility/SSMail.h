/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <memory/Alloc.h>
#include <utility/LocklessQueue.h>
#include <messaging/Message.h>
#include <messaging/Subscriber.h>

// Conveniency access function
#define g_SSMail SSMail::GetInstance( )

class SSMail : public Subsystem
{
public:
	static SSMail&			GetInstance( );

	void					UpdateUserLayer(const float deltaTime ) override;
 	void					UpdateSimLayer(	const float timeStep ) override;
	void					Shutdown( ) override;

	bool					RegisterSubscriber(		Subscriber*					subscriberToRegister );
	bool					UnregisterSubscriber(	const Subscriber* const		subscriberToUnregister );

	unsigned int			PushToCurrentFrame(		const Message&	message );
	unsigned int			PushToNextFrame(		const Message&	message );
	unsigned int			PushWithDelay(			const Message&	message, const unsigned int delay );
	unsigned int			PushToFrame(			const Message&	message, const unsigned int frameToExecute );
	void					PushToUserLayer(		const Message&	message );
	
private:
	// No external instancing allowed
	SSMail( ) : Subsystem( "Mail" ) { }
	SSMail( const SSMail& rhs );
	~SSMail( ) { };
	SSMail& operator=( const SSMail & rhs );

	bool							PopSimulationQueue( const Message*& outMessage );
	bool							PopUserQueue(		const Message*& outMessage );
	void							CalculateInterests( );

	rVector<Subscriber*>			m_Subscribers; // TODODB: Make this const when the STL allocator can handle it
	rVector<const Message*>			m_DeliveredSimMessages;
	rVector<const Message*>			m_DeliveredUserMessages;
	LocklessQueue<const Message*>	m_UserMessageQueue;
	int								m_TotalSimInterests	 = 0;
	int								m_TotalUserInterests = 0;

	struct GreaterThanOperator
	{
		bool operator()( const std::pair<unsigned int, const Message*> lhs, const std::pair<unsigned int, const Message*> rhs )
		{
			return lhs.first > rhs.first;
		}
	};
	std::priority_queue<std::pair<unsigned int, const Message*>, std::vector<std::pair<unsigned int, const Message*>>, GreaterThanOperator>	m_SimMessagesQueue; // TODODB: Use the correct allocator
};