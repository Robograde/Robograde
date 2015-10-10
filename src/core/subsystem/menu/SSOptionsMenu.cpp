/**************************************************
Copyright 2015 Isak Almgren
***************************************************/

#include "SSOptionsMenu.h"
#include "../../utility/GameData.h"
#include <SDL2/SDL.h>
#include <utility/Logger.h>
#include <input/KeyBindings.h>
#include <gfx/GraphicsEngine.h>
#include <utility/ConfigManager.h>
#include <input/Input.h>
#include "../input/SSKeyBinding.h"

#include "../input/SSKeyBinding.h"
#include "../../utility/GameModeSelector.h"
#include "../audio/SSAudio.h"

SSOptionsMenu& SSOptionsMenu::GetInstance()
{
	static SSOptionsMenu instance;
	return instance;
}

void SSOptionsMenu::Startup()
{
	glm::ivec2 windowSize = g_GUI.GetWindowSize( "RootWindow" );
	
	g_GUI.AddWindow( m_OptionsWindowName, GUI::Rectangle( 0, 0, windowSize.x, windowSize.y ), "RootWindow" );
	
	g_GUI.AddSprite( "bg", GUI::SpriteDefinition( "", 0, 0, windowSize.x, windowSize.y, glm::vec4( 0.05f, 0.05f, 0.07f, 1.0f ) ), m_OptionsWindowName ); 

	g_GUI.UseFont( FONT_ID_LEKTON_20 );
	int btnWidth = 256;
	int btnHeight = 64;
	int btnSpacing = btnWidth + 1;
	int numButtons = 4;
	int x = ( windowSize.x / 2 ) - (((btnSpacing * numButtons) - 10) / 2 ); //Magic centering of buttons :)

	//Lambda to make sure buttons all get the same design, TODOIA: Think about making this into some kind of style or something
	auto setButtonDesign = [] ( GUI::Button* btn )
	{
		btn->SetBackgroundImage( "Button.png" );
		btn->SetColour( glm::vec4( 1.0f ) ); 
		btn->SetHighLightColour( glm::vec4( 0.8f, 1.0f, 1.0f, 1.0f ) );
		btn->GetBackgroundRef().BorderSize = 0;
	};
	//Lambda function to make the addition of more menu buttons easier
	auto addOptionPage = [this, windowSize, x, btnWidth, btnHeight, btnSpacing, setButtonDesign] ( const rString& windowName, const rString& buttonText ) mutable -> GUI::Button*
	{
		int y = 32;
		int topMargin = 128;
		int bottomMargin = 128;
		
		GUI::Button* btn = g_GUI.AddButton( buttonText, GUI::Rectangle( x, y, btnWidth, btnHeight ), m_OptionsWindowName );
		btn->SetClickScript( "ToggleWindowGroup('" + windowName + "', 'OptionsTabWindows' ); GE_ControlsStopBind();" );
		btn->SetText( buttonText );
		btn->SetToggleGroup( "OptionsButtons" );

		setButtonDesign( btn );
		GUI::Window* window = g_GUI.AddWindow( windowName, GUI::Rectangle( 0, topMargin, windowSize.x, windowSize.y - topMargin - bottomMargin ), m_OptionsWindowName, true );
		window->SetToggleGroup( "OptionsTabWindows" );
		
		x += btnSpacing;
		
		return btn;
	};
	//Back and appply buttons
	GUI::Button* btn = g_GUI.AddButton( "", GUI::Rectangle( (windowSize.x / 2) - btnWidth, windowSize.y - btnHeight - 32, btnWidth, btnHeight ), m_OptionsWindowName );
	btn->SetText( "Cancel" );
	btn->SetClickScript( "SwitchGameMode('mm')" );
	setButtonDesign( btn );
	
	btn = g_GUI.AddButton( "", GUI::Rectangle( (windowSize.x / 2) , windowSize.y - btnHeight - 32, btnWidth, btnHeight ), m_OptionsWindowName );
	btn->SetText( "Apply" );
	btn->SetClickScript( "GE_ApplyOptions()" );
	setButtonDesign( btn );

	//Remember to increase numButtons if more buttons are added
	GUI::Button* gameButton = addOptionPage( m_GameOptionsWindowname, "Game" );
	addOptionPage( m_AudioOptionsWindowname, "Audio" );
	addOptionPage( m_GraphicalOptionsWindowName, "Graphics" );
	addOptionPage( m_ControlOptionsWindowName, "Controls" );
	
	//Script functions that are to be called by buttons
	g_Script.Register( "GE_ApplyOptions", [this] ( IScriptEngine* ) -> int { ApplyOptions(); return 0; } );
	g_Script.Register( "GE_ControlsStopBind", [this] ( IScriptEngine* ) -> int { ResetIsInputting(); return 0; } );
	
	StartupAudioWindow( );
	StartupGraphicalWindow( );
	StartupControlsWindow( );
	StartUpGameWindow();

	gameButton->SimulateClick();
}

