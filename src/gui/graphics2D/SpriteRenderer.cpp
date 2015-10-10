/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "SpriteRenderer.h"
#include <utility/Logger.h>
#include <profiler/AutoGPUProfiler.h>
#include <input/Input.h>

GUI::SpriteRenderer::SpriteRenderer()
{

}

GUI::SpriteRenderer::~SpriteRenderer()
{
	glDeleteVertexArrays( 1, &m_VBO );
	glDeleteVertexArrays( 1, &m_VAO );
	
	pDeleteArray( m_ShaderInputs );
}

void GUI::SpriteRenderer::Initialize()
{
	m_ShaderInputs = pNewArray( ShaderInput, MAX_SPRITES );
	
	glGenBuffers( 1, &m_VBO );
	glGenVertexArrays( 1, &m_VAO );
	glBindBuffer( GL_ARRAY_BUFFER, m_VBO );
	glBufferData( GL_ARRAY_BUFFER, MAX_SPRITES * sizeof( ShaderInput ), nullptr, GL_DYNAMIC_DRAW );
	glBindVertexArray( m_VAO );
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );
	glEnableVertexAttribArray( 3 );
	glEnableVertexAttribArray( 4 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( ShaderInput ), 0 );								// Position.xyz
	glVertexAttribPointer( 1, 1, GL_FLOAT, GL_FALSE, sizeof( ShaderInput ), ( void* )( 3 * sizeof(GLfloat) ) ); // TextureID
	glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, sizeof( ShaderInput ), ( void* )( 4 * sizeof(GLfloat) ) ); // Texture coords for top-left and bottom-right
	glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, sizeof( ShaderInput ), ( void* )( 8 * sizeof(GLfloat) ) ); //
	glVertexAttribPointer( 4, 2, GL_FLOAT, GL_FALSE, sizeof( ShaderInput ), ( void* )( 12 * sizeof(GLfloat) ) );//
	


	m_SpriteQueue.reserve( MAX_SPRITES );
	m_SpecialSpriteQueue.reserve( MAX_SPECIAL_SPRITES );
	
	
	m_SpriteAtlas.LoadFile( "../../../asset/gui/sprites.json" );
	glGenBuffers( 1, &m_TextureBuffer );
	glBindBuffer( GL_UNIFORM_BUFFER, m_TextureBuffer );
	glBufferData( GL_UNIFORM_BUFFER, static_cast<int>( m_SpriteAtlas.GetGPUTextures().size() * sizeof( gfx::GPUTexture ) ), m_SpriteAtlas.GetGPUTextures().data(), GL_DYNAMIC_DRAW );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );
	
	m_SpriteAtlas.GetTexture()->Apply( 0, 0 );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void GUI::SpriteRenderer::Render( int windowWidth, int windowHeight, gfx::ShaderProgram* shaderProg )
{
	if( g_Input->KeyDownUp( SDL_SCANCODE_L ) )
		m_SpriteAtlas.LoadFile( "../../../asset/gui/sprites.json" );

	int numSprites = 0;

	for( SpriteDefinition* spriteDef : m_SpriteQueue )
	{
		if( spriteDef->Width <= 0 || spriteDef->Height <= 0 )
			continue;
		
		int offsetX = 0;
		int offsetY = 0;
		if( spriteDef->CenterX )
			offsetX = static_cast<int>( spriteDef->Width / 2.0f );
		if( spriteDef->CenterY )
			offsetY = static_cast<int>( spriteDef->Height / 2.0f );

		float xPos = ( spriteDef->Origin.x + spriteDef->Position.x - offsetX ) / (float) windowWidth;
		float yPos = ( spriteDef->Origin.y + spriteDef->Position.y - offsetY ) / (float) windowHeight;

		float sx = 2.0f / windowWidth;
		float sy = 2.0f / windowHeight;

		float x = (((xPos)* 2) - 1);
		float y = -(((yPos)* 2) - 1);

		float w = spriteDef->Width * sx;
		float h = spriteDef->Height * sy;
		
		ShaderInput input;
		input.Position = glm::vec3( x, y, spriteDef->Depth );
		
		if( spriteDef->Texture == "" )
		{
			if( spriteDef->TextureHandle != -1 )
			{
				input.TextureID = -1;
				shaderProg->SetUniformTextureHandle( "specialTexture", spriteDef->TextureHandle, 1 );
				
			}
			else if( spriteDef->TextureHandle2 != -1 )
			{
				input.TextureID = -2;
				shaderProg->SetUniformTextureHandle( "specialTexture2", spriteDef->TextureHandle2, 2 );
			}
			else
				input.TextureID = (float)m_SpriteAtlas.GetHandle( "default.png" );
		}
		else
		{
			input.TextureID = (float)m_SpriteAtlas.GetHandle( spriteDef->Texture );
				
			if( input.TextureID == -1 )
				input.TextureID = (float)m_SpriteAtlas.GetHandle( "default.png" );
		}
		input.UV_Tl_Br = glm::vec4( spriteDef->TopLeftUV.x, spriteDef->TopLeftUV.y, spriteDef->BottomRightUV.x, spriteDef->BottomRightUV.y );
		input.Colour = spriteDef->Colour;
		input.Size = glm::vec2( w, h );
		
		m_ShaderInputs[numSprites++] = input;
		
		if( spriteDef->BorderSize > 0 )
			RenderBorder( sx, sy, x, y, w, h, spriteDef, numSprites );
	
		
	}
	shaderProg->Apply();
	m_SpriteAtlas.GetTexture()->Apply( 0, 0 );
	
	glBindBuffer( GL_UNIFORM_BUFFER, m_TextureBuffer );
	GLint bi = glGetUniformBlockIndex( shaderProg->GetHandle(), "TextureBuffer" );
	glUniformBlockBinding( shaderProg->GetHandle(), bi, 1 );
	glBindBufferBase( GL_UNIFORM_BUFFER, 1, m_TextureBuffer );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );
	
	//Update buffers
	glBindBuffer( GL_ARRAY_BUFFER, m_VBO );
	glBufferSubData( GL_ARRAY_BUFFER, 0, numSprites * sizeof( ShaderInput ), m_ShaderInputs );
	glBindVertexArray( m_VAO );
	
	//Draw
	{
		GPU_PROFILE( AutoGPUProfiler gpuProfile("GUIRenderSprite", true) );
		glDrawArrays( GL_POINTS, 0, numSprites );
		GPU_PROFILE( gpuProfile.Stop(); );
	}
	

	glBindVertexArray( 0 );
	glUseProgram( 0 );
	m_SpriteQueue.clear();
}

