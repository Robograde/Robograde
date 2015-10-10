#include "Catalogue.h"
#include <gui/GUIEngine.h>
#include <script/IScriptEngine.h>

#define DEFAULT_BUTTON_OFFSET_X 4
#define DEFAULT_BUTTON_OFFSET_Y 4
#define DEFAULT_BUTTON_WIDTH	64
#define DEFAULT_BUTTON_HEIGHT	64

Catalogue::Catalogue( const rString& name, const GUI::Rectangle& boundingBox, const rString& parentWindow, const rString& buttonClickScript, unsigned int buttonOffsetX, unsigned int buttonOffsetY,
	unsigned int buttonWidth, unsigned int buttonHeight, bool useWindowBorder )
{
	m_Window = g_GUI.AddWindow( name + "Window", boundingBox, parentWindow, useWindowBorder );
	m_Window->Open();
	m_Window->BringToFront();

	m_OnButtonClickScript = buttonClickScript;

	m_ButtonOffsetX	= buttonOffsetX;
	m_ButtonOffsetY	= buttonOffsetY;
	m_ButtonWidth	= buttonWidth;
	m_ButtonHeight	= buttonHeight;

	m_ButtonCount = 0;
}

Catalogue::Catalogue( const rString& name, const GUI::Rectangle& boundingBox, const rString& parentWindow, const rString& buttonClickScript, bool useWindowBorder )
{
	m_Window = g_GUI.AddWindow( name + "Window", boundingBox, parentWindow, useWindowBorder );
	m_Window->Open();
	m_Window->BringToFront();

	m_OnButtonClickScript = buttonClickScript;

	m_ButtonOffsetX = DEFAULT_BUTTON_OFFSET_X;
	m_ButtonOffsetY = DEFAULT_BUTTON_OFFSET_Y;
	m_ButtonWidth	= DEFAULT_BUTTON_WIDTH;
	m_ButtonHeight	= DEFAULT_BUTTON_HEIGHT;

	m_ButtonCount = 0;
}

Catalogue::~Catalogue()
{
	g_GUI.DeleteObject( m_Window->GetName() );
}

GUI::Window* Catalogue::GetWindow() const
{
	return m_Window;
}

void Catalogue::AddButton( const rString& textureName, const glm::vec4& colour, const rString& text, const rString& toolTipText )
{
	GUI::Button* button = g_GUI.AddButton( m_Window->GetName() + "Button" + rToString( m_ButtonCount ), GUI::Rectangle( m_ButtonOffsetX + ( (m_ButtonWidth + m_ButtonOffsetX) * m_ButtonCount ), m_ButtonOffsetY, m_ButtonWidth, m_ButtonHeight ), m_Window->GetName() );

	if ( button != nullptr )
	{
		button->SetClickScript( m_OnButtonClickScript + "(" + rToString(m_ButtonCount) + ")" );
		button->SetImagePath( textureName );
		button->SetColour( colour );
		button->SetText( text );
		button->SetToolTipText( toolTipText );
		++m_ButtonCount;
	}
}