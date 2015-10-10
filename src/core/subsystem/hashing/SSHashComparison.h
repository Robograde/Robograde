/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"

// Conveniency access function
#define g_SSHashComparison SSHashComparison::GetInstance()

class SSHashComparison : public Subsystem
{
public:
	static SSHashComparison& GetInstance();

	void UpdateSimLayer( const float timeStep ) override;
	void Shutdown() override;

private:
	// No external instancing allowed
	SSHashComparison() : Subsystem( "HashComparison" ) {}
	SSHashComparison( const SSHashComparison& rhs );
	~SSHashComparison() {};
	SSHashComparison& operator=( const SSHashComparison & rhs );

	bool				m_OutOfSync						= false;
	rString				m_DesynchronizationCause		= DefaultDesynchronizationCause;

	const rString		DefaultDesynchronizationCause	= "SSHashing failed to set cause";
};