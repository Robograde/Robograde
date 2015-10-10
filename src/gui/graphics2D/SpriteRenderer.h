/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#pragma once
#include "SpriteDefinition.h"
#include <gfx/Shader/ShaderProgram.h>
#include <gfx/TextureAtlas.h>


namespace GUI
{
	#define MAX_SPRITES 2048
	#define MAX_SPECIAL_SPRITES 16
	
	struct ShaderInput
	{
		glm::vec3		Position;
		float			TextureID = -1;
		glm::vec4		UV_Tl_Br;
		glm::vec4		Colour;
		glm::vec2		Size;
		glm::vec2		Padding = glm::vec2( 0.0f );
	};
	
	struct LoneShaderInput
	{
		glm::vec4		Position;
		glm::vec4		TexCoords; //Top-left Bottom-Right UV coordinates
		glm::vec4		Colour;
		glm::vec2		Size;
		glm::vec2		Padding = glm::vec2( 0.0f );
	};
	
	class SpriteRenderer
	{
	public:
									SpriteRenderer();
									~SpriteRenderer();
									
			void					Initialize( );
									
			void 					Render( int windowWidth, int windowHeight, gfx::ShaderProgram* shaderProg );
									
			void					EnqueueSprite( SpriteDefinition* spriteDefinition );
									
		private:					
			void					RenderBorder( float sx, float sy, float x, float y, float w, float h, SpriteDefinition* spriteDef, int& numSprites );
		
		rVector<SpriteDefinition*>	m_SpriteQueue;
		rVector<SpriteDefinition*>	m_SpecialSpriteQueue;
		
		gfx::TextureAtlas			m_SpriteAtlas;
		
		
		ShaderInput*				m_ShaderInputs;
		
		
		GLuint						m_VBO;
		GLuint						m_VAO;
		GLuint						m_TextureBuffer;
		
		GLuint						m_SpecialVBO;
		
	};
}
