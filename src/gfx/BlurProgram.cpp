/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "BlurProgram.h"
#include <utility/Logger.h>
#include <glm/gtx/transform.hpp>
gfx::BlurProgram::BlurProgram()
{
	m_FBO[0] = 0;
	m_FBO[1] = 0;

	m_Textures[0] = 0;
	m_Textures[1] = 0;
}

gfx::BlurProgram::~BlurProgram()
{

}

void gfx::BlurProgram::Init( glm::vec2 screenSize, float downsample )
{
	m_Width		= static_cast<int>( screenSize.x * downsample );
	m_Height	= static_cast<int>( screenSize.y * downsample );
	glDeleteTextures(2, m_Textures );
	//generate textures
	glGenTextures( 2, m_Textures );
	glBindTexture( GL_TEXTURE_2D, m_Textures[0] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)(screenSize.x * downsample), (GLsizei)(screenSize.y * downsample), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindTexture( GL_TEXTURE_2D, m_Textures[1] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)(screenSize.x * downsample), (GLsizei)(screenSize.y * downsample), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glDeleteFramebuffers( 2, m_FBO );
	glGenFramebuffers( 2, m_FBO );
	glBindFramebuffer( GL_FRAMEBUFFER, m_FBO[0] );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_Textures[0], 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, m_FBO[1] );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_Textures[1], 0 );
	
	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == false )
	{
		Logger::Log( "GBuffer error attaching a texture to the framebuffer", "BlurProgram", LogSeverity::ERROR_MSG );
	}
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	CalculateWeight(1.0f);
	//load shader
	m_Program = g_ShaderBank.LoadShaderProgram("../../../shader/BlurEffect.glsl");
}

void gfx::BlurProgram::Draw(DrawData* data)
{
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle( m_Program );
	prog->Apply();
	prog->SetUniformFloat( "gWeights[0]", m_Weights[0] );
	prog->SetUniformFloat( "gWeights[1]", m_Weights[1] );
	prog->SetUniformFloat( "gWeights[2]", m_Weights[2] );

	//blur horisontaly 
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_FBO[0] );
	GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, drawbuffers );
	//Draw to texture 0
	prog->SetUniformBool( "gVerticaly", false );
	prog->SetUniformTextureHandle( "gInputTex", ((Inparams*)data->ExtraData)->SrcTexture, 0 );
	glBindVertexArray( 0 );
	glDrawArrays( GL_POINTS, 0, 1 );
	//blur verticaly
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_FBO[1] );
	glDrawBuffers( 1, drawbuffers );
	//draw to texture 1
	prog->SetUniformBool( "gVerticaly", true );
	prog->SetUniformTextureHandle( "gInputTex", m_Textures[0], 0 );
	glBindVertexArray( 0 );
	glDrawArrays( GL_POINTS, 0, 1 );

	//ping pong!
	for(int i = 0; i < 1; i++)
	{
		//blur horisontaly 
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_FBO[0] );
		GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers( 1, drawbuffers );
		//Draw to texture 0
		prog->SetUniformBool( "gVerticaly", false );
		prog->SetUniformTextureHandle( "gInputTex", m_Textures[1], 0 );
		glBindVertexArray( 0 );
		glDrawArrays( GL_POINTS, 0, 1 );
		//blur verticaly
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_FBO[1] );
		glDrawBuffers( 1, drawbuffers );
		//draw to texture 1
		prog->SetUniformBool( "gVerticaly", true );
		prog->SetUniformTextureHandle( "gInputTex", m_Textures[0], 0 );
		glBindVertexArray( 0 );
		glDrawArrays( GL_POINTS, 0, 1 );
	}

	glUseProgram(0);
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void gfx::BlurProgram::Resize(glm::vec2 screenSize, float downsample)
{
	m_Width		= static_cast<int>( screenSize.x * downsample );
	m_Height	= static_cast<int>( screenSize.y * downsample );

	glBindTexture( GL_TEXTURE_2D, m_Textures[0] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)(screenSize.x * downsample), (GLsizei)(screenSize.y * downsample), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glBindTexture( GL_TEXTURE_2D, m_Textures[1] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)(screenSize.x * downsample), (GLsizei)(screenSize.y * downsample), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
}

void gfx::BlurProgram::CalculateWeight(float sigma)
{
	float constant = 1.0f / (sigma * sigma * sqrtf(glm::pi<float>() * 2.0f));
	for(int i = 0; i < 3; i++)
	{
		float expression = -1 * (i * i / 2.0f * sigma * sigma);
		m_Weights[i] = constant * glm::pow(glm::e<float>(), expression);
	}
}