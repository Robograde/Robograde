/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <utility/CallbackConfig.h>
// Conveniency access function
#define g_SSGraphicsSettings SSGraphicsSettings::GetInstance()

class SSGraphicsSettings : public Subsystem
{
public:
    static SSGraphicsSettings& GetInstance();
    void Startup( ) override;
    void Shutdown( ) override;

private:
    // No external instancing allowed
    SSGraphicsSettings ( ) : Subsystem( "GraphicsSettings" ) {}
    SSGraphicsSettings ( const SSGraphicsSettings & rhs );
    ~SSGraphicsSettings ( ) {};
    SSGraphicsSettings& operator=(const SSGraphicsSettings & rhs);

	void GraphicsConfigChangeCallback( CallbackConfig* cfg );

	const rString m_GraphicsConfigPath 	= "graphics.cfg";
	CallbackConfigRegisterID m_CallbackConfigRegisterID	= 0;
};
