/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>

#define g_SSGUIRender SSGUIRender::GetInstance()

class SSGUIRender : public Subsystem
{
public:
	static SSGUIRender& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;

private:
	// No external instancing allowed
	SSGUIRender() : Subsystem( "GUIRender" ) {}
	SSGUIRender( const SSGUIRender& rhs );
	~SSGUIRender() {};
	SSGUIRender& operator=( const SSGUIRender& rhs );
	
	
	GUI::Sprite*			m_MouseCursor;
};