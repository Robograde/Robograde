/**************************************************
Copyright 2015 Johan Melin & Viktor Kelkannen
***************************************************/

#pragma once

#include "../Subsystem.h"

// Conveniency access function
#define g_SSKillAll SSKillAll::GetInstance()

class SSKillAll : public Subsystem
{
	public:
		static SSKillAll& GetInstance();

		void	Startup			() override;
		void	Shutdown		() override;
		void	CrashToDesktop	();

	private:
		// No external instancing allowed
		SSKillAll ( ) : Subsystem( "KillAll" ) {}
		SSKillAll ( const SSKillAll & rhs );
		~SSKillAll ( ) {};
		SSKillAll& operator=(const SSKillAll & rhs);
};
