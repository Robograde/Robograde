/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "Object.h"
#include "../graphics2D/SpriteDefinition.h"


namespace GUI
{
	class Slider : public Object
	{
	public:
		Slider();
		Slider( const rString& name, const rString& parent, Rectangle boundingBox );
		~Slider();
		
		void					Initialize();
		void					Update( float deltaTime, glm::ivec2 parentPos );
		
		void					Render( glm::ivec2 parentPos );
		
		//Getters
		GUI_API SpriteDefinition& GetBackgroundRef();
		
		
		
		GUI_API void			SetBackgroundImage( const rString& imagePath );
		GUI_API void			SetSliderImage( const rString& imagePath );
		
		GUI_API void			SetValue( float value );
		GUI_API void			SetMaxValue( float maxValue );
		
		GUI_API float			GetValue();
		GUI_API float			GetMaxValue();
		
		
	private:
		
		void					UpdateSliderPosition();
		
		SpriteDefinition		m_Background;
		
		SpriteDefinition		m_Slider;
		Rectangle				m_SliderBounds;
		
		glm::ivec2				m_ClickPos;
		
		bool					m_Hover;
		bool					m_Dragging;
		
		float					m_MaxValue;
		float					m_Value;
		float					m_StepSize;
		
		float					m_PrevValue;
	
	};
}
