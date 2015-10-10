/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "SSGameTimer.h"
#include "../../utility/GameData.h"
#include "../../utility/GameSpeedController.h"

SSGameTimer& SSGameTimer::GetInstance()
{
	static SSGameTimer instance;
	return instance;
}

void SSGameTimer::Startup()
{
	int posX = g_GameData.GetWindowWidth() - 256;
	int posY = g_GameData.GetWindowHeight() - 256 - 16;

	g_GUI.AddWindow( "GameTimer", GUI::Rectangle( 0, 0, g_GameData.GetWindowWidth(), g_GameData.GetWindowHeight() ), "InGameWindow" );
	g_GUI.OpenWindow( "GameTimer" );

	m_TimerBackground = g_GUI.AddSprite( "", GUI::SpriteDefinition( "", posX, posY, 256 - 128, 16, glm::vec4( 0.0f, 0.0f, 0.0f, 0.5f ) ), "GameTimer" );

	g_GUI.UseFont( FONT_ID_LEKTON_11 );

	m_TimerText = g_GUI.AddText( "", GUI::TextDefinition( "test", posX, posY, 256 - 128, 16 ), "GameTimer" );
	m_TimerText->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );
	
}

void SSGameTimer::Shutdown()
{
	m_Ticks = 0;
	g_GUI.DeleteObject( "GameTimer" );
}

void SSGameTimer::UpdateUserLayer( const float deltaTime )
{
	
}

void SSGameTimer::UpdateSimLayer( const float timeStep )
{
	const int week = 604800;
	const int day = 86400;
	const int hour = 3600;
	const int minute = 60;

	rStringStream ss;

	uint64_t seconds = m_Ticks / DEFAULT_SIMULATION_RATE;

	uint64_t weeks = seconds / week;
	if( weeks > 0 )
	{
		ss << weeks << "w ";
		seconds -= week * weeks;
	}

	uint64_t days = seconds / day;
	if( days > 0 )
	{
		ss << days << "d ";
		seconds -= day * days;
	}
	
	uint64_t hours = seconds / hour;
	if( hours > 0 )
	{
		ss << hours << "h ";
		seconds -= hour * hours;
	}

	uint64_t minutes = seconds / minute;
	if( minutes > 0 )
	{
		ss << minutes << "m ";
		seconds -= minute * minutes;
	}

	ss << seconds << "s ";

	rString time = ss.str(); 
	m_TimerText->SetText( time );

	m_Ticks++;
}
