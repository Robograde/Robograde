/**************************************************
Zlib Copyright 2015 Henrik Johansson
***************************************************/

#include "DepthBuffer.h"
#include <utility/Logger.h>
#include <memory/Alloc.h>
gfx::DepthBuffer::DepthBuffer()
{

}
gfx::DepthBuffer::~DepthBuffer()
{

}

void gfx::DepthBuffer::Initialize( int width, int height )
{
	m_Width = width;
	m_Height = height;

	//Generate depth texture
	glGenTextures( 1, &m_DepthTexture );
	glBindTexture( GL_TEXTURE_2D, m_DepthTexture );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Width, m_Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );

	glGenFramebuffers( 1, &m_FBO );
	glBindFramebuffer( GL_FRAMEBUFFER, m_FBO );

	glDrawBuffer( GL_NONE );
	glReadBuffer( GL_NONE );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0 );
	if ( !glIsTexture( m_DepthTexture ) )
	{
		Logger::Log( "Error, unable to create depth buffer", "GFX", LogSeverity::ERROR_MSG );
		assert( false );
		return;
	}
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if ( status != GL_FRAMEBUFFER_COMPLETE )
	{
		Logger::Log( ( rString( "FBO error!!, status: " ) + rString( ( char* )glGetString( status ) ) ).c_str(), "GFX", LogSeverity::ERROR_MSG );
		assert( false );
		return;
	}
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void gfx::DepthBuffer::Apply()
{
	glBindFramebuffer( GL_FRAMEBUFFER, m_FBO );
	glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}

GLuint gfx::DepthBuffer::GetTexture() const
{
	return m_DepthTexture;
}
