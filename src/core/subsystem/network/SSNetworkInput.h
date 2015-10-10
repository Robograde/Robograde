/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <messaging/Message.h>

// Conveniency access function
#define g_SSNetworkInput SSNetworkInput::GetInstance()

class SSNetworkInput : public Subsystem
{
public:
	static SSNetworkInput& GetInstance( );

	void UpdateUserLayer( const float deltaTime ) override;
	void UpdateSimLayer( const float timeStep ) override;

private:
	// No external instancing allowed
	SSNetworkInput( ) : Subsystem( "NetworkInput" ) { }
	SSNetworkInput( const SSNetworkInput& rhs );
	~SSNetworkInput( ) { };
	SSNetworkInput& operator=( const SSNetworkInput& rhs );

	void ForwardToUserLayer( Message& message ) const;
	void ForwardToSimLayer( Message& message ) const;
	void ForwardToSimLayer( Message& message, unsigned int executionFrame ) const;
};