/**************************************************
Copyright 2015 Ola Enberg
***************************************************/

#pragma once

#include "../Subsystem.h"

// Conveniency access function
#define g_SSStartupScript SSStartupScript::GetInstance()

class SSStartupScript : public Subsystem
{
public:
    static SSStartupScript& GetInstance();

    void Startup( ) override;

    private:
        // No external instancing allowed
        SSStartupScript ( ) : Subsystem( "StartupScriptsName" ) {}
        SSStartupScript ( const SSStartupScript & rhs );
        ~SSStartupScript ( ) {};
        SSStartupScript& operator=(const SSStartupScript & rhs);
};