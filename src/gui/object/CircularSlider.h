/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "Object.h"
#include "../graphics2D/SpriteDefinition.h"


namespace GUI
{
	class CircularSlider : public Object
	{
	public:
		CircularSlider();
		CircularSlider( const rString& name, const rString& parent, glm::ivec2 position );
		~CircularSlider();
		
		void					Initialize();
		void					Update( float deltaTime, glm::ivec2 parentPos );
		
		void					Render( glm::ivec2 parentPos );

		GUI_API void			OnValueChange();

		GUI_API void			SetMinValue();
		GUI_API void			SetMaxValue();
		GUI_API void			SetValue();

		GUI_API void			SetRadius( int radius );
		GUI_API void			SetMinRadius( int radius );
		GUI_API void			SetMaxRadius( int radius );
		GUI_API void			SetPosition( int x, int y );
		
		//Getters
		GUI_API SpriteDefinition& GetBackgroundRef();
		GUI_API glm::vec4&		GetColour();

		GUI_API bool			GetChanged();
		GUI_API int				GetRadius();
		
		
		GUI_API void			SetBackgroundImage( const rString& imageName );
		
		GUI_API void			SetColour( glm::vec4 colour );
		GUI_API void			SetHighLightColour( glm::vec4 colour );

		GUI_API void			SetDisabledColour( glm::vec4 colour );
	private:
		
		glm::ivec2		m_ClickPos;
		
		bool			m_Hover = false;
		bool			m_Dragging = false;
		bool			m_Down = false;
		bool			m_Changed = false;
		
		glm::vec4		m_Colour;
		glm::vec4		m_HighLightColour;
		glm::vec4		m_DisabledColour;
		
		
		SpriteDefinition	m_Background;

		glm::ivec2			m_CenterPoint;
		int					m_Radius;
		int					m_BorderSize;
		int					m_MinRadius;
		int					m_MaxRadius;


		//float maxValue
	};
}
