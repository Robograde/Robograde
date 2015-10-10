/**************************************************
Zlib Copyright 2015 Isak Almgren
***************************************************/

#include "Graphics2D.h"

namespace GUI 
{
	Graphics2D& Graphics2D::GetInstance() 
	{
		static Graphics2D input;
		return input;
	}
	
	Graphics2D::Graphics2D()
	{ }

	Graphics2D::~Graphics2D()
	{
		pDelete( m_TextShader );
		pDelete( m_SpriteShader );
	}

	void Graphics2D::Initialize( )
	{
		m_TextRenderer.Initialize();
		m_SpriteRenderer.Initialize();
		
		m_TextShader =		pNew( gfx::ShaderProgram );
		m_SpriteShader =	pNew( gfx::ShaderProgram );
		
		m_TextShader->LoadCompleteShaderProgramFromFile( "../../../shader/gui/TextShader.glsl", true );
		
		m_SpriteShader->LoadCompleteShaderProgramFromFile( "../../../shader/gui/SpriteShader.glsl", true );
	}

	void Graphics2D::Render( int windowWidth, int windowHeight )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		//Just some code to make sure no stuff is changed
		bool blendWasDisabled = false;
		bool depthWasDisabled = false;
		if ( !glIsEnabled( GL_BLEND ) ) 
		{
			blendWasDisabled = true;
			glEnable( GL_BLEND );
		}
		
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		
		RenderType nextRenderType = RenderType::None;
		
		int renderQueueSize = static_cast<int>( m_RenderQueue.size() );
		
		float zOffset = 0.999f / renderQueueSize; //Won't render first thing if it's placed at 1, that's why it should be very close to 1 instead.
		
		for( int i = 0; i < renderQueueSize; i++ )
		{
			RenderDefinition* renderDefinition = m_RenderQueue[i];
			
			renderDefinition->Depth = 0.999f - i* zOffset;
			
			if( i + 2 < renderQueueSize )
				nextRenderType = m_RenderQueue[i+1]->MyRenderType;
			else
				nextRenderType = RenderType::None;
			
			if( renderDefinition->MyRenderType == RenderType::Text )
				m_TextRenderer.EnqueueText( static_cast<TextDefinition*>( renderDefinition ) );
			else if ( renderDefinition->MyRenderType == RenderType::Sprite )
				m_SpriteRenderer.EnqueueSprite( static_cast<SpriteDefinition*>( renderDefinition ) );
		}
		
		m_SpriteRenderer.Render( windowWidth, windowHeight, m_SpriteShader );
		m_TextRenderer.Render( windowWidth, windowHeight, m_TextShader );
		
		m_RenderQueue.clear();
		
		//Just some code to make sure no stuff is changed
		if ( blendWasDisabled )
			glDisable( GL_BLEND );
		if ( !depthWasDisabled )
			glEnable( GL_DEPTH_TEST );
	}
	
	void Graphics2D::EnqueueText( TextDefinition* textDefinition )
	{
		m_RenderQueue.push_back( textDefinition );
	}
	
	void Graphics2D::EnqueueSprite( SpriteDefinition* spriteDefinition )
	{
		m_RenderQueue.push_back( spriteDefinition );
	}
	
	glm::ivec2 Graphics2D::GetTextSize( TextDefinition* textDefinition )
	{
		return m_TextRenderer.GetTextSize( textDefinition->Text.c_str(), textDefinition->FontID );
	}
	
}
