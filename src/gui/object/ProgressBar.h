/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "Object.h"
#include "../graphics2D/SpriteDefinition.h"

namespace GUI
{
	class ProgressBar : public Object
	{
	public:
		ProgressBar();
		ProgressBar( rString name, rString parent, Rectangle boundingBox );
		~ProgressBar();
		
		void					Initialize();
		void					Update( float deltaTime, glm::ivec2 parentPos );
		void					Render( glm::ivec2 parentPos );
		
		GUI_API void			SetBackgroundTexture( rString texturePath );
		GUI_API void			SetBarTexture( rString texturePath );
		
		GUI_API void SetBackgroundColour( glm::vec4 colour );
		GUI_API void SetBarColour( glm::vec4 colour );
		GUI_API void SetBorderColour( glm::vec4 colour );
		
		
		GUI_API void SetMinValue( float value );
		GUI_API void SetMaxValue( float value );
		GUI_API void SetValue( float value );
		GUI_API void SetPosition( int x, int y );
		
	private:
		SpriteDefinition		m_Background;
		SpriteDefinition		m_Bar;
		
		int						m_BarWidth;
		
		
		float					m_MinValue = 0;
		float					m_MaxValue = 1;
		float					m_Value = 0;
	};
}
