/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>

#define g_SSHealthBar SSHealthBar::GetInstance()


class SSHealthBar : public Subsystem
{
public:
	static SSHealthBar& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;

private:
	// No external instancing allowed
	SSHealthBar() : Subsystem( "HealthBar" ) {}
	SSHealthBar( const SSHealthBar& rhs );
	~SSHealthBar() {};
	SSHealthBar& operator=( const SSHealthBar& rhs );
	
	rVector<GUI::ProgressBar*> m_HealthBars;
	
	const char*				m_HealthBarWindowName = "HealthBarWindow";
	const int m_BarYOffset = 5;
	
	const int m_HealthBarWidth = 32;
	const int m_HealthBarHeight = 8;
	
	const glm::vec4			m_NeutralBarColour = glm::vec4( 0.7f, 0.7f, 0.7f, 1.0f );
	const glm::vec4			m_EnemyBarColour = glm::vec4( 0.7f, 0.1f, 0.1f, 1.0f );
	const glm::vec4			m_FriendlyBarColour = glm::vec4( 0.1f, 0.1f, 0.7f, 1.0f );
	const glm::vec4			m_OwnBarColour = glm::vec4( 0.1f, 0.7f, 0.1f, 1.0f );
};