/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once

#include "../Subsystem.h"
#include <gui/object/Text.h>
#include <gui/object/Window.h>

// Conveniency access function
#define g_SSNetworkInfoWindow SSNetworkInfoWindow::GetInstance()

class SSNetworkInfoWindow : public Subsystem
{
public:
    static SSNetworkInfoWindow& GetInstance();
    void Startup( ) override;
    void UpdateUserLayer( const float deltaTime ) override;
    void Shutdown( ) override;

private:
    // No external instancing allowed
    SSNetworkInfoWindow ( ) : Subsystem( "NetworkInfoWindow" ) {}
    SSNetworkInfoWindow ( const SSNetworkInfoWindow & rhs );
    ~SSNetworkInfoWindow ( ) {};
    SSNetworkInfoWindow& operator=(const SSNetworkInfoWindow & rhs);

	// Debug window
	const rString 	m_DebugWindowName 		= "NetworkDebugInfo";
	const int 		m_DebugWindowPosX 		= 0;
	const int 		m_DebugWindowPosY 		= 50;
	const int 		m_DebugWindowWidth 		= 300;
	const int 		m_DebugWindowHeight 	= 100;
	const int 		m_DebugWindowTitlesX	= 10;
	const int 		m_DebugWindowValuesX	= 150;
	const int 		m_DebugWindowTextSize	= 20;
	const int 		m_DebugWindowTopPadding = 10;

	GUI::Window* 	m_DebugWindow 			= nullptr;
	GUI::Text* 		m_DebugTextStepsBehind	= nullptr;
	GUI::Text* 		m_DebugTextServerIP		= nullptr;


	// Info for player
	const rString 	m_WindowName 		= "NetworkInfo";
	const int 		m_WindowPosX 		= 0;
	const int 		m_WindowPosY 		= 150;
	const int 		m_WindowWidth 		= 370;
	const int 		m_WindowHeight 		= 100;
	const int 		m_WindowValuesX		= 150;
	const int 		m_WindowTextSize	= 20;
	const int 		m_WindowTopPadding 	= 10;
	const int 		m_WindowLeftPadding	= 10;
	const int 		m_NickWidth 		= 100;
	const int 		m_IPWidth	 		= 200;
	const int 		m_PingWidth 		= 70;

	GUI::Window* 	m_Window			= nullptr;
	GUI::Text* 		m_TextPing 			= nullptr;
	GUI::Text* 		m_TextNick 			= nullptr;
	GUI::Text* 		m_TextIP 			= nullptr;
};
