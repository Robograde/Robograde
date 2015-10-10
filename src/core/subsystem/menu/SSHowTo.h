/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <glm/glm.hpp>
#include <gui/GUIEngine.h>

#define g_SSHowTo SSHowTo::GetInstance()

class SSHowTo : public Subsystem
{
public:
	static SSHowTo& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;

private:
	// No external instancing allowed
	SSHowTo() : Subsystem( "HowTo" ) {}
	SSHowTo( const SSHowTo& rhs );
	~SSHowTo() {};
	SSHowTo& operator=( const SSHowTo& rhs );

	void LoadTextures();

	const rString m_WindowName = "HowTo";

	glm::ivec2 m_BackgroundSize = glm::ivec2( 1000, 600 );
	glm::ivec2 m_LogoSize = glm::ivec2( 384, 128 ); 

	GUI::Sprite* m_Image;

	rVector<gfx::Texture*> m_Textures;

	GUI::Button* m_ButtonPrevious = nullptr;
	GUI::Button* m_ButtonNext = nullptr;

	int			m_HowToIndex = 0;
};