void GUI::SpriteRenderer::EnqueueSprite( GUI::SpriteDefinition* spriteDefinition )
{
	m_SpriteQueue.push_back( spriteDefinition );
}

void GUI::SpriteRenderer::RenderBorder( float sx, float sy, float x, float y, float w, float h, SpriteDefinition* spriteDef, int& numSprites )
{

	float bsh = spriteDef->BorderSize * sy; //Border size horizontal
	float bsv = spriteDef->BorderSize * sx; //Border size vertical

	float depth = spriteDef->Depth - 0.001f;

	
	ShaderInput input;
	input.Colour = spriteDef->BorderColour;
	input.TextureID = (float)m_SpriteAtlas.GetHandle( "default.png" );
	input.UV_Tl_Br = glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f);
	
	//Top rect
	input.Position =	glm::vec3( x, y, depth );
	input.Size =		glm::vec2( w, bsh );
	m_ShaderInputs[numSprites++] = input;
	
	//Bottom
	input.Position =	glm::vec3( x, y - h + bsh, depth );
	input.Size =		glm::vec2( w, bsh );
	m_ShaderInputs[numSprites++] = input;
	
	//Left
	input.Position =	glm::vec3( x, y - bsh, depth );
	input.Size =		glm::vec2( bsv, h - bsh * 2 );
	m_ShaderInputs[numSprites++] = input;
	
	//Right
	input.Position =	glm::vec3( x + w - bsv, y - bsh, depth );
	input.Size =		glm::vec2( bsv, h - bsh * 2 );
	m_ShaderInputs[numSprites++] = input;
}

