/**************************************************
2015 Johan Melin
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <utility/FrameCounter.h>
#include <gui/GUIEngine.h>

// Conveniency access function
#define g_SSFrameCounter SSFrameCounter::GetInstance()

class SSFrameCounter : public Subsystem
{
public:
	static SSFrameCounter& GetInstance( );

	void Startup( ) override;
	void UpdateUserLayer( const float deltaTime ) override;
	void Shutdown( ) override;
	
	void ResetMaxFrameTime( );

private:
	// No external instancing allowed
	SSFrameCounter( ) : Subsystem( "FrameCounter" ) {}
	SSFrameCounter( const SSFrameCounter& rhs );
	~SSFrameCounter( ) {}
	SSFrameCounter& operator=(const SSFrameCounter& rhs);

	FrameCounter 		m_FrameCounter;
	GUI::Window	*		m_Window;
	GUI::Text*			m_Text;
};
