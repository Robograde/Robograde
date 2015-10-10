/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "Slider.h"
#include <math.h>
#include "../GUIEngine.h"
#include <script/ScriptEngine.h>
#include <input/Input.h>

namespace GUI
{
	Slider::Slider()
	{
	
	}
	
	Slider::Slider( const rString& name, const rString& parent, Rectangle boundingBox ) :
		Object( name, parent, boundingBox )
	{
		m_BoundingBox		= boundingBox;
		
		m_Background		= SpriteDefinition( "", boundingBox.X, boundingBox.Y + (boundingBox.Height / 4), boundingBox.Width, boundingBox.Height / 2, glm::vec4( 0.9f, 0.9f, 1.0f, 1.0f ) );

		m_Slider			= SpriteDefinition( "", boundingBox.X, boundingBox.Y, boundingBox.Height, boundingBox.Height, glm::vec4( 0.8f, 0.8f, 1.0f, 0.95f ) );
		
		m_Value = 0.0f;
		m_PrevValue = 0.0f;
		
		m_MaxValue = 1.0f;
		
		m_StepSize = 0.1f;
	}
	
	Slider::~Slider()
	{
	
	}
	
	void Slider::Initialize()
	{
	
	}
	
	void Slider::Update( float deltaTime , glm::ivec2 parentPos )
	{
		m_BoundingBox.Origin = parentPos;
		m_SliderBounds.Origin = parentPos;
		m_SliderBounds.X = m_Slider.Position.x;
		m_SliderBounds.Y = m_Slider.Position.y;
		m_SliderBounds.Width = m_Slider.Width;
		m_SliderBounds.Height = m_Slider.Height;
		
		if( m_SliderBounds.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
		{
			m_Hover = true;
			
			if( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT ) )
			{
				m_Dragging = true;
				m_ClickPos = glm::ivec2( g_Input->GetMousePosX() - m_SliderBounds.X , g_Input->GetMousePosY() - m_SliderBounds.Y );
				
				g_Input->ConsumeMouseButtons();
			}
			if( g_Input->MouseDownUp( MOUSE_BUTTON_LEFT ) )
				g_Input->ConsumeMouseButtons();
			
		}
		 else if( m_BoundingBox.Intersects( g_Input->GetMousePosX(), g_Input->GetMousePosY() ) )
		{
			if( g_Input->MouseUpDown( MOUSE_BUTTON_LEFT ) )
			{
				m_Dragging = true;
				m_Slider.Position.x = g_Input->GetMousePosX() - parentPos.x - ( m_Slider.Width / 2 );
				m_SliderBounds.X = m_Slider.Position.x;
				m_ClickPos = glm::ivec2( g_Input->GetMousePosX() - m_SliderBounds.X , g_Input->GetMousePosY() - m_SliderBounds.Y );


				g_Input->ConsumeMouseButtons();
			}
			if( g_Input->MouseDownUp( MOUSE_BUTTON_LEFT ) )
				g_Input->ConsumeMouseButtons();
			

			int scroll = g_Input->GetScrollY();
			
			if( scroll != 0 )
			{
				m_Value += m_StepSize * scroll;
				
				if( m_Value > m_MaxValue )
					m_Value = m_MaxValue;
				else if( m_Value < 0 )
					m_Value = 0;
				
				UpdateSliderPosition();
			}
		}
		
		if( m_Dragging && g_Input->MouseUp( MOUSE_BUTTON_LEFT ) )
		{
			m_Dragging = false;
		}
		
		if( m_Dragging )
		{
			m_Slider.Position.x = g_Input->GetMousePosX() - m_ClickPos.x;
		}
		
		if(			m_Slider.Position.x > m_BoundingBox.X + m_BoundingBox.Width - ( m_SliderBounds.Width / 2 ))
			m_Slider.Position.x = m_BoundingBox.X + m_BoundingBox.Width - ( m_SliderBounds.Width / 2 );
		else if(	m_Slider.Position.x < m_BoundingBox.X - ( m_SliderBounds.Width / 2 ) )
			m_Slider.Position.x = m_BoundingBox.X - ( m_SliderBounds.Width / 2 );
			
			
		m_Value = ( ( m_SliderBounds.X - m_BoundingBox.X + ( m_SliderBounds.Width / 2 ) ) / (float)m_BoundingBox.Width ) * m_MaxValue;
		
		
		if( m_Value != m_PrevValue )
		{
			m_Value = floorf( m_Value * 100 ) / 100;
			m_PrevValue = m_Value;
		}
	}
	
	void Slider::Render( glm::ivec2 parentPos )
	{
		m_Background.Origin = parentPos;
		m_Slider.Origin = parentPos;
		

		g_GUI.EnqueueSprite( &m_Background );
		g_GUI.EnqueueSprite( &m_Slider );
		
	}

	SpriteDefinition& Slider::GetBackgroundRef()
	{
		return m_Background;
	}
	
	void Slider::SetBackgroundImage( const rString& imagePath )
	{
		m_Background.Texture = imagePath;
	}

	void Slider::SetSliderImage( const rString& imagePath )
	{
		m_Slider.Texture = imagePath;
	}
	
	void Slider::SetValue( float value )
	{
		m_Value = value;
		UpdateSliderPosition();
	}
	
	void Slider::SetMaxValue( float maxValue )
	{
		m_MaxValue = maxValue;
	}
	
	float Slider::GetValue()
	{
		return m_Value;
	}
	
	float Slider::GetMaxValue()
	{
		return m_MaxValue;
	}
	
	void Slider::UpdateSliderPosition()
	{
		float posX = ( m_BoundingBox.X + m_BoundingBox.Width * (m_Value / m_MaxValue) ) - ( m_Slider.Width / 2 );
		
		m_Slider.Position.x = static_cast<int>( posX );
	}
	
	
}
