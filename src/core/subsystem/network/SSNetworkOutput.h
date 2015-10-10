/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <messaging/Subscriber.h>

// Conveniency access function
#define g_SSNetworkOutput SSNetworkOutput::GetInstance()

class SSNetworkOutput : public Subsystem, public Subscriber // TODODB: Document this class
{
public:
	static SSNetworkOutput& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;
	void UpdateSimLayer( const float timeStep ) override;

private:
	// No external instancing allowed
	SSNetworkOutput() : Subsystem( "NetworkOutput" ), Subscriber( "NetworkOutput" ) {}
	SSNetworkOutput( const SSNetworkOutput& rhs );
	~SSNetworkOutput() {};
	SSNetworkOutput& operator=( const SSNetworkOutput& rhs );

	void SendConditionally( const Message& packet );
};