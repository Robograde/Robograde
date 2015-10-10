#include "SSEditorEvents.h"
#include <gui/GUIEngine.h>
#include <input/Input.h>
#include "SSColourPicker.h"

SSEditorEvents& SSEditorEvents::GetInstance()
{
	static SSEditorEvents instance;
	return instance;
}

void SSEditorEvents::Startup()
{
	int x = 16;
	int y = 16;

	//Static GUI elements
	g_GUI.UseFont( FONT_ID_LEKTON_11 );
	GUI::Text* text;
	text = g_GUI.AddText( "", GUI::TextDefinition( "I want the event to be triggered by a", x, y ), m_WindowName );
	x += text->GetTextSize().x + 8;

	m_EventTypeComboBox = g_GUI.AddComboBox( "", GUI::Rectangle( x, y - 8, 128, 32 ), m_WindowName );
	m_EventTypeComboBox->SetBackgroundImage( "Button_128x32.png" );
	m_EventTypeComboBox->SetItemBackgroundImage( "Button_128x32.png" );
	m_EventTypeComboBox->AddItem( "Collision" );
	m_EventTypeComboBox->AddItem( "Timer" );
	m_EventTypeComboBox->SetDirectionUp();

	x += 128 + 8;

	text = g_GUI.AddText( "", GUI::TextDefinition( "and should trigger a", x, y ), m_WindowName );
	x += text->GetTextSize().x + 8;

	m_EventEffectComboBox = g_GUI.AddComboBox( "", GUI::Rectangle( x, y - 8, 128 + 32, 32 ), m_WindowName );
	m_EventEffectComboBox->SetBackgroundImage( "Button_128x32.png" );
	m_EventEffectComboBox->SetItemBackgroundImage( "Button_128x32.png" );
	m_EventEffectComboBox->AddItem( "Camera path" );
	m_EventEffectComboBox->AddItem( "Sound" );
	m_EventEffectComboBox->AddItem( "Particle effect" );
	m_EventEffectComboBox->AddItem( "Script" );
	m_EventEffectComboBox->SetDirectionUp();

	//Dynamic elements
	
	x = m_EventTypeComboBox->GetBoundingBoxRef().GetLeft();
	y += 32 + 16;
	m_TimeInputWindow = g_GUI.AddWindow( "TimeInputWindow", GUI::Rectangle( x, y, 0, 0 ), m_WindowName );
	x = y = 0;
	text = g_GUI.AddText( "", GUI::TextDefinition( "Trigger after", x, y ), "TimeInputWindow" );
	
	int tWidth = text->GetTextSize().x;
	x += tWidth + 8;

	m_TimeInput = g_GUI.AddTextBox( "", GUI::Rectangle( x, y - 8, 64, 32 ), "TimeInputWindow" );
	m_TimeInput->SetText( "10.0" );
	x += 64 + 8;
	g_GUI.AddText( "", GUI::TextDefinition( "seconds.", x, y ), "TimeInputWindow" );



	////Test
	//m_ColourResult = g_GUI.AddButton( "ColourResultBtn", GUI::Rectangle( 0, 64, 64, 64 ), m_WindowName );
	//m_ColourResult->GetBackgroundRef().BorderSize = 1;
	//m_ColourResult->SetClickScript( "GE_OpenColourPicker( 'ColourResultBtn', '" + m_WindowName + "')" );


}

void SSEditorEvents::UpdateUserLayer( const float deltaTime )
{
	if( g_GUI.IsWindowOpen( m_WindowName ) )
	{
		if( m_EventTypeComboBox->GetSelectedIndex() == 0 )
		{
		}

		if ( m_EventTypeComboBox->GetSelectedIndex() == 1 )
		{
			m_TimeInputWindow->Open();
		}
		else
		{
			m_TimeInputWindow->Close();
		}
	}
}

void SSEditorEvents::Shutdown()
{

}