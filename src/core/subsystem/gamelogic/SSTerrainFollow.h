/**************************************************
Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../Subsystem.h"

#define g_SSTerrainFollow SSTerrainFollow::GetInstance()		// Conveniency access function

class SSTerrainFollow : public Subsystem
{
public:
	static SSTerrainFollow&	GetInstance();

	void						Startup				( ) override;
	void						UpdateUserLayer		( const float deltaTime ) override;
	void						Shutdown			( ) override;

private:
								// No external instancing allowed
								SSTerrainFollow		( ) : Subsystem( "TerrainFollowSubsystem" ) {}
								SSTerrainFollow		( const SSTerrainFollow & rhs );
								~SSTerrainFollow	( ) {};
	SSTerrainFollow&			operator=			( const SSTerrainFollow & rhs );
};