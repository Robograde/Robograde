/**************************************************
2015 Ola Enberg
***************************************************/

#pragma once

#include "../Subsystem.h"

#include <script/ScriptEngine.h>

// Conveniency access function
#define g_SSCommandQueueGFX SSCommandQueueGFX::GetInstance()

class SSCommandQueueGFX : public Subsystem
{
public:
    static SSCommandQueueGFX&	GetInstance		( );
    void						Startup			( ) override;
    void						Shutdown		( ) override;
    void						UpdateUserLayer	( const float deltaTime ) override;

private:
								// No external instancing allowed
								SSCommandQueueGFX	( ) : Subsystem( "CommandQueueGFX" ) {}
								SSCommandQueueGFX	( const SSCommandQueueGFX & rhs );
								~SSCommandQueueGFX	( ) {};
    SSCommandQueueGFX&			operator=			(const SSCommandQueueGFX & rhs);

	int							ScriptSetShowSquadPath	( IScriptEngine* scriptEngine );
	int							ScriptSetShowUnitPath	( IScriptEngine* scriptEngine );

	float						m_LineAnimationProgress;

	bool						m_ShowSquadPath;
	bool						m_ShowUnitPath;
};