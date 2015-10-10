/**************************************************
Copyright 2015 Henrik Johansson
***************************************************/

#pragma once
#include <gfx/ShadowMap.h>
#include <gfx/LightEngine.h>
#include "../Subsystem.h"
#include "../../picking/Picking.h"
#include "../../picking/PickingType.h"
// Conveniency access function
#define g_SSShadow SSShadow::GetInstance()

class SSShadow : public Subsystem
{
public:
    static SSShadow& GetInstance();
    void Startup( ) override;
    void UpdateUserLayer( const float deltaTime ) override;
    //void UpdateSimLayer( const float timeStep ) override;
    void Shutdown( ) override;

private:
    // No external instancing allowed
    SSShadow ( ) : Subsystem( "SSShadow" ) {}
    SSShadow ( const SSShadow & rhs );
    ~SSShadow ( ) {};
    SSShadow& operator=(const SSShadow & rhs);

	gfx::ShadowMap* m_ShadowMap;
	gfx::Light		m_DirLight;
	Picking			m_Picking;
};
