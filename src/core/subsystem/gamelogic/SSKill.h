/**************************************************
2015 Johan Melin & Viktor Kelkkanen
***************************************************/

#pragma once

#include "../Subsystem.h"

#include "../../datadriven/ComponentTypes.h"

// Conveniency access function
#define g_SSKill SSKill::GetInstance()

class SSKill : public Subsystem
{
public:
	static SSKill&	GetInstance( );

	void			Startup			() override;
	void			UpdateSimLayer	( const float timeStep ) override;

	void			KillEntity		( Entity entityID );

private:
	// No external instancing allowed
	SSKill( ) : Subsystem( "Kill" ) {}
	SSKill( const SSKill& rhs );
	~SSKill( ) {};
	SSKill& operator=(const SSKill& rhs);
};
