/**************************************************
Copyright 2015 Johan Melin
***************************************************/

#include "SSFrameCounter.h"
#include <input/Input.h>
#include <iostream>
#include "../input/SSConsole.h"
#include "../../CompileFlags.h"

SSFrameCounter& SSFrameCounter::GetInstance( )
{
	static SSFrameCounter instance;
	return instance;
}

void SSFrameCounter::Startup( )
{
	m_Window = g_GUI.AddWindow("FrameCounterWindow", GUI::Rectangle( g_GUI.GetWindowSize("DebugWindow").x - 280, 0, 280, 80), "DebugWindow", true );
	m_Window->SetClickThrough( true );

	DEV( m_Window->Open(); )
	
	g_GUI.UseFont( FONT_ID_LEKTON_11 );
	
	m_Text = g_GUI.AddText( "", GUI::TextDefinition("", 5, 0 ), "FrameCounterWindow" );
	m_Text->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_LEFT );
}

void SSFrameCounter::UpdateUserLayer( const float deltaTime )
{
	m_FrameCounter.TickFrame();
	
	if ( m_Window->IsOpen() )
	{
		if( g_GUI.IsWindowOpen( "ConsoleWindow" ) )
			m_Window->SetPosition( m_Window->GetPosition().x, g_GUI.GetWindowSize( "ConsoleWindow" ).y );
		else
			m_Window->SetPosition( m_Window->GetPosition().x, 0 );
				
		m_Text->SetText( 
			//"Frametime: " + m_FrameCounter.GetMSString() + "ms\n" +
			"Maximum frame time: " + m_FrameCounter.GetMaxFrameTimeString() + "ms\n" +
			"Average frametime: " + m_FrameCounter.GetAverageMSString() + "ms\n" +
			"Frames per second: " + m_FrameCounter.GetFPSString() + "fps\n" +
			"Average frames per second: " + m_FrameCounter.GetAverageFPSString() + "fps\n" );
	}
}

void SSFrameCounter::Shutdown()
{
	g_GUI.DeleteObject( "FrameCounterWindow" );
}

void SSFrameCounter::ResetMaxFrameTime( )
{
	m_FrameCounter.ResetMaxFrameTime( );
}
