/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "TextBox.h"
#include "../GUIEngine.h"
#include <input/TextInput.h>

namespace GUI
{
	
	TextBox::TextBox() : TextBox( "", "", Rectangle() )
	{

	}
	
	TextBox::TextBox( const rString& name, const rString& parent, Rectangle boundingBox ) : 
		Object( name, parent, boundingBox )
	{
		m_TextDefinition = TextDefinition( "", boundingBox.X + 5, boundingBox.Y , boundingBox.Width, boundingBox.Height );
		m_TextDefinition.Alignment = ALIGNMENT_MIDDLE_LEFT;
		
		m_BackgroundColour = glm::vec4( 0.0f, 0.0f, 0.0f, 0.7f );
		m_BorderColour = glm::vec4( 0.2f, 0.2f, 0.2f, 0.7f );

		m_Background = SpriteDefinition( "", boundingBox.X, boundingBox.Y, boundingBox.Width, boundingBox.Height, m_BackgroundColour );
		m_Background.BorderSize = 1;
		m_Background.BorderColour = m_BorderColour;

		m_Focused = false;
		
		m_InputStringID = g_TextInput.ReserveAndGetInputID();
	}
	
	TextBox::~TextBox()
	{
	}
	
	void TextBox::Initialize()
	{
	}
	
	void TextBox::Update(float deltaTime , glm::ivec2 parentPos)
	{
		if( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT ))
		{
			if( m_BoundingBox.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
			{
			
				if( m_Enabled )
				{
					m_Focused = true;
					g_TextInput.StartInput( m_InputStringID );
					g_Input->ConsumeMouseButtons();
				}
			}
			else
			{
				m_Focused = false;
				g_TextInput.StopInput();
			}
		}
		
		if( m_Focused && g_TextInput.IsInputting( m_InputStringID ) )
		{
			unsigned int	cursor	= g_TextInput.GetTextCursor( m_InputStringID );
			rString			str		= g_TextInput.GetString( m_InputStringID );

			if( m_MarkerFlashTime > 0.5f )
				m_TextDefinition.Text = str.substr( 0, cursor ) + "_" + str.substr( cursor );
			else if( m_MarkerFlashTime > 0 && m_MarkerFlashTime <= 0.5f )
				m_TextDefinition.Text = str.substr( 0, cursor ) + " " + str.substr( cursor );
			else
				m_MarkerFlashTime = 1.0f;
			
			m_MarkerFlashTime -= deltaTime;
			m_TextDefinition.Colour = glm::vec4( 1.0f );
			
			m_Text = g_TextInput.GetString( m_InputStringID );
		}
		else
			m_TextDefinition.Text = g_TextInput.GetString( m_InputStringID ) + " ";
	}
	
	void TextBox::Render( glm::ivec2 parentPos )
	{
		m_Background.Colour = m_BackgroundColour * m_Opacity;
		m_Background.BorderColour = m_BorderColour * m_Opacity;
		m_TextDefinition.Opacity = m_Opacity;

		m_Background.Origin = parentPos;
		m_TextDefinition.Origin = parentPos;
		
		g_GUI.EnqueueSprite( &m_Background );
			
		g_GUI.EnqueueText( &m_TextDefinition );
	}
	
	void TextBox::OnWindowOpen()
	{
	}
	
	void TextBox::OnWindowClose()
	{
		g_TextInput.StopInput();
		m_Focused = false;
	}
	
	void TextBox::StartInput()
	{
		g_TextInput.StartInput( m_InputStringID );
		m_Focused = true;
	}
	
	void TextBox::StopInput()
	{
		g_TextInput.StopInput();
		m_Focused = false;
	}

	SpriteDefinition& TextBox::GetBackgroundRef()
	{
		return m_Background;
	}

	TextDefinition& TextBox::GetTextDefinitionRef()
	{
		return m_TextDefinition;
	}
	
	rString& TextBox::GetText()
	{
		return m_Text;
	}
	
	void TextBox::SetText( const rString& text )
	{
		g_TextInput.SetString( m_InputStringID, text );
		m_Text = text;
		m_TextDefinition.Text = text;
	}
	
	bool TextBox::IsInputting()
	{
		return g_TextInput.IsInputting( m_InputStringID );
	}

	void TextBox::SetSize( int width, int height )
	{
		m_BoundingBox.Width = width;
		m_BoundingBox.Height = height;
		m_TextDefinition.BoundsSize = glm::ivec2( width, height );
	}
}
