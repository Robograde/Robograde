/**************************************************
2015 Jens Stjernkvist
***************************************************/

#pragma once

#include "../Subsystem.h"

// Conveniency access function
#define g_SSAudioSettings SSAudioSettings::GetInstance()

class SSAudioSettings : public Subsystem
{
	public:
		static SSAudioSettings& GetInstance();

		void Startup( ) override;

	private:
		// No external instancing allowed
		SSAudioSettings ( ) : Subsystem( "AudioSettings", "Sounds" ) {}
		SSAudioSettings ( const SSAudioSettings & rhs );
		~SSAudioSettings ( ) {};
		SSAudioSettings& operator=(const SSAudioSettings & rhs);
};