void SSOptionsMenu::Shutdown()
{
	g_GUI.DeleteObject( m_OptionsWindowName );
	g_GUI.ClearButtonGroup( "OptionsButtons" );
	g_GUI.ClearWindowGroup( "OptionsTabWindows" );
	m_WantsInputKey = false;
	m_PrimaryButtons.clear( );
	m_SecondaryButtons.clear( );
}

void SSOptionsMenu::UpdateUserLayer( const float deltaTime )
{
	if ( m_WantsInputKey )
	{
		const rVector<SDL_Scancode>* pressStack = &g_Input->GetPressStack();
		if ( pressStack->size( ) > 0 )
		{
			if ( g_SSKeyBinding.ConsumeFromPressStack( ACTION_GUI_ABORT ) )
			{
				ResetIsInputting( );
			}
			else
			{
				m_LatestPressedKey = pressStack->at( pressStack->size( ) - 1 );
				rString errorString = "";
				if ( m_KeyBindingCollection.AddMappingWithScancode( m_LatestPressedKey, g_SSKeyBinding.GetActionIdentifier( m_ActionToChange ), m_IsInputtingPrimary ? KeyBindingType::Primary : KeyBindingType::Secondary, true, false, &errorString ) )
				{
					rMap<ACTION, GUI::Button*>* toChange = m_IsInputtingPrimary ? &m_PrimaryButtons : &m_SecondaryButtons;
					auto it = toChange->find( m_ActionToChange );
					if ( it != toChange->end( ) )
					{
						it->second->SetText( SDL_GetScancodeName( m_LatestPressedKey ) );
						ResetIsInputting();
					}
				}
				else
				{
					m_ErrorText->SetText( errorString );
				}
			}
		}
	}
	
	if ( g_GUI.IsWindowOpen( m_OptionsWindowName ) )
	{
		if ( g_SSKeyBinding.ConsumeFromPressStack( ACTION_GUI_ABORT ) )
		{
			g_GameModeSelector.SwitchToGameMode( GameModeType::MainMenu );
		}
	}
	
	if( g_GUI.IsWindowOpen( m_AudioOptionsWindowname ) )
	{
		char num[5];
		sprintf( num, "%.2f", m_VolumeMaster->GetValue() );
		m_VolumeMasterText->GetTextDefinitionRef().Text = num;
		sprintf( num, "%.2f", m_VolumeMusic->GetValue() );
		m_VolumeMusicText->GetTextDefinitionRef().Text = num;
		sprintf( num, "%.2f", m_VolumeSFX->GetValue() );
		m_VolumeSFXText->GetTextDefinitionRef().Text = num;
	}

	// Hax to avoid recreating buttons while iterating over buttons
	if ( m_WantsToReinitializeKeybindingButtons )
	{
		g_KeyBindings.GetDefault( m_KeyBindingCollection ); 
		CreateKeyBindingButtons( );
	}
}

void SSOptionsMenu::StartUpGameWindow()
{
	CallbackConfig* networkConfig = g_ConfigManager.GetConfig( "network.cfg" );
	GUI::Text* label;
	label = g_GUI.AddText( "", GUI::TextDefinition( "Player name: ", 0, 50, 400, 32 ), m_GameOptionsWindowname );
	label->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_RIGHT );
	m_PlayerName = g_GUI.AddTextBox( "", GUI::Rectangle( 400, 50, 256, 32 ), m_GameOptionsWindowname );
	m_PlayerName->SetText( networkConfig->GetString( "PlayerName", "ANON" ) );
	
}

