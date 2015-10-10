/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>
#include <messaging/Subscriber.h>
#include "../../utility/PlayerData.h"

#define g_SSInGameMenu SSInGameMenu::GetInstance()

class SSInGameMenu : public Subsystem, public Subscriber
{
public:
	static SSInGameMenu& GetInstance();

	void	Startup() override;
	void	Shutdown() override;
	void	UpdateUserLayer( const float deltaTime ) override;

private:
	enum class ConnectionState
	{
		EnteringIP,
		WaitingToConnect,
		Connected,
		Hosting
	};
	// No external instancing allowed
	SSInGameMenu() : Subsystem( "InGameMenu" ), Subscriber( "InGameMenu" ) {}
	SSInGameMenu( const SSInGameMenu& rhs );
	~SSInGameMenu() {};
	SSInGameMenu& operator=( const SSInGameMenu& rhs );

	void OnResumeButtonClicked();
	void OnPauseButtonClicked();
	void OnMainMenuButtonClicked();
	void OnQuitButtonClicked();

	void OnClientDisconnect( short clientID );

	rVector<int>		m_NetworkCallbackHandles;

	const rString 		m_WindowNameParent				= "RootWindow";
	const rString 		m_WindowNameInGameMenu			= "InGameMenu";
	const rString 		m_ScriptNameResume				= "GE_ResumeGame";
	const rString		m_ScriptNamePause				= "GE_Pause";
	const rString		m_ScriptNameMainMenu			= "GE_MainMenu";
	const rString 		m_ScriptNameQuit				= "GE_Quit";

	GUI::Window* 		m_WindowInGameMenu				= nullptr;
	GUI::Button* 		m_ButtonResume	 				= nullptr;
	GUI::Button*		m_ButtonMainMenu				= nullptr;
	GUI::Button* 		m_ButtonQuit 					= nullptr;
	GUI::Button*		m_ButtonPause					= nullptr;
	GUI::Text* 			m_TextPause						= nullptr;

	const int 			m_WidthButton					= 256;
	const int 			m_HeightButton					= 64;
	const int 			m_HeightPauseText 				= 30;
	const int			m_ButtonOffsetY					= 10;
};