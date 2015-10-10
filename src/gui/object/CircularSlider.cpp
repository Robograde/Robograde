/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "CircularSlider.h"
#include "../GUIEngine.h"
#include <script/ScriptEngine.h>
#include <input/Input.h>

namespace GUI
{
	CircularSlider::CircularSlider()
	{
	
	}
	
	CircularSlider::CircularSlider( const rString& name, const rString& parent, glm::ivec2 position ) :
		Object( name, parent, Rectangle(0,0,0,0) )
	{
		m_CenterPoint = position;
		
		//m_Background		= SpriteDefinition( "", boundingBox.X, boundingBox.Y, boundingBox.Width, boundingBox.Height );
		
		m_BorderSize = 32;

		m_Colour = glm::vec4( 1.0f, 1.0f, 1.0f, 0.5f );
		m_HighLightColour = glm::vec4( 0.8f, 1.0f, 1.0f, 0.8f );

		m_DisabledColour = glm::vec4( 0.3f, 0.3f, 0.3f, 0.0f );


		m_MinRadius = 0;
		m_MaxRadius = INT_MAX;
	}
	
	CircularSlider::~CircularSlider()
	{
	
	}
	
	void CircularSlider::Initialize()
	{
	
	}
	
	void CircularSlider::Update( float deltaTime , glm::ivec2 parentPos )
	{
		m_Changed = false;
		glm::vec2 mousePos = glm::vec2( g_Input->GetMousePosX(), g_Input->GetMousePosY() );

		float distance = glm::length( mousePos - glm::vec2( m_CenterPoint ) );


		if( distance < m_Radius && distance > m_Radius - m_BorderSize )
		{
			if( m_Enabled )
			{
				if( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT )  )
				{
					m_Dragging = true;
					m_ClickPos = glm::ivec2( g_Input->GetMousePosX() , g_Input->GetMousePosY() );
					g_Input->ConsumeMouseButtons();
				}
					
				if( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT ) )
					g_Input->ConsumeMouseButtons();
			}

			m_Hover = true;

		}
		else
			m_Hover = false;

		int tempRadius = m_Radius;

		if( m_Dragging )
		{
			float clickDistance = glm::length( glm::vec2( m_ClickPos - m_CenterPoint ) );

			tempRadius += static_cast<int>( distance - clickDistance );

			if( tempRadius > m_MaxRadius )
				tempRadius = m_MaxRadius;
			else if( tempRadius < m_MinRadius )
				tempRadius = m_MinRadius;

			if( g_Input->MouseUp( MOUSE_BUTTON_LEFT ) )
			{
				m_Dragging = false;
				m_Changed = true;
				m_Radius = tempRadius;
			}
		}

		m_Background.Texture = "CircularSlider.png";
		m_Background.Width = tempRadius * 2;
		m_Background.Height = tempRadius * 2;
		m_Background.Position = glm::ivec2( m_CenterPoint.x - tempRadius, m_CenterPoint.y - tempRadius );

		//m_Background.Position = m_CenterPoint

		/*m_BoundingBox.Origin = parentPos;
		if( m_BoundingBox.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() )  && g_GUI.IsInputEnabled() )
		{
			m_Hover = true;
			if( m_Enabled )
			{
				if( g_Input->MouseDown( MOUSE_BUTTON_LEFT ) )
				{
					m_Down = true;
				}
				else
					m_Down = false;
				
				if( g_Input->MouseDownUp( MOUSE_BUTTON_LEFT )  )
				{
					g_Input->ConsumeMouseButtons();
				}
					
				if( m_Down || g_Input->MouseUpDown( MOUSE_BUTTON_LEFT ) )
					g_Input->ConsumeMouseButtons();
			}
		}
		else
		{
			m_Down = false;
			m_Hover = false;
		}*/
	}
	
	
	void CircularSlider::Render( glm::ivec2 parentPos )
	{
		//m_Background.SetBounds( m_BoundingBox );
		
		m_Background.Origin = parentPos;
		
		if( !m_Enabled )
		{
			m_Background.Colour = m_Colour - m_DisabledColour;
		}
		else if( m_Dragging )
		{
			m_Background.Colour = m_Colour * ( m_HighLightColour + 0.2f );
		}
		else if( m_Hover )
		{
			m_Background.Colour = m_Colour * ( m_HighLightColour + 0.3f );
		}
		else
		{
			m_Background.Colour = m_Colour;
		}
			
		g_GUI.EnqueueSprite( &m_Background );
		
	}
	
	void CircularSlider::OnValueChange()
	{
	}
	
	SpriteDefinition& CircularSlider::GetBackgroundRef()
	{
		return m_Background;
	}


	glm::vec4& CircularSlider::GetColour()
	{
		if( m_Background.Colour.r > 1.0f )
			m_Background.Colour.r = 1.0f;
		if( m_Background.Colour.g > 1.0f )
			m_Background.Colour.g = 1.0f;
		if( m_Background.Colour.b > 1.0f )
			m_Background.Colour.b = 1;

		return m_Background.Colour;
	}

	bool CircularSlider::GetChanged()
	{
		return m_Changed;
	}
	
	int CircularSlider::GetRadius()
	{
		return m_Radius;
	}
	
	void CircularSlider::SetBackgroundImage( const rString& imageName )
	{
		m_Background.Texture = imageName.c_str();
		m_Background.Colour = glm::vec4( 1.0f );
		m_Colour = m_Background.Colour;
	}
	
	void CircularSlider::SetColour( glm::vec4 colour )
	{
		m_Colour = colour;
	}
	
	void CircularSlider::SetHighLightColour( glm::vec4 colour )
	{
		m_HighLightColour = colour;
	}

	void CircularSlider::SetDisabledColour( glm::vec4 colour )
	{
		m_DisabledColour = colour;
		m_DisabledColour.a = 0.0f; //Just to avoid button being transparent
	}
	
	void CircularSlider::SetRadius( int radius )
	{
		m_Radius = radius;
	}

	void CircularSlider::SetMinRadius( int radius )
	{
		m_MinRadius = radius;
	}

	void CircularSlider::SetMaxRadius( int radius )
	{
		m_MaxRadius = radius;
	}

	void CircularSlider::SetPosition( int x, int y )
	{
		m_CenterPoint = glm::ivec2( x, y );
	}
}