void SSOptionsMenu::StartupAudioWindow()
{
	int x = 400;
	int y = 50;
	int slideroffset = 16;
	int height = 32;
	int width = 256;
	GUI::Text* label;
	
	auto addVolumeSlider = [&]( const rString text, GUI::Slider** slider, GUI::Text** textDisplay ) mutable
	{
		GUI::Text* label = g_GUI.AddText( "", GUI::TextDefinition( text.c_str(), 0, y, x, height ), m_AudioOptionsWindowname );
		label->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_RIGHT );
		*slider = g_GUI.AddSlider( "", GUI::Rectangle( x + slideroffset, y, width, height ), m_AudioOptionsWindowname );
		(*slider)->SetBackgroundImage( "Button_256x16.png" );
		(*slider)->SetSliderImage( "Button_32.png" );

		*textDisplay = g_GUI.AddText( "", GUI::TextDefinition( "", x + width + slideroffset * 2, y ), m_AudioOptionsWindowname );
		
		y += 32;
	};
	
	addVolumeSlider( "Master Volume: ", &m_VolumeMaster, &m_VolumeMasterText );
	addVolumeSlider( "Music Volume: ", &m_VolumeMusic, &m_VolumeMusicText );
	addVolumeSlider( "SFX Volume: ", &m_VolumeSFX, &m_VolumeSFXText );
	
	label = g_GUI.AddText( "", GUI::TextDefinition( "Toggle Music: ", 0, y, x, height ), m_AudioOptionsWindowname );
	label->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_RIGHT );
	m_MusicToggle= g_GUI.AddButton( "", GUI::Rectangle( x, y, height, height ), m_AudioOptionsWindowname );
	m_MusicToggle->SetBackgroundImage( "Button_32.png" );
	m_MusicToggle->SetImagePath( "CheckBox.png" );
	m_MusicToggle->SetIsCheckBox( true );
	y += 32;
	
	label = g_GUI.AddText( "", GUI::TextDefinition( "Toggle SFX: ", 0, y, x, height ), m_AudioOptionsWindowname );
	label->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_RIGHT );
	m_SFXToggle = g_GUI.AddButton( "", GUI::Rectangle( x, y, height, height ), m_AudioOptionsWindowname );
	m_SFXToggle->SetBackgroundImage( "Button_32.png" );
	m_SFXToggle->SetImagePath( "CheckBox.png" );
	m_SFXToggle->SetIsCheckBox( true );
	
	
	g_SSAudio.ReadConfig( "audio.cfg" );
	CallbackConfig* audioConfig = g_ConfigManager.GetConfig( "audio.cfg" );
	m_MusicToggle->SetToggled( audioConfig->GetBool( "useMusic" ) );
	m_SFXToggle->SetToggled( audioConfig->GetBool( "useSFX" ) );
	m_VolumeMaster->SetValue( audioConfig->GetFloat( "volumeMaster" ) );
	m_VolumeMusic->SetValue( audioConfig->GetFloat( "volumeMusic" ) );
	m_VolumeSFX->SetValue( audioConfig->GetFloat( "volumeSFX" ) );
}

