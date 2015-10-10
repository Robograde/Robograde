/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "Object.h"
#include "../graphics2D/TextDefinition.h"
#include "../graphics2D/SpriteDefinition.h"

namespace GUI
{
	class TextBox : public Object
	{
	public:
								TextBox();
								TextBox( const rString& name, const rString& parent, Rectangle boundingBox );
								~TextBox();
								
		void					Initialize();
		void					Update( float deltaTime, glm::ivec2 parentPos );
		void					Render( glm::ivec2 parentPos );
		void					OnWindowOpen();
		void					OnWindowClose();
		
		GUI_API void			StartInput();
		GUI_API void			StopInput();
		
		GUI_API SpriteDefinition& GetBackgroundRef();
		GUI_API TextDefinition&	GetTextDefinitionRef();
		GUI_API rString&		GetText();
		
		GUI_API void			SetText( const rString& text );
		GUI_API bool			IsInputting();

		GUI_API void			SetSize( int width, int height );
	private:
		TextDefinition			m_TextDefinition;
		SpriteDefinition		m_Background;
		rString					m_Text;

		glm::vec4				m_BackgroundColour;
		glm::vec4				m_BorderColour;
		
		rString					m_FontName;
		int						m_FontSize;
		
		bool					m_IsInputting = false;
		unsigned int			m_InputStringID = -1;
		
		float					m_MarkerFlashTime = 0.0f;

		int						m_MaxLength = 10;
	};
}
