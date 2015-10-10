/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <glm/glm.hpp>
#include <gui/GUIEngine.h>

#define g_SSMainMenu SSMainMenu::GetInstance()

class SSMainMenu : public Subsystem
{
public:
	static SSMainMenu& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;

private:
	// No external instancing allowed
	SSMainMenu() : Subsystem( "MainMenu" ) {}
	SSMainMenu( const SSMainMenu& rhs );
	~SSMainMenu() {};
	SSMainMenu& operator=( const SSMainMenu& rhs );

	int	m_LeftOffset					= 64;
	GUI::Button* m_SinglePlayerButton	= nullptr;
	GUI::Button* m_MultiPlayerButton	= nullptr;
	GUI::Button* m_ReplayButton			= nullptr;
	GUI::Button* m_OptionsButton		= nullptr;
	GUI::Button* m_ExitButton			= nullptr;

	glm::ivec2 m_BackgroundSize = glm::ivec2( 512, 512 );
	glm::ivec2 m_LogoSize = glm::ivec2( 384, 128 ); 
};