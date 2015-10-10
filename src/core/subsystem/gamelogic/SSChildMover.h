/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#pragma once

#include "../Subsystem.h"

#define g_SSChildMover SSChildMover::GetInstance()


class SSChildMover : public Subsystem
{
public:
	static SSChildMover&	GetInstance( );

	void					UpdateSimLayer( const float deltaTime ) override;

private:
							SSChildMover( ) : Subsystem( "ChildMover" ) {}
							SSChildMover( const SSChildMover & rhs );
							~SSChildMover( ) {}
	SSChildMover&			operator= (const SSChildMover & rhs);
};

