/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "Object.h"
#include <input/Input.h>
#include <gui/GUIEngine.h>

namespace GUI
{
	Object::Object( )
	{
	}

	Object::Object( const rString& name, const rString& parent, Rectangle boundingbox )
	{
		m_Name = name.c_str();
		m_Parent = parent.c_str();
		m_BoundingBox = boundingbox;
		m_ToolTipDelay = 0.5f;
		m_ToolTipTimer = m_ToolTipDelay;
	}
	
	Object::~Object()
	{
	}
	
	void Object::OnWindowOpen()
	{
	}
	
	void Object::OnWindowClose()
	{
	}
	
	void Object::OnFocus()
	{
	}
	
	bool Object::GetBringToFront()
	{
		bool bringToFront = m_BringToFront;
		m_BringToFront = false;
		return bringToFront;
	}
	
	void Object::BringToFront()
	{
		m_BringToFront = true;
	}
	
	void Object::Initialize()
	{
	}
	
	void Object::Update(float deltaTime , glm::ivec2 parentPos)
	{
		
		if( m_ToolTipText != "" )
		{
			glm::ivec2 mousePos = glm::vec2( g_Input->GetMousePosX(), g_Input->GetMousePosY() );
			
			if( m_BoundingBox.Intersects( mousePos.x, mousePos.y ) )
			{
				if( m_ToolTipTimer <= 0.0f )
					g_GUI.SetToolTipText( m_ToolTipText );
				else
					m_ToolTipTimer -= deltaTime;
			}
			else
				m_ToolTipTimer = m_ToolTipDelay;
		}

	}
	
	void Object::Render( glm::ivec2 parentPos )
	{
	}

	void Object::SetOrigin( int x, int y )
	{
		m_BoundingBox.Origin = glm::ivec2( x, y );
	}
	
	void Object::SetPosition( int x, int y )
	{
		m_BoundingBox.X = x;
		m_BoundingBox.Y = y;
	}

	void Object::SetSize( int width, int height )
	{
		m_BoundingBox.Width = width;
		m_BoundingBox.Height = height;
	}

	glm::ivec2 Object::GetPosition( )
	{
		return m_BoundingBox.GetPosition();
	}

	glm::ivec2 Object::GetSize( )
	{
		return m_BoundingBox.GetSize();
	}

	void Object::SetToolTipText( const rString& text )
	{
		m_ToolTipText = text;
	}

	void Object::SetToolTipDelay( float delay )
	{
		m_ToolTipDelay = delay;
	}


}
