/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#include "SSNetworkInfoWindow.h"
#include <network/NetworkInfo.h>
#include <gui/GUIEngine.h>
#include "../../utility/GameData.h"
#include "../input/SSKeyBinding.h"
#include "SSNetworkController.h"
#include "../../CompileFlags.h"

SSNetworkInfoWindow& SSNetworkInfoWindow::GetInstance( )
{
    static SSNetworkInfoWindow instance;
    return instance;
}

void SSNetworkInfoWindow::Startup( )
{
	g_GUI.UseFont( FONT_ID_LEKTON_11 );
	{
		int yOffset = m_DebugWindowTopPadding;

		m_DebugWindow = g_GUI.AddWindow( m_DebugWindowName,
				GUI::Rectangle( m_DebugWindowPosX, m_DebugWindowPosY, m_DebugWindowWidth, m_DebugWindowHeight ), "DebugWindow", true );
		//m_DebugWindow->Open( );

		g_GUI.AddText( "", GUI::TextDefinition( "Server IP: ", m_DebugWindowTitlesX, yOffset ), m_DebugWindowName );
		m_DebugTextServerIP = g_GUI.AddText( "", GUI::TextDefinition( "TODOJM", m_DebugWindowValuesX, yOffset ), m_DebugWindowName );
		yOffset += m_DebugWindowTextSize;

		g_GUI.AddText( "", GUI::TextDefinition( "Steps behind: ", m_DebugWindowTitlesX, yOffset ), m_DebugWindowName );
		m_DebugTextStepsBehind = g_GUI.AddText( "", GUI::TextDefinition( "TODOJM", m_DebugWindowValuesX, yOffset), m_DebugWindowName );
		yOffset += m_DebugWindowTextSize;
	}

	{
		int yOffset = m_WindowTopPadding;
		int xOffset = m_WindowLeftPadding;
		m_Window = g_GUI.AddWindow( m_WindowName,
				GUI::Rectangle( m_WindowPosX, m_WindowPosY, m_WindowWidth, m_WindowHeight ), "RootWindow", true );
		//m_Window->Open( );
	
		// Titles
		g_GUI.AddText( "", GUI::TextDefinition( "Name", xOffset, yOffset), m_WindowName );
		xOffset += m_NickWidth;
		g_GUI.AddText( "", GUI::TextDefinition( "IP-adress", xOffset, yOffset), m_WindowName );
		xOffset += m_IPWidth;
		g_GUI.AddText( "", GUI::TextDefinition( "Ping", xOffset, yOffset), m_WindowName );
		xOffset = m_WindowLeftPadding;
		yOffset += m_WindowTextSize;

		// Entries
		m_TextNick = g_GUI.AddText( "", GUI::TextDefinition( "TODOJM", xOffset, yOffset ), m_WindowName );
		xOffset += m_NickWidth;
		m_TextIP = g_GUI.AddText( "", GUI::TextDefinition( "TODOJM", xOffset, yOffset ), m_WindowName );
		xOffset += m_IPWidth;
		m_TextPing = g_GUI.AddText( "", GUI::TextDefinition( "TODOJM", xOffset, yOffset ), m_WindowName );
	}
}

void SSNetworkInfoWindow::UpdateUserLayer( const float deltaTime )
{
	m_Window->Close( );
	if ( g_SSKeyBinding.ActionDown( ACTION_SHOW_NETWORK_INFO ) )
	{
		m_Window->Open();
	}
	DEV(
	if ( g_SSKeyBinding.ActionUpDown( ACTION_TOGGLE_NETWORK_DEBUG_INFO ) )
	{
		m_DebugWindow->ToggleOpen();
	}
	);
	if ( m_DebugWindow->IsOpen() )
	{
		m_DebugTextServerIP->SetText( g_NetworkInfo.GetHostIPString( ) );

		unsigned int hostStep;
		g_NetworkInfo.AmIHost() ? hostStep = g_GameData.GetFrameCount() : hostStep = g_NetworkInfo.GetHostStep();
		if ( !g_NetworkInfo.AmIHost() )
		{
			m_DebugTextStepsBehind->SetText( rToString( hostStep - g_GameData.GetFrameCount() + 1 ) ); // TODODB: Investigate why it becomes a negative number
		}
		else
		{
			m_DebugTextStepsBehind->SetText( "" );
			for ( auto& networkedPlayers : g_SSNetworkController.GetNetworkedPlayers() )
				m_DebugTextStepsBehind->GetTextDefinitionRef().Text += "Client: " + rToString( networkedPlayers.first ) + "   " + rToString( hostStep - networkedPlayers.second.FrameCounter - 1 ) + '\n'; // TODODB: Investigate why it becomes 1 to high
		}
	}
	if ( m_Window->IsOpen() )
	{
		m_TextNick->SetText( "" );
		m_TextIP->SetText( "" );
		m_TextPing->SetText( "" );

		rMap<short, ConnectionInfo> connectionInfoMap = g_NetworkInfo.GetConnections();
		for ( auto& player : g_SSNetworkController.GetNetworkedPlayers() )
		{
			auto connectionInfo = connectionInfoMap.find( player.second.NetworkID );

			m_TextNick->GetTextDefinitionRef().Text += g_SSNetworkController.GetPlayerName( player.first ) + '\n';
			if ( connectionInfo != connectionInfoMap.end() )
			{
				m_TextIP->GetTextDefinitionRef().Text += ( connectionInfo->second.Destination != nullptr ? connectionInfo->second.Destination->GetPrintableAdress() : "REDACTED" ) + '\n'; // TODODB: Make constans for the inline strings
				m_TextPing->GetTextDefinitionRef().Text += rToString( static_cast<int>( ceil( connectionInfo->second.Latency) ) ) + "ms\n";
			}
			else
			{
				m_TextIP->GetTextDefinitionRef().Text	+= "N/A\n";
				m_TextPing->GetTextDefinitionRef().Text += "N/A\n"; // TODODB: Make clients put the N/A and their own ping to server in the right place
			}
		}
	}
}

void SSNetworkInfoWindow::Shutdown( )
{
	g_GUI.DeleteObject( m_DebugWindowName );
	g_GUI.DeleteObject( m_WindowName );
}
