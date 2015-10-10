/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>

#define g_SSGameTimer SSGameTimer::GetInstance()



class SSGameTimer : public Subsystem
{
public:
	static SSGameTimer& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;
	void UpdateSimLayer( const float timeStep ) override;
	

private:
	// No external instancing allowed
	SSGameTimer() : Subsystem( "GameTimer" ) {}
	SSGameTimer( const SSGameTimer& rhs );
	~SSGameTimer() {};
	SSGameTimer& operator=( const SSGameTimer& rhs );



	GUI::Text*		m_TimerText;
	GUI::Sprite*	m_TimerBackground;


	uint64_t m_Ticks = 0;
};