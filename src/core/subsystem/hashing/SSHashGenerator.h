/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"

// Conveniency access function
#define g_SSHashGenerator SSHashGenerator::GetInstance()

class SSHashGenerator : public Subsystem
{
public:
	static SSHashGenerator& GetInstance();

	void UpdateSimLayer( const float timeStep ) override;
	void Shutdown() override;

	unsigned int GetHash() const;

private:
	// No external instancing allowed
	SSHashGenerator() : Subsystem( "HashGenerator" ) {}
	SSHashGenerator( const SSHashGenerator& rhs );
	~SSHashGenerator() {};
	SSHashGenerator& operator=( const SSHashGenerator & rhs );

	unsigned int CalculatePositionHash() const;

	unsigned int m_PositionHash = 0;
};