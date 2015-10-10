/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "Object.h"
#include "../graphics2D/TextDefinition.h"


namespace GUI
{
	class Text : public Object
	{
	public:
		Text();
		Text( const rString& name, TextDefinition textDefinition, const rString& parent, Rectangle boundingBox );
		~Text();
				
		void					Initialize();
		void					Update( float deltaTime, glm::ivec2 parentPos );
		void					Render( glm::ivec2 parentPos );
		
		GUI_API TextDefinition&	GetTextDefinitionRef();
		GUI_API void			SetTextDefinition( TextDefinition textDefinition );
		GUI_API void			SetText( const rString& text );
		GUI_API void			SetTextAlignment( ALIGNMENT textAlign );

		GUI_API void			SetPosition( int x, int y );
		GUI_API void			SetPosition( const glm::ivec2& position );
		
	private:
		TextDefinition	m_TextDefinition;
		TextDefinition	m_TextBackground;
	};
}
