#include "SSColourPicker.h"
#include <gui/GUIEngine.h>
#include <input/Input.h>
#include <utility/ColourConversion.h>
#include "../input/SSKeyBinding.h"

SSColourPicker& SSColourPicker::GetInstance()
{
	static SSColourPicker instance;
	return instance;
}

void SSColourPicker::Startup()
{
	int x = 16;
	int y = 16;

	g_GUI.UseFont( FONT_ID_LEKTON_16 );

	m_Window = g_GUI.AddWindow( m_WindowName, GUI::Rectangle( 0, 0, 8 + 256 + 8 + 32 + 8, 256 + 16 + 32 ), "RootWindow", true );
	m_Window->SetBackgroundColour( glm::vec4( 0.2f, 0.2f, 0.2f, 1.0f ) );

	m_HVPicker = g_GUI.AddSprite( "", GUI::SpriteDefinition( "HV_Colour.png", 8, 8, 256, 256 ), m_WindowName );
	m_SPicker = g_GUI.AddSprite( "", GUI::SpriteDefinition( "S_Colour.png", 8 + 256 + 8, 8, 32, 256 ), m_WindowName );
	m_HVMarker = g_GUI.AddSprite( "", GUI::SpriteDefinition( "HV_Marker.png", 0, 0, 16, 16 ), m_WindowName );
	m_SMarker = g_GUI.AddSprite( "", GUI::SpriteDefinition( "S_Marker.png", 0, 0, 32, 16 ), m_WindowName );

	m_HVPicker->GetSpriteDefinitionRef().BorderSize = 1;
	m_SPicker->GetSpriteDefinitionRef().BorderSize = 1;

	m_RGB = g_GUI.AddText( "", GUI::TextDefinition( "", 8, 256 + 16 ), m_WindowName );

	m_H = 0;
	m_S = 1.0f;
	m_V = 0.0f;

	g_Script.Register( "GE_OpenColourPicker", [this]( IScriptEngine* scriptEngine )
	{
		rString& parent = scriptEngine->PopString();
		rString& buttonName = scriptEngine->PopString();
		GUI::Button* btn = static_cast<GUI::Button*>( g_GUI.GetObject( buttonName, parent ) );
		
		Open( glm::ivec2( 256, 256 ), btn );

		m_Colour = btn->GetColour();


		glm::vec3 hsv = RGBtoHSV( glm::vec3( m_Colour.r, m_Colour.g, m_Colour.b ) );
		m_H = (int)hsv.x;
		m_S = hsv.y;
		m_V = hsv.z;

		m_HVMarker->SetPosition( m_HVPicker->GetPosition().x +static_cast<int>( m_HVSize * (hsv.x / 360) ) - (m_HVMarker->GetSize().x / 2), 
			m_HVPicker->GetPosition().y + static_cast<int>(m_HVSize * (1.0f - hsv.z)) - (m_HVMarker->GetSize().y / 2) );

		m_SMarker->SetPosition( m_SPicker->GetPosition().x, 
			m_SPicker->GetPosition().y + static_cast<int>(1.0f - hsv.y) * m_SPicker->GetSize().y - (m_SMarker->GetSize().y / 2) );

		return 0;
	} );
}

void SSColourPicker::UpdateUserLayer( const float deltaTime )
{
	if( m_Window->IsOpen() )
	{
		m_Colour = glm::vec4( HSVtoRGB( glm::vec3( m_H, m_S, m_V ) ), 1.0f );
		glm::ivec3 rgb255 = glm::ivec3( m_Colour.r * 255, m_Colour.g * 255, m_Colour.b * 255 );

		if( m_ColourResult )
			m_ColourResult->SetColour( m_Colour );

		m_RGB->SetText( "R: " + rToString( rgb255.r ) + " G: " + rToString( rgb255.g ) + " B: " + rToString( rgb255.b ) );

		if( !m_Window->GetBoundingBoxRef().Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
		{
			if( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT )  )
			{
				m_Window->Close();
			}
		}
		if( g_SSKeyBinding.ActionUpDown( ACTION::ACTION_GUI_ABORT ) )
		{
			m_Window->Close();
		}

		if( m_HVPicker->GetBoundingBoxRef().Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
		{		

			glm::ivec2 localPos = glm::ivec2( g_Input->GetMousePosX() - m_HVPicker->GetBoundingBoxRef().GetLeft(),
													g_Input->GetMousePosY() - m_HVPicker->GetBoundingBoxRef().GetTop() );

			if( g_Input->MouseDown( MOUSE_BUTTON_LEFT ) )
			{
				m_HVMarker->SetPosition( m_HVPicker->GetPosition().x + localPos.x - (m_HVMarker->GetSize().x / 2),  
					m_HVPicker->GetPosition().y + localPos.y  - (m_HVMarker->GetSize().y / 2) );

				m_H = static_cast<int>( ( localPos.x / (float)m_HVSize ) * 360 );
				m_V = 1.0f - ( localPos.y / (float)m_HVSize );

			}

		}

		if( m_SPicker->GetBoundingBoxRef().Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
		{		
			if( g_Input->MouseDown( MOUSE_BUTTON_LEFT ) )
			{
				glm::ivec2 localPos = glm::ivec2( g_Input->GetMousePosX() - m_SPicker->GetBoundingBoxRef().GetLeft(),
														g_Input->GetMousePosY() - m_SPicker->GetBoundingBoxRef().GetTop() );

				m_S = 1.0f - ( localPos.y / (float)m_SPicker->GetSize().y );

				m_SMarker->SetPosition( m_SPicker->GetPosition().x, 
					m_SPicker->GetPosition().y + static_cast<int>( ( 1.0f - m_S ) * m_SPicker->GetSize().y ) - (m_SMarker->GetSize().y / 2) );
			}
		}
		
	}
}

void SSColourPicker::Shutdown()
{

}

glm::vec4& SSColourPicker::GetColour()
{
	return m_Colour;
}

void SSColourPicker::Open( glm::ivec2 position, GUI::Button* colourResultButton )
{
	m_Window->Open();

	glm::ivec2 rootSize = g_GUI.GetWindowSize( "RootWindow" );
	glm::ivec2 bottomBarSize = g_GUI.GetWindowSize( "BottomBarWindow" );
	int y = rootSize.y - bottomBarSize.y - m_Window->GetSize().y;
	int x = 0;
	m_Window->SlideOpen( glm::ivec2( x - m_Window->GetSize().x, y ), glm::ivec2( x, y ), 512 );

	m_ColourResult = colourResultButton;
}