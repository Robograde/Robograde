/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "../Subsystem.h"
#include <gui/GUIEngine.h>
#include <SDL2/SDL_scancode.h>
#include <input/KeyBindingCollection.h>
#include "../../input/Action.h"

#define g_SSOptionsMenu SSOptionsMenu::GetInstance()

class SSOptionsMenu : public Subsystem
{
public:
	static SSOptionsMenu& GetInstance();

	void Startup() override;
	void Shutdown() override;
	void UpdateUserLayer( const float deltaTime ) override;

private:
	// No external instancing allowed
	SSOptionsMenu() : Subsystem( "OptionsMenu" ) {}
	SSOptionsMenu( const SSOptionsMenu& rhs );
	~SSOptionsMenu() {};
	SSOptionsMenu& operator=( const SSOptionsMenu& rhs );
	
	void			StartUpGameWindow();
	void			StartupAudioWindow();
	void			StartupGraphicalWindow();
	void 			StartupControlsWindow();
	void 			CreateKeyBindingButtons( );
	void 			ResetIsInputting( );
	
	void			ApplyOptions();
	
	const rString 		m_OptionsWindowName = "Options";
	const rString		m_GameOptionsWindowname = "GameOptionsWindow";
	const rString		m_GraphicalOptionsWindowName = "GraphicalOptionsWindow";
	const rString 		m_ControlOptionsWindowName = "ControlOptionsWindow";
	const rString		m_AudioOptionsWindowname = "AudioOptionsWindow";
	
	//Game menu
	GUI::TextBox*		m_PlayerName;
	
	//Audio menu
	GUI::Slider*		m_VolumeMaster;
	GUI::Slider*		m_VolumeMusic;
	GUI::Slider*		m_VolumeSFX;
	GUI::Text*			m_VolumeMasterText;
	GUI::Text*			m_VolumeMusicText;
	GUI::Text*			m_VolumeSFXText;
	
	
	GUI::Button*		m_MusicToggle;
	GUI::Button*		m_SFXToggle;
	
	//Graphics menu
	GUI::ComboBox*		m_ScreenResolutions;
	GUI::ComboBox*		m_TextureQuality;
	rVector<glm::ivec2> m_Resolutions;
	GUI::Button*		m_FullscreenToggle;
	GUI::Button*		m_MouseLockToggle;
	GUI::Button*		m_VsyncToggle;
	
	//Controls menu
	const rString 		m_NameWindowKeybindings = "KeybindingsWindow";
	const rString 		m_ScriptNameResetToDefault = "GE_KeyBindingsResetToDefault";
	const rString 		m_ScriptNameBindKey = "GE_BindKey";
	const int 			m_PaddingTopKeybindings = 50;
	const int 			m_PaddingBottomKeyBindings = 60;
	const int 			m_KeybindingsButtonWidth = 128;
	const int 			m_KeybindingsButtonHeight = 26;
	const int 			m_KeybindingsButtonVerticalSpacing = 0;
	const int 			m_KeybindingsButtonHorizontalSpacing = 8;
	const int 			m_WindowIsInputtingHeight = 200;
	const int 			m_HeightKeybindingsButtonResetToDefault = 32;
	const int 			m_WidthKeybindingsButtonResetToDefault = 200;

	SDL_Scancode		m_LatestPressedKey = SDL_SCANCODE_UNKNOWN;
	ACTION				m_ActionToChange = ACTION::ACTION_SIZE;
	bool				m_IsInputtingPrimary = false;
	bool 				m_WantsToReinitializeKeybindingButtons = false;

	rMap<ACTION, GUI::Button*> 	m_PrimaryButtons;
	rMap<ACTION, GUI::Button*> 	m_SecondaryButtons;
	KeyBindingCollection 		m_KeyBindingCollection;
	GUI::Window* 				m_WindowIsInputting = nullptr;
	GUI::Window* 				m_WindowKeybindings = nullptr;
	const rString 				m_WindowNameIsInputting = "InputtingWindow";
	GUI::Text* 					m_ErrorText = nullptr;
	
	bool 						m_WantsInputKey = false;
};