void SSOptionsMenu::StartupGraphicalWindow()
{
	int y = 82 + 32;
	
	CallbackConfig* gfxConfig = g_ConfigManager.GetConfig( "graphics.cfg" );

	auto addToggleButton = [&]( const rString& text, bool value ) -> GUI::Button*
	{
		GUI::Text* label;
		label = g_GUI.AddText( "", GUI::TextDefinition( text.c_str(), 0, y, 400, 32 ), m_GraphicalOptionsWindowName );
		GUI::Button* button = g_GUI.AddButton( "", GUI::Rectangle( 400, y, 32, 32 ), m_GraphicalOptionsWindowName );
		button->SetBackgroundImage( "Button_32.png" );
		button->SetImagePath( "CheckBox.png" );
		button->SetIsCheckBox( true );
		label->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_RIGHT );
		button->SetToggled( value );
		y += 32;
		return button;
	};

	m_FullscreenToggle = addToggleButton( "Fullscreen: ", g_GameData.m_Fullscreen );
	m_MouseLockToggle = addToggleButton( "Lock mouse: ", gfxConfig->GetBool( "lockmouse", true ) );
	m_VsyncToggle = addToggleButton( "Vertical Sync: ", gfxConfig->GetBool( "vsync", true ) );


	//These have to be added in reverse order because of how comboboxes are rendered
	GUI::Text* label = g_GUI.AddText( "", GUI::TextDefinition( "Texture quality: ", 0, 82, 400, 32 ), m_GraphicalOptionsWindowName );
	label->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_RIGHT );
	m_TextureQuality = g_GUI.AddComboBox( "", GUI::Rectangle( 400, 82, 256, 32 ), m_GraphicalOptionsWindowName );
	m_TextureQuality->SetBackgroundImage( "Button_128x32.png" );
	m_TextureQuality->SetItemBackgroundImage( "Button_128x32.png" );
	m_TextureQuality->AddItem( "Low" );
	m_TextureQuality->AddItem( "Medium" );
	m_TextureQuality->AddItem( "High" );
	int quality = gfxConfig->GetInt("TextureQuality", 1, "Which quality the textures should have low medium high");
	m_TextureQuality->SetSelectedIndex( quality );
	
	label = g_GUI.AddText( "", GUI::TextDefinition( "Fullscreen resolution: ", 0, 50, 400, 32 ), m_GraphicalOptionsWindowName );
	label->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_RIGHT );
	m_ScreenResolutions = g_GUI.AddComboBox( "ScreenRes", GUI::Rectangle( 400, 50, 256, 32 ), m_GraphicalOptionsWindowName );
	m_ScreenResolutions->SetBackgroundImage( "Button_128x32.png" );
	m_ScreenResolutions->SetItemBackgroundImage( "Button_128x32.png" );


	//Time to populate the screen resolution comboBox
	SDL_DisplayMode currentMode;
	SDL_GetWindowDisplayMode( gfx::g_GFXEngine.GetWindow(), &currentMode );
	m_Resolutions.clear();
	int count = 0;
	for( int i = 0; i < SDL_GetNumDisplayModes( 0 ); i++ )
	{
		SDL_DisplayMode mode;
		SDL_GetDisplayMode( 0, i, &mode );
		
		//Since SDL will give us all resolutions available for each refresh rate, we'll need to filter out some duplicate resolutions
		if( std::find( m_Resolutions.begin(), m_Resolutions.end(), glm::ivec2( mode.w, mode.h ) ) == m_Resolutions.end() )
		{
			if( mode.w < 1280 || mode.h < 720 )
				continue; //Don't want people to select too small resolution

			m_Resolutions.push_back( glm::ivec2( mode.w, mode.h ) );
			if( mode.w == currentMode.w && mode.h == currentMode.h )
				m_ScreenResolutions->SetSelectedIndex( count );
			
			rString res = rToString( mode.w ) + "x" + rToString( mode.h );
			
			m_ScreenResolutions->AddItem( res );
			
			count++;
		}
	}
}

void SSOptionsMenu::StartupControlsWindow( )
{
	m_KeyBindingCollection = g_KeyBindings.GetKeyBindCollection();
	g_Script.Register( m_ScriptNameResetToDefault.c_str( ), [this](IScriptEngine*)
		{
			m_WantsToReinitializeKeybindingButtons = true; return 0; 
		} );
	CreateKeyBindingButtons( );
	glm::ivec2 windowSize = g_GUI.GetWindowSize( "RootWindow" );
	glm::ivec2 windowSizeControls = g_GUI.GetWindowSize( m_ControlOptionsWindowName );

	g_Script.Register( m_ScriptNameBindKey.c_str(), [this]( IScriptEngine* scriptEngine ) -> int
	{
		bool primary = scriptEngine->PopBool( );
		ACTION action = static_cast<ACTION>(scriptEngine->PopInt( ));
		m_WantsInputKey = true;
		m_ActionToChange = action;
		m_IsInputtingPrimary = primary;
		m_WindowIsInputting->Open();
		m_WindowIsInputting->BringToFront();
		return 0;
	} );

	int middleX = windowSizeControls.x / 2;
	g_GUI.AddText( "", GUI::TextDefinition( "Primary", 
			middleX - m_KeybindingsButtonHorizontalSpacing / 2 - m_KeybindingsButtonWidth, 0, m_KeybindingsButtonWidth, m_KeybindingsButtonHeight ),
		m_ControlOptionsWindowName )->SetTextAlignment( GUI::ALIGNMENT::ALIGNMENT_MIDDLE_CENTER );

	g_GUI.AddText( "", GUI::TextDefinition( "Secondary", 
			middleX + m_KeybindingsButtonHorizontalSpacing / 2, 0, m_KeybindingsButtonWidth, m_KeybindingsButtonHeight ),
		m_ControlOptionsWindowName )->SetTextAlignment( GUI::ALIGNMENT::ALIGNMENT_MIDDLE_CENTER );

	GUI::Button* buttonResetToDefault = g_GUI.AddButton( "", GUI::Rectangle( windowSizeControls.x / 2 - m_WidthKeybindingsButtonResetToDefault / 2,
				windowSizeControls.y - m_HeightKeybindingsButtonResetToDefault, m_WidthKeybindingsButtonResetToDefault, m_HeightKeybindingsButtonResetToDefault ),
		   m_ControlOptionsWindowName );
	buttonResetToDefault->SetText( "Defaults" );
	buttonResetToDefault->SetClickScript( m_ScriptNameResetToDefault + "()" );
	buttonResetToDefault->SetBackgroundImage( "keybindButton.png" );

	g_GUI.UseFont( FONT_ID::FONT_ID_LEKTON_20 );
	m_WindowIsInputting = g_GUI.AddWindow( m_WindowNameIsInputting, GUI::Rectangle(
				0, windowSize.y / 2 - m_WindowIsInputtingHeight, 
				windowSize.x, m_WindowIsInputtingHeight ), m_OptionsWindowName, true );
	m_WindowIsInputting->GetBackgroundRef().Colour.a = 0.95f;
	g_GUI.AddText( "", GUI::TextDefinition( rString("Please press a key. " + m_KeyBindingCollection.GetScancodeNameForAction( g_SSKeyBinding.GetActionIdentifier( ACTION_GUI_ABORT ) ) + " to abort.").c_str(), 0, 0 ), m_WindowNameIsInputting )->SetTextAlignment( GUI::ALIGNMENT_MIDDLE_CENTER );
	g_GUI.UseFont( FONT_ID::FONT_ID_LEKTON_11 );
	m_ErrorText = g_GUI.AddText( "", GUI::TextDefinition( "", 0, -35 ), m_WindowNameIsInputting );
	m_ErrorText->SetTextAlignment( GUI::ALIGNMENT_BOTTOM_CENTER );
}

