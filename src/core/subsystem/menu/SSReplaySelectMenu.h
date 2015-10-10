/**************************************************
Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>

#define g_SSReplaySelectMenu SSReplaySelectMenu::GetInstance()

class SSReplaySelectMenu : public Subsystem
{
public:
	static SSReplaySelectMenu& GetInstance();

	void	Startup() override;
	void	UpdateUserLayer( const float deltaTime ) override;
	void	Shutdown() override;

	bool	HasReplays() const;

private:
	// No external instancing allowed
	SSReplaySelectMenu() : Subsystem( "ReplaySelectMenu" ) {}
	SSReplaySelectMenu( const SSReplaySelectMenu& rhs );
	~SSReplaySelectMenu() {};
	SSReplaySelectMenu& operator=( const SSReplaySelectMenu& rhs );

	const rString 		m_ReplaySelectWindowName	= "ReplaySelectWindow";
	const rString		m_ReplayFolderPath			= "../../../replay/";

	GUI::ComboBox*		m_ReplaySelector			= nullptr;
};
