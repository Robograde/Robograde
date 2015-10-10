/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "ProgressBar.h"
#include "../GUIEngine.h"

GUI::ProgressBar::ProgressBar()
{
}

GUI::ProgressBar::ProgressBar( rString name, rString parent , Rectangle boundingBox ) :
	Object( name, parent, boundingBox )
{
	m_Background = SpriteDefinition( "", boundingBox.X, boundingBox.Y, boundingBox.Width, boundingBox.Height, glm::vec4( 0.1f, 0.1f, 0.1f, 1.0f ) );
	m_Background.BorderSize = 1;
	
	m_Bar = SpriteDefinition( "", boundingBox.X + 1, boundingBox.Y + 1, boundingBox.Width - 2, boundingBox.Height - 2, glm::vec4( 0.1, 0.1, 0.7f, 1.0f ) );
	
	m_BarWidth = boundingBox.Width - 2;
	
	
}

GUI::ProgressBar::~ProgressBar()
{
}

void GUI::ProgressBar::Initialize()
{
}

void GUI::ProgressBar::Update( float deltaTime , glm::ivec2 parentPos )
{
}

void GUI::ProgressBar::Render( glm::ivec2 parentPos )
{
	m_Bar.Width = static_cast<int>( m_BarWidth * m_Value / (float)m_MaxValue );
	if( m_Visible )
	{
		m_BoundingBox.Origin = parentPos;
		m_Background.Origin = parentPos;
		m_Bar.Origin = parentPos;
		
		m_Background.Position = m_BoundingBox.GetPosition();
		m_Bar.Position = m_BoundingBox.GetPosition() + glm::ivec2( 1, 1 );
		
		g_GUI.EnqueueSprite( &m_Background );
		g_GUI.EnqueueSprite( &m_Bar );
	}
}

void GUI::ProgressBar::SetBackgroundTexture( rString texturePath )
{
	m_Background.Texture = texturePath;
}

void GUI::ProgressBar::SetBarTexture( rString texturePath )
{
	m_Bar.Texture = texturePath;
}

void GUI::ProgressBar::SetBackgroundColour( glm::vec4 colour )
{
	m_Background.Colour = colour;
}

void GUI::ProgressBar::SetBarColour( glm::vec4 colour )
{
	m_Bar.Colour = colour;
}

void GUI::ProgressBar::SetBorderColour( glm::vec4 colour )
{
	m_Background.BorderColour = colour;
}

void GUI::ProgressBar::SetMinValue( float value )
{
	m_MinValue = value;
}

void GUI::ProgressBar::SetMaxValue( float value )
{
	m_MaxValue = value;
}

void GUI::ProgressBar::SetValue( float value )
{
	m_Value = value;
}

void GUI::ProgressBar::SetPosition( int x, int y )
{
	m_BoundingBox.X = x;
	m_BoundingBox.Y = y;
}
