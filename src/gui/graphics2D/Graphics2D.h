/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "TextRenderer.h"
#include "SpriteRenderer.h"

#include <memory/Alloc.h>

namespace GUI 
{
	#define g_Graphics2D Graphics2D::GetInstance()
	
	class Graphics2D
	{
	public:
		static Graphics2D&	GetInstance();
		
		Graphics2D();
		~Graphics2D();
		
		void Initialize( );
		
		void				Render( int windowWidth, int windowHeight );

		void				EnqueueText( TextDefinition* textDefinition );
		void				EnqueueSprite( SpriteDefinition* spriteDefinition );
		
		glm::ivec2			GetTextSize( TextDefinition* textDefinition );
		
	private:
		
		rVector<RenderDefinition*>		m_RenderQueue;
		
		TextRenderer					m_TextRenderer;
		SpriteRenderer					m_SpriteRenderer;
		
		gfx::ShaderProgram*				m_SpriteShader;
		gfx::ShaderProgram*				m_TextShader;
	};
}
