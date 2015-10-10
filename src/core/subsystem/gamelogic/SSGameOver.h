/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson & David Pejtersen
***************************************************/

#pragma once
#include "../Subsystem.h"
#include "../../utility/GlobalDefinitions.h"
#include <gui/GUIEngine.h>

// Conveniency access function
#define g_SSGameOver SSGameOver::GetInstance()

enum class GameOverState : int
{
	NO_GAME_OVER,
	WON_KILL,
	WON_CONTROL_POINT_TIMEOUT,
	LOST_KILL,
	LOST_CONTROL_POINT_TIMEOUT,
};

class SSGameOver : public Subsystem
{
public:
	static SSGameOver& GetInstance( );

	void Startup() override;
	void UpdateSimLayer( const float timestep ) override;
	void Shutdown() override;

private:
	// No external instancing allowed
	SSGameOver( ) : Subsystem( "GameOver" ) { }
	SSGameOver( const SSGameOver& rhs );
	~SSGameOver( ) { };
	SSGameOver& operator=( const SSGameOver& rhs );

	void CheckPlayersDead();
	void CheckControlPoints(const float timestep);

	bool			m_PlayersDead[MAX_PLAYERS];
	bool			m_GameOver						= false;
	GameOverState	m_GameOverState					= GameOverState::NO_GAME_OVER;

	short			m_PlayerOwningAllControlPoints	= -1;
	float			m_ControlPointWinCountdown;

	GUI::Text*		m_GameOverText					= nullptr;
	GUI::Text*		m_ControlPointWinCountdownText	= nullptr;


	GUI::Window*	m_GameOverWindow;
	GUI::Sprite*	m_GameOverSprite;

	glm::ivec2		m_GameOverWindowSize = glm::ivec2( 600, 300 );
};