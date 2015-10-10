/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>

#define g_SSGUIUpdate SSGUIUpdate::GetInstance()

class SSGUIUpdate : public Subsystem
{
public:
	static SSGUIUpdate& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;

private:
	// No external instancing allowed
	SSGUIUpdate() : Subsystem( "GUIUpdate" ) {}
	SSGUIUpdate( const SSGUIUpdate& rhs );
	~SSGUIUpdate() {};
	SSGUIUpdate& operator=( const SSGUIUpdate& rhs );
	
	
	GUI::Sprite*			m_MouseCursor;
};