void SSOptionsMenu::CreateKeyBindingButtons( )
{
	m_WantsToReinitializeKeybindingButtons = false;
	g_GUI.UseFont( FONT_ID::FONT_ID_CONSOLE_11 );
	glm::ivec2 windowSizeControls = g_GUI.GetWindowSize( m_ControlOptionsWindowName );
	int middleX = windowSizeControls.x / 2;
	int yOffset = 20;

	if ( m_WindowKeybindings )
	{
		m_PrimaryButtons.clear( );
		m_SecondaryButtons.clear( );
		g_GUI.DeleteObject( m_NameWindowKeybindings );
	}

	const rVector<SDL_Scancode>* 	primaryBindings 	= &m_KeyBindingCollection.GetPrimaryBindings();
	const rVector<SDL_Scancode>* 	secondaryBindings 	= &m_KeyBindingCollection.GetSecondaryBindings();
	const rVector<rString>* 		actionDescriptions 	= &g_KeyBindings.GetActionDescriptions();

	m_WindowKeybindings = g_GUI.AddWindow( m_NameWindowKeybindings, GUI::Rectangle( 0, m_PaddingTopKeybindings, windowSizeControls.x,
				windowSizeControls.y - m_PaddingTopKeybindings - m_PaddingBottomKeyBindings),
			m_ControlOptionsWindowName, true );
	m_WindowKeybindings->SetScrollable( true );
	m_WindowKeybindings->SetCulling( true );
	m_WindowKeybindings->ToggleOpen( );

	for ( auto& action : g_KeyBindings.GetActionTitleToAction() )
	{
		g_GUI.AddText( "", GUI::TextDefinition( action.first.c_str(),
			middleX - m_KeybindingsButtonWidth - static_cast<int>(m_KeybindingsButtonHorizontalSpacing * 1.5f), yOffset, 0, m_KeybindingsButtonHeight ),
			m_NameWindowKeybindings )->SetTextAlignment( GUI::ALIGNMENT::ALIGNMENT_MIDDLE_RIGHT );

		SDL_Scancode primary = primaryBindings->at( static_cast<int>( action.second.Action ) );
		SDL_Scancode secondary = secondaryBindings->at( static_cast<int>( action.second.Action ) );

		GUI::Button* primaryButton = g_GUI.AddButton( "",
			GUI::Rectangle(middleX - m_KeybindingsButtonWidth - m_KeybindingsButtonHorizontalSpacing / 2,
				yOffset, m_KeybindingsButtonWidth, m_KeybindingsButtonHeight ),
			m_NameWindowKeybindings );
		primaryButton->SetText( SDL_GetScancodeName( primary ) );
		primaryButton->SetClickScript( m_ScriptNameBindKey + "(" + rToString( g_SSKeyBinding.GetAction( action.second.Action ) ) + ", true)" );
		primaryButton->SetTextAlignment( GUI::ALIGNMENT::ALIGNMENT_MIDDLE_CENTER );
		primaryButton->SetBackgroundImage( "keybindButton.png" );
		primaryButton->GetBackgroundRef().BorderSize = 0;
		
		m_PrimaryButtons.emplace( g_SSKeyBinding.GetAction( action.second.Action ), primaryButton );

		GUI::Button* secondaryButton = g_GUI.AddButton( "",
			GUI::Rectangle(middleX + m_KeybindingsButtonHorizontalSpacing / 2,
				yOffset, m_KeybindingsButtonWidth, m_KeybindingsButtonHeight),
			m_NameWindowKeybindings );
		
		secondaryButton->SetText( SDL_GetScancodeName( secondary ) );
		secondaryButton->SetClickScript( m_ScriptNameBindKey + "(" + rToString( g_SSKeyBinding.GetAction( action.second.Action ) ) + ", false)" );
		m_SecondaryButtons.emplace( g_SSKeyBinding.GetAction( action.second.Action ), secondaryButton );
		secondaryButton->SetTextAlignment( GUI::ALIGNMENT::ALIGNMENT_MIDDLE_CENTER );
		secondaryButton->SetBackgroundImage( "keybindButton.png" );
		secondaryButton->GetBackgroundRef().BorderSize = 0;

		g_GUI.AddText( "", GUI::TextDefinition( 
			actionDescriptions->at( static_cast<int>( action.second.Action ) ).c_str( ),
			middleX + static_cast<int>(m_KeybindingsButtonHorizontalSpacing * 1.5f) + m_KeybindingsButtonWidth, yOffset, 0, m_KeybindingsButtonHeight ), 
					   m_NameWindowKeybindings )->GetTextDefinitionRef().Alignment = GUI::ALIGNMENT::ALIGNMENT_MIDDLE_LEFT;
		yOffset += m_KeybindingsButtonHeight + m_KeybindingsButtonVerticalSpacing;
	}
}

