/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "GBuffer.h"
#include <utility/Logger.h>
using namespace gfx;

GBuffer::GBuffer( )
{
	// Set default values
	std::fill( m_BufferTex, m_BufferTex + NUMBER_OF_GBUFFERS, 0 );
}

GBuffer::~GBuffer( )
{
}

void GBuffer::Initialize( int width, int height )
{
	m_Width = width;
	m_Height = height;
	// Set up Textures
	glDeleteTextures( NUMBER_OF_GBUFFERS, m_BufferTex );
	glGenTextures( NUMBER_OF_GBUFFERS, m_BufferTex );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[ALBEDO24] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[NORMAL48] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[DEPTH_SENCIL32] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Width, m_Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[ROUGHNESS8_METAL8] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RG8, m_Width, m_Height, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[MATID8] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_R8UI, m_Width, m_Height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[GLOW24] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//glBindTexture( GL_TEXTURE_2D, m_BufferTex[EMISSIVE24] );
	//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[COLLECTIVE24] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	for( int i = 0; i < NUMBER_OF_GBUFFERS; ++i )
	{
		if( !glIsTexture( m_BufferTex[i] ) )
		{
			Logger::Log( "Error creating texture for GBuffer", "GBuffer", LogSeverity::ERROR_MSG );
			assert( false );
		}
	}
	// Set up framebufffer
	glDeleteFramebuffers( 1, &m_FBO );
	glGenFramebuffers( 1, &m_FBO );
	glBindFramebuffer( GL_FRAMEBUFFER, m_FBO );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_BufferTex[ALBEDO24], 0 );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_BufferTex[NORMAL48], 0 );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_BufferTex[ROUGHNESS8_METAL8], 0 );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, m_BufferTex[MATID8], 0 );
	//glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, m_BufferTex[EMISSIVE24], 0 );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, m_BufferTex[GLOW24], 0 );
	glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_BufferTex[DEPTH_SENCIL32], 0 );

	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == false )
	{
		Logger::Log( "GBuffer error attaching a texture to the framebuffer", "GBuffer", LogSeverity::ERROR_MSG );
	}
}

void GBuffer::ApplyGeometryStage( )
{
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_FBO );
	GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
							 GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers( 5, drawbuffers );
}

void GBuffer::ApplyLightingStage( )
{
	// not needed since lighting uses a compute shader
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	// GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT5 };
	// glDrawBuffers(1,drawbuffers);
	// glClear(GL_COLOR_BUFFER_BIT);
}

void GBuffer::ApplyPostProcessStage( )
{
	// reset to normal framebuffer
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// glClear(GL_COLOR_BUFFER_BIT);
}

void GBuffer::ApplyDecalStage()
{
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_FBO );
	GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers( 2, drawbuffers );
}

void  GBuffer::ClearScreen()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}


void GBuffer::Resize( int width, int height )
{
	m_Width = width;
	m_Height = height;

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[ALBEDO24] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[NORMAL48] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, nullptr );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[DEPTH_SENCIL32] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Width, m_Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[ROUGHNESS8_METAL8] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RG8, m_Width, m_Height, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[MATID8] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_R8UI, m_Width, m_Height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[GLOW24] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );

	//glBindTexture( GL_TEXTURE_2D, m_BufferTex[EMISSIVE24] );
	//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );

	glBindTexture( GL_TEXTURE_2D, m_BufferTex[COLLECTIVE24] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, nullptr );
}

GLuint GBuffer::GetTexture( GBUFFER_TEX texture ) const
{
	return m_BufferTex[texture];
}
