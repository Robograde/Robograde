/**************************************************
2015 Daniel "MonzUn" Bengtsson
***************************************************/
#pragma once

#include <memory/rString.h>
#include <gui/graphics2D/Rectangle.h>
#include <gui/object/Window.h>

class Catalogue
{
public:
	Catalogue( const rString& name, const GUI::Rectangle& boundingBox, const rString& parentWindow, const rString& buttonClickScript, unsigned int buttonOffsetX, unsigned int buttonOffsetY, unsigned int buttonWidth, unsigned int buttonHeight, bool useWindowBorder = false );
	Catalogue( const rString& name, const GUI::Rectangle& boundingBox, const rString& parentWindow, const rString& buttonClickScript, bool useWindowBorder = false );
	~Catalogue();

	GUI::Window* GetWindow() const;

	void AddButton( const rString& textureName = "", const glm::vec4& colour = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ), const rString& text = "", const rString& toolTipText = "" );

private:

	void OnButtonClicked( int buttonIndex );

	GUI::Window*	m_Window;
	unsigned int	m_ButtonCount;

	unsigned int	m_ButtonOffsetX;
	unsigned int	m_ButtonOffsetY;
	unsigned int	m_ButtonWidth;
	unsigned int	m_ButtonHeight;

	rString			m_OnButtonClickScript;
};