void SSOptionsMenu::ResetIsInputting( )
{
	m_WindowIsInputting->Close();
	m_WantsInputKey = false;
	m_ErrorText->SetText( "" );
}

void SSOptionsMenu::ApplyOptions()
{
	//Save Graphics config
	glm::ivec2 size = m_Resolutions[m_ScreenResolutions->GetSelectedIndex()];
	
	CallbackConfig* networkConfig = g_ConfigManager.GetConfig( "network.cfg" );
	networkConfig->SetString( "PlayerName", m_PlayerName->GetText() );
	networkConfig->SaveFile( "network.cfg" );
	
	CallbackConfig* gfxConfig = g_ConfigManager.GetConfig( "graphics.cfg" );
	gfxConfig->SetInt( "width", size.x );
	gfxConfig->SetInt( "height", size.y );
	gfxConfig->SetBool( "fullscreen", m_FullscreenToggle->GetToggled() );
	gfxConfig->SetBool( "vsync", m_VsyncToggle->GetToggled() );
	gfxConfig->SetBool( "lockmouse", m_MouseLockToggle->GetToggled() );
	gfxConfig->SetInt( "TextureQuality", m_TextureQuality->GetSelectedIndex() );
	gfxConfig->SaveFile( "graphics.cfg" );
	
	CallbackConfig* audioConfig = g_ConfigManager.GetConfig( "audio.cfg" );
	audioConfig->SetBool( "useMusic", m_MusicToggle->GetToggled() );
	audioConfig->SetBool( "useSFX", m_SFXToggle->GetToggled() );
	audioConfig->SetFloat( "volumeMaster", m_VolumeMaster->GetValue() );
	audioConfig->SetFloat( "volumeMusic", m_VolumeMusic->GetValue() );
	audioConfig->SetFloat( "volumeSFX", m_VolumeSFX->GetValue() );
	audioConfig->SaveFile( "audio.cfg" );
	
	//Save other configs here
	

	g_KeyBindings.SetKeyBindingCollection( m_KeyBindingCollection );
	g_KeyBindings.SaveConfig( "keybindings.cfg" );

	g_GameModeSelector.SwitchToGameMode( GameModeType::MainMenu );

	g_GUI.CloseWindow( m_OptionsWindowName );
	g_GUI.OpenWindow( "MainMenu");
}
