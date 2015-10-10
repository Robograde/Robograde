/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "Text.h"
#include "../GUIEngine.h"

namespace GUI
{
	Text::Text() 
	{

	}

	Text::Text( const rString& name, TextDefinition textDefinition, const rString& parent, Rectangle boundingBox ) :
		Object( name, parent, boundingBox )
	{
		m_TextDefinition = textDefinition;
		m_TextDefinition.BoundsSize = glm::ivec2( boundingBox.Width, boundingBox.Height );
		
		m_TextDefinition.Origin = g_GUI.GetWindowPos( parent.c_str() );
	}

	Text::~Text() 
	{

	}

	void Text::Initialize() 
	{

	}

	void Text::Update(float deltaTime , glm::ivec2 parentPos) 
	{

	}

	void Text::Render( glm::ivec2 parentPos ) 
	{
		m_TextDefinition.Opacity = m_Opacity;
		m_BoundingBox.X = m_TextDefinition.Position.x;
		m_BoundingBox.Y = m_TextDefinition.Position.y;
		
		m_TextDefinition.Origin = parentPos;

		
		//g_GUI.EnqueueText( &m_TextBackground );
		g_GUI.EnqueueText( &m_TextDefinition );
		
	}

	TextDefinition& Text::GetTextDefinitionRef()
	{
		return m_TextDefinition;
	}

	void Text::SetTextDefinition( TextDefinition textDefinition )
	{
		m_TextDefinition = textDefinition;
	}

	void Text::SetText( const rString& text )
	{
		m_TextDefinition.Text = text;
	}

	void Text::SetTextAlignment( ALIGNMENT textAlign )
	{
		m_TextDefinition.Alignment = textAlign;
	}

	void Text::SetPosition( int x, int y )
	{
		m_TextDefinition.Position = glm::ivec2( x, y );
	}

		void Text::SetPosition( const glm::ivec2& position )
	{
		m_TextDefinition.Position = position;
	}
}
