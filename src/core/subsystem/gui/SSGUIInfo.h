/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>

#define g_SSGUIInfo SSGUIInfo::GetInstance()



class SSGUIInfo : public Subsystem
{
public:
	static SSGUIInfo& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;

	void DisplayMessage( const rString& text, float time = 5.0f );

private:
	// No external instancing allowed
	SSGUIInfo() : Subsystem( "GUIInfo" ) {}
	SSGUIInfo( const SSGUIInfo& rhs );
	~SSGUIInfo() {};
	SSGUIInfo& operator=( const SSGUIInfo& rhs );
	
	GUI::Window*	m_InfoWindow1;
	GUI::Text*		m_InfoText1;
	
	GUI::Window*	m_InfoWindow2;
	GUI::Text*		m_InfoText2;
	
	bool			m_KeepOpen = true;
	bool			m_WantToClose = false;
	bool			m_WantToOpen = false;
	
	const float		m_TimeOut = 0.3f;
	float			m_TimeLeft = 0.0f;
	
	const glm::ivec2 m_InfoWindow1Size = glm::ivec2( 352, 146 );
	const glm::ivec2 m_InfoWindow2Size = glm::ivec2( 288, 64 );

	GUI::Window*	m_PlayerGenericHepWindow;
	GUI::Text*		m_PlayerGenericHelpText;
	GUI::Text*		m_PlayerGenericHelpTextShadow;

	float			m_MsgTimerTimer = 0.0f;
	float			m_MsgTimerTotalTime = 5.0f;
	bool			m_DisplayHelpMsg = false;
};