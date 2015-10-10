/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "Object.h"
#include "../graphics2D/SpriteDefinition.h"

namespace GUI
{
	class Sprite : public Object
	{
	public:
		Sprite();
		Sprite( rString name, SpriteDefinition spriteDefinition, rString parent );
		~Sprite();

		void					Initialize();
		void					Update( float deltaTime, glm::ivec2 parentPos );
		void					Render( glm::ivec2 parentPos );

		GUI_API SpriteDefinition&	GetSpriteDefinitionRef();

		GUI_API void			SetTexture( rString texturePath );

		GUI_API void			SetPosition( int x, int y );

	private:
		SpriteDefinition		m_SpriteDefinition;
	};
